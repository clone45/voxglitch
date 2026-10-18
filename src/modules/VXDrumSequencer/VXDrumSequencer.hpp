#pragma once
//
// VX Drum Sequencer — a 16-step x 6-lane trigger sequencer with an accent lane,
// per-step ratchets and sixteen pattern memories, driven by an external clock.
//
// The sequencer half of the vxsynth "VX Drum Machine" (vxdrums.c). The voices
// live in the VXDrums kit module; the two panels are linked by a polyphonic
// TRIG cable (channel c = voice c: BD SD CP PERC CH OH) and a separate mono ACC
// gate cable. See:
//   docs/modules/vx-drum-sequencer/rack-port-design.md    <- what diverges from vxsynth and why; §4 is the reset rule
//   docs/modules/vx-drum-sequencer/chain-foundations.md   <- the engine split and the chain
//   src/vgLib-2.0/sequencer/ResetClockWindow.hpp           <- the VCV reset standard the rule follows (2026-09-16)
//   scratchpad brief-clock-reset.md §5-§7                <- the clock algorithm this follows (its reset half is superseded)
//
// This file is the RACK ADAPTER. The sequencing itself — the clock period, the
// playhead and its first-step convention, the ratchets, the trigger pulses
// with their gap rule — is vx_drum_sequencer::Sequencer in
// VXDrumSequencerEngine.hpp, which is Rack-free and tested in
// tests/vx_drum_sequencer/. This module reads the jacks, resolves WHICH memory
// plays this sample (a PlaySource), calls the engine, and writes the jacks.
//
// External clock only (owner decision, 2026-09-02): the source's internal
// clock, RUN transport and swing are not ported. Song mode (clockmode 2) is
// not ported either: Rack has no host playhead.
//
// CHAIN (2026-09-07): VX Drum Sequencers placed side by side, touching, play
// one after another as a single longer pattern. The leftmost is the HEAD: it
// owns the one engine, reads CLK / RST, and computes TRIG / ACC. Every module to
// its right is a FOLLOWER: its CLK, RST and RWD are inert, but its MEM buttons,
// MEM CV and mutes still choose what it contributes, its grid shows the playhead
// while it is the active member, and its TRIG / ACC MIRROR the head's (one
// sample late, see chain_out), so the kit can be patched from whichever member
// sits nearest it. Each member plays its
// effective memory from step 1 to its length, then hands off to the next; the
// last hands back to the head. The head pulls each follower's bank, slot and
// mute directly through the follower's own module (one brain pulling, no
// hop-by-hop expander messages); followers copy the head's ChainReport into
// their own Report. Roles are resolved from adjacency every sample and never
// persisted (identity by pointer, never by name or id).
//
// THREADS: the pattern Bank is UI-owned and double-buffered for the audio
// thread (PianoRoll.hpp:107-129 idiom). Widgets mutate a copy and publish; the
// audio thread takes ONE reference per process() call. `mute`, `memory_slot`,
// `report.*`, `trigger_length_index` and the ChainReport fields are single
// aligned words shared across threads, accepted as-is (PianoRoll.hpp:157-159).
// A follower reads the head's ChainReport while the head may be writing it (Rack
// can run one frame's modules on several workers): the fields are aligned
// words, so the worst case is one frame's flash misdrawn — the same class of
// read the grid already accepts for fired_mask.
//

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// House precedent for a file-scope using-directive on the module's own
// namespace: PianoRoll.hpp:24 (`using namespace piano_roll;`).
using namespace vx_drum_sequencer;

struct VXDrumSequencer : VoxglitchModule
{
    enum ParamIds
    {
        ENUMS(MEMORY_PARAMS, 16),   // momentary (configButton) — selects the memory
        REWIND_PARAM,               // configButton
        RANDOM_PARAM,               // configButton
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        RESET_INPUT,
        MEM_CV_INPUT,
        TWEAK_INPUT,
        RANDOM_INPUT,        // poly trigger: mono = the whole pattern as the RND button, channel c = lane c only (2026-09-18); appended last so older patches keep their cable ids
        NUM_INPUTS
    };

    enum OutputIds
    {
        TRIG_OUTPUT,
        ACC_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        ENUMS(MEMORY_LIGHTS, 16),
        NUM_LIGHTS
    };

    // ── Pattern bank, UI-owned, double-buffered for the audio thread ─────────
    //
    // Two buffers are enough because there is exactly one writer (the UI), it
    // writes only to the buffer the reader is NOT using, and the reader takes a
    // single reference at the top of process() and finishes within that call
    // (PianoRoll.hpp:113-121 explains why a shared_ptr swap would be wrong).
    //
    // UI thread only: Bank b = module->bankCopy(); mutate b; module->publishBank(b);
    // plus one VXDrumSequencerBankAction per gesture (§4.7 below).
    //
    Bank banks[2];
    std::atomic<int> live_bank{0};

    const Bank& liveBank() const { return banks[live_bank.load()]; }
    Bank bankCopy() const { return banks[live_bank.load()]; }

    void publishBank(const Bank& b)
    {
        int n = 1 - live_bank.load();
        banks[n] = b;
        live_bank.store(n);
    }

    uint8_t mute = 0;                    // bitmask, bit 6 = accent lane; UI writes, audio reads (single byte)
    int memory_slot = 0;                 // button-selected 0..15 (persisted)
    int trigger_length_index = 0;        // index into triggerLengths() (ArpSeq's list), persisted
    bool chance_mode = false;            // the grid draws pads as chance bars and a vertical drag sets them; UI only, persisted
    uint8_t random_lanes = 0x7F;         // which lanes RND and Randomize rewrite, bit l = lane l; all seven by default, persisted

    // TWEAK (VXDrumSequencerTweak.hpp, experiment 2026-09-08): the CV picks a
    // level 0..8 and the chosen lanes play a deterministic evolution of the
    // stored pattern. `tweak_lanes` UI writes / audio reads (persisted);
    // `tweak_level` audio writes from the jack every sample / UI reads for the
    // grid; `tweak_cache` is the head's tweaked copy, rebuilt on each clock
    // edge, reset and implicit reset (the samples on which a memory is
    // read), audio only.
    uint8_t tweak_lanes = 0x7F;
    int tweak_level = 0;
    Memory tweak_cache;
    // Randomize requests, audio -> UI (the widget's step() drains them): bit l
    // (0..6) = "randomize lane l" from channel l+1 of the RND jack; bit 7 =
    // "as the button" (the RND button, or a MONO cable on the jack), which
    // rewrites the lanes chosen in Randomize settings. One undo step per frame.
    static const int RANDOM_AS_BUTTON = 0x80;
    std::atomic<int> random_request{0};
    dsp::SchmittTrigger random_input_triggers[LANES];

    // What the UI reads (chain-foundations.md §4): the playhead, the last
    // fire, and the effective memory. Audio writes, UI reads. NOT persisted:
    // after load, step 1 is current and nothing has fired (the load is an
    // implicit reset, rack-port-design.md §4.2).
    Report report;

    // ── The engine ───────────────────────────────────────────────────────────
    Sequencer seq;

    dsp::SchmittTrigger clock_trigger;   // NEVER .reset() after construction (§1.2, §6.c)
    dsp::SchmittTrigger reset_trigger;

    dsp::BooleanTrigger memory_button_triggers[SLOTS];
    dsp::BooleanTrigger rewind_button_trigger;
    dsp::BooleanTrigger random_button_trigger;

    float last_sample_rate = 0.f;        // for rescaling the measured period across a rate change

    // ── The chain ────────────────────────────────────────────────────────────
    //
    // Resolved from adjacency on every process() call (a walk of at most
    // MAX_CHAIN pointers) and in onExpanderChange. `index` is structure, valid
    // for the frame it was computed in.
    //
    static const int MAX_CHAIN = 16;

    struct ChainLink
    {
        VXDrumSequencer* head = nullptr;              // nullptr: I am the head
        int index = 0;                                // 0 = head, 1.. = position in the chain
        int length = 1;                               // members, head included (a follower copies the head's)
        VXDrumSequencer* members[MAX_CHAIN] = {};     // head only: members[0] == self

        bool isHead() const { return head == nullptr; }
    };
    ChainLink chain;

    // Head-only telemetry the followers copy. `fired_member` tags the last
    // fire with the member it belonged to, so a follower flashes only its own
    // fires and shows the playhead only while it is active.
    struct ChainReport
    {
        int active = 0;                  // the member playing now
        int fired_member = 0;            // the member whose step fired last
        int position = -1;               // the engine's playhead
        uint32_t fired_mask = 0;
        uint32_t fired_serial = 0;
    };
    ChainReport chain_report;

    // The head's output voltages, mirrored by every member. Two slots indexed
    // by frame parity: the head writes slot (frame & 1) during its process(),
    // a member reads slot ((frame + 1) & 1), which the head wrote LAST frame
    // and does not touch this frame. Rack may run one frame's modules on
    // several workers, so reading the current slot could catch a half-written
    // sample or, sampled twice, skip the one-sample gap between two pulses; a
    // fixed one-sample delay has neither problem. The engine's frame counter
    // is the same for every module in a frame (ProcessArgs::frame).
    float chain_out[2][LANES] = {};

    int chain_active = 0;                // head only: the member playing now
    bool handoff_pending = false;        // head only: the active member fired its last step; switch on the next clock
    bool was_follower = false;           // to rewind the engine once on becoming a follower
    bool loaded = false;                 // the first process() call is the load's implicit reset (step 1 current, window armed, nothing fires); Initialize clears it so the same path runs again

    // const to keep the binding signature; Port::isConnected() is a non-const
    // member in the SDK (PianoRoll.hpp:490-491 notes the same), so a read-only
    // const_cast is the only way to ask a const module about its cables.
    bool memCvConnected() const
    {
        return const_cast<Input&>(inputs[MEM_CV_INPUT]).isConnected();
    }

    // Trigger pulse lengths in seconds, ArpSeq's list (ArpSeq.hpp:120); index 0
    // = 1 ms, the VCV trigger standard (brief-clock-reset §7.1).
    static const std::vector<float>& triggerLengths()
    {
        static const std::vector<float> lengths = {0.001f, 0.002f, 0.005f, 0.010f, 0.020f, 0.050f, 0.100f, 0.200f};
        return lengths;
    }

    VXDrumSequencer()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < SLOTS; i++)
        {
            configButton(MEMORY_PARAMS + i, "Memory " + std::to_string(i + 1));
        }

        configButton(REWIND_PARAM, "Rewind");
        configButton(RANDOM_PARAM, "Random pattern");

        configInput(CLOCK_INPUT, "Clock (one pulse per step)");
        configInput(RESET_INPUT, "Reset (jumps to step 1 and plays it; the next clock is step 2)");
        configInput(MEM_CV_INPUT, "Memory select CV (0-10 V = memory 1-16; overrides the buttons)");
        configInput(TWEAK_INPUT, "Tweak (0-10 V = level 0-8: evolves the chosen lanes, 0 V = the pattern as drawn)");
        configInput(RANDOM_INPUT, "Randomize (trigger; mono = the whole pattern like RND, poly = channel 1-6 randomizes that voice's lane, 7 the accent lane)");
        configOutput(TRIG_OUTPUT, "Triggers (poly: 1 BD, 2 SD, 3 CP, 4 PERC, 5 CH, 6 OH)");
        configOutput(ACC_OUTPUT, "Accent (10 V pulse with every accented hit)");

        Bank b;
        seedBank(b);
        publishBank(b);

        seq.rng.seed(random::u32());   // the chance rolls; a different stream per instance
    }

    // ── Chain resolution ─────────────────────────────────────────────────────

    static VXDrumSequencer* asSequencer(Module* m)
    {
        return (m && m->model == modelVXDrumSequencer) ? static_cast<VXDrumSequencer*>(m) : nullptr;
    }

    // Walk left to find the head (and my index); if I am the head, walk right
    // to list the members. Bounded by MAX_CHAIN either way; a longer run of
    // modules is simply not part of the chain.
    void resolveChain()
    {
        chain.head = nullptr;
        chain.index = 0;
        for (VXDrumSequencer* m = asSequencer(leftExpander.module); m && chain.index < MAX_CHAIN - 1;
             m = asSequencer(m->leftExpander.module))
        {
            chain.head = m;
            chain.index++;
        }

        chain.length = 1;
        chain.members[0] = this;
        if (chain.isHead())
        {
            for (VXDrumSequencer* m = asSequencer(rightExpander.module); m && chain.length < MAX_CHAIN;
                 m = asSequencer(m->rightExpander.module))
            {
                chain.members[chain.length++] = m;
            }
        }
    }

    void onExpanderChange(const ExpanderChangeEvent& e) override
    {
        resolveChain();
    }

    // The memory and mute that `member` contributes this sample: its effective
    // slot (its MEM CV or buttons, as it computed in its own process()) and its
    // mute mask. Both are single aligned words; a one-sample stale read is the
    // accepted cost.
    static PlaySource sourceOf(const VXDrumSequencer* member)
    {
        PlaySource src;
        const int slot = rack::math::clamp(member->report.slot, 0, SLOTS - 1);
        src.memory = &member->liveBank().memories[slot];
        src.mute = member->mute;
        return src;
    }

    // ── process() — brief-clock-reset §5.3 with the reset rule of
    //    rack-port-design.md §4.2, in this order, every sample ─────────────
    void process(const ProcessArgs& args) override
    {
        const float sample_rate = args.sampleRate;

        // The standard's 1 ms clock-ignore window, in samples at the live
        // rate, computed afresh each time it is armed and never cached, so
        // onSampleRateChange has no part in it (rack-port-design.md §4.1 row 6).
        const int window_samples = vgLib_v2::ResetClockWindow::samplesFor(sample_rate);

        resolveChain();

        // 1. Buttons. Memory buttons: CV locks them (presses ignored while MEM
        //    is patched). Every member does this: its slot is its own.
        const bool mem_cv = memCvConnected();
        for (int i = 0; i < SLOTS; i++)
        {
            if (memory_button_triggers[i].process(params[MEMORY_PARAMS + i].getValue() > 0.5f))
            {
                if (!mem_cv) memory_slot = i;
            }
        }

        // 1b. TWEAK level from the jack, every member for itself (the head
        //     reads a member's level the way it reads its slot). Unpatched = 0.
        tweak_level = inputs[TWEAK_INPUT].isConnected()
            ? tweakLevelFromVolts(inputs[TWEAK_INPUT].getVoltage(), tweak_level) : 0;

        // 2. Effective slot (source: ov_selector, core_head.c:624 — truncation, clamped).
        if (mem_cv)
        {
            const int cv_slot = (int)((inputs[MEM_CV_INPUT].getVoltage() / 10.f) * (float)SLOTS);
            report.slot = rack::math::clamp(cv_slot, 0, SLOTS - 1);
        }
        else
        {
            report.slot = rack::math::clamp(memory_slot, 0, SLOTS - 1);
        }

        for (int i = 0; i < SLOTS; i++)
        {
            lights[MEMORY_LIGHTS + i].setBrightness(i == report.slot ? 1.f : 0.f);
        }

        const bool rewind_pressed = rewind_button_trigger.process(params[REWIND_PARAM].getValue() > 0.5f);
        if (random_button_trigger.process(params[RANDOM_PARAM].getValue() > 0.5f))
        {
            random_request.fetch_or(RANDOM_AS_BUTTON);   // the widget does the undo-wrapped randomize on the UI thread
        }
        // The RND jack: a mono cable is the button; a poly cable addresses the
        // lanes, channel c = lane c (1 BD .. 6 OH, 7 accent). Channels beyond
        // the cable's count never fire; the Schmitts use the house thresholds.
        {
            const int channels = inputs[RANDOM_INPUT].getChannels();
            if (channels <= 1)
            {
                if (random_input_triggers[0].process(inputs[RANDOM_INPUT].getVoltage(),
                                                     constants::gate_low_trigger, constants::gate_high_trigger))
                    random_request.fetch_or(RANDOM_AS_BUTTON);
            }
            else
            {
                const int n = std::min(channels, LANES);
                for (int c = 0; c < n; c++)
                {
                    if (random_input_triggers[c].process(inputs[RANDOM_INPUT].getVoltage(c),
                                                         constants::gate_low_trigger, constants::gate_high_trigger))
                        random_request.fetch_or(1 << c);
                }
            }
        }

        // 3. A follower: no transport, no outputs. Copy the head's telemetry
        //    for my grid and stop.
        if (!chain.isHead())
        {
            processAsFollower(args);
            return;
        }

        // True when this sample is an IMPLICIT reset (steps 3 and 4 below, and
        // the one Initialize asks for by clearing `loaded`): step 1 current,
        // nothing fires. Step 7 reads it to decide the hand-off the way it
        // does for a fire. A head one step long is put on its LAST step by an
        // implicit reset, so the next accepted clock is member 1's step 1, not
        // a replay of the head's; without this the chain sat one step behind a
        // flat sequencer of the same total length after every patch load
        // until the next explicit reset (chain-foundations.md §1: members
        // "play one after another as a single longer pattern").
        bool implicit_reset = false;
        if (!loaded)
        {
            // Patch load / module add is an IMPLICIT reset (Impromptu's rule,
            // rack-port-design.md §4.2): step 1 current, the window armed,
            // nothing fires, so the first clock plays step 2. Armed here on
            // the first process() call, not in the constructor, because the
            // window length needs the engine's live sample rate.
            loaded = true;
            implicit_reset = true;
            seq.rewind(window_samples);
        }
        if (was_follower)
        {
            // Just became the head (the module to my left went away): an
            // implicit reset rather than a stale playhead.
            was_follower = false;
            implicit_reset = true;
            seq.rewind(window_samples);
        }

        // 4. A member left the chain from under the playhead: an implicit
        //    reset to the head's step 1. A clock edge on this same sample is
        //    inside the window just armed and does not step.
        if (chain_active >= chain.length)
        {
            chain_active = 0;
            handoff_pending = false;
            implicit_reset = true;
            seq.rewind(window_samples);
        }

        // 5. Rewind button (source: vxdrums_fire_rewind, :542-549) and RESET
        //    input: the VCV standard (rack-port-design.md §4.1-§4.3, SEQ3's
        //    order, reset BEFORE the clock). Either one returns the chain to
        //    its head and, once the head's source is resolved in step 7,
        //    plays the head's step 1 on THIS sample through seq.reset(). A
        //    clock edge on this sample is the reset's own: the engine
        //    swallows it inside the window, so the next clock plays step 2.
        //    A reset while the clock line is high sees no edge (the Schmitt
        //    is never reset(): a high line is high, not an edge) and the next
        //    rising edge is step 2's.
        const bool reset_edge = reset_trigger.process(inputs[RESET_INPUT].getVoltage(),
                                                      constants::gate_low_trigger, constants::gate_high_trigger);
        const bool reset_now = rewind_pressed || reset_edge;
        if (reset_now)
        {
            chain_active = 0;
            handoff_pending = false;
        }

        // 6. CLOCK input — poll the Schmitt EVERY sample (§1.2). A pending
        //    hand-off is taken on the edge, before the advance, so the member
        //    that fired its last step keeps its lamp lit for the whole step and
        //    the next member's step 1 lands on this very clock. Never on a
        //    sample that reset: step 5 cleared the pending hand-off, so the
        //    reset's own clock cannot carry the chain past the head. And only
        //    on an edge the engine will honour: an edge inside the reset window
        //    is swallowed whole (SEQ3's `clockTriggered && !resetGate`), so it
        //    takes no hand-off either; the pending flag survives and the next
        //    accepted clock takes it. Otherwise a head one step long (its
        //    reset fire is also its last step) would hand off on the master's
        //    late clock a sample after the reset and every grid would show
        //    position -1 until the next clock.
        const bool clock_edge = clock_trigger.process(inputs[CLOCK_INPUT].getVoltage(),
                                                      constants::gate_low_trigger, constants::gate_high_trigger);
        if (clock_edge && handoff_pending && !seq.window.open())
        {
            handoff_pending = false;
            chain_active = (chain_active + 1) % chain.length;
            seq.handOff();
        }

        // 7. The engine: this sample's source is the active member's effective
        //    memory and mute — or, with TWEAK up, a tweaked copy of it, rebuilt
        //    on the clock edge, the reset, or the implicit reset that is about
        //    to read it (an implicit reset reads only its length, for the
        //    hand-off below, but on the load sample the cache has never been
        //    built). A reset plays step 1 now, through seq.reset() BEFORE
        //    seq.process(), which then finds the window open and drops this
        //    sample's clock edge.
        VXDrumSequencer* member = chain.members[chain_active];
        PlaySource src = sourceOf(member);
        if (member->tweak_level > 0 && member->tweak_lanes)
        {
            if (clock_edge || reset_now || implicit_reset) tweak_cache = tweakMemory(*src.memory, member->tweak_lanes, member->tweak_level);
            src.memory = &tweak_cache;
        }
        const int index = rack::math::clamp(trigger_length_index, 0, (int)triggerLengths().size() - 1);
        const int64_t pulse_len = std::max((int64_t)1, (int64_t)(sample_rate * triggerLengths()[index]));

        float out[LANES];
        if (reset_now) seq.reset(src, window_samples);
        seq.process(clock_edge, src, sample_rate, pulse_len, out);

        // A reset's fire counts exactly like a clocked one from here on: the
        // lamp and the serial.
        if (seq.fired_this_sample)
        {
            chain_report.fired_member = chain_active;
            chain_report.fired_mask = seq.report.fired_mask;
            chain_report.fired_serial = seq.report.fired_serial;
        }

        // The hand-off. The active member is on its last step, because it just
        // PLAYED it (a clocked advance, or an explicit reset on a one-step
        // head) or because an implicit reset just PUT it there (a one-step
        // head after load, after becoming the head, after a member left): the
        // next accepted clock belongs to the next member. Deciding it here for
        // the implicit case keeps the chain in step with a flat sequencer
        // whose first clock after load is step 2, i.e. member 1's step 1. A
        // lone sequencer wraps as it always has.
        if ((seq.fired_this_sample || implicit_reset)
            && chain.length > 1 && src.memory && seq.head.position >= src.memory->length - 1)
        {
            handoff_pending = true;
        }
        chain_report.active = chain_active;
        chain_report.position = seq.head.position;

        // My own grid, through the same rule a follower applies.
        applyChainReport(chain_report, 0);

        // 8. Trigger outputs, and this frame's slot of the mirror.
        for (int lane = 0; lane < LANES; lane++) chain_out[args.frame & 1][lane] = out[lane];
        writeOutputs(out);
    }

    // EVERY sample: setChannels() is a no-op while the port is disconnected
    // (PianoRoll.hpp:670-675), so a one-shot push would be lost.
    void writeOutputs(const float out[LANES])
    {
        outputs[TRIG_OUTPUT].setChannels(VOICES);
        for (int lane = 0; lane < VOICES; lane++) outputs[TRIG_OUTPUT].setVoltage(out[lane], lane);
        outputs[ACC_OUTPUT].setVoltage(out[ACCENT_LANE]);
    }

    // A follower's frame: the head's outputs of last frame on my jacks, the
    // head's telemetry for my grid.
    void processAsFollower(const ProcessArgs& args)
    {
        if (!was_follower)
        {
            was_follower = true;
            seq.rewind(vgLib_v2::ResetClockWindow::samplesFor(args.sampleRate));
            seq.shaper.reset();
            chain_active = 0;
            handoff_pending = false;
        }

        writeOutputs(chain.head->chain_out[(args.frame + 1) & 1]);

        chain.length = chain.head->chain.length;   // for the menu's status line only
        applyChainReport(chain.head->chain_report, chain.index);
    }

    // The playhead shows only while I am the active member; a fire is mine
    // only when it is tagged with my index. Holding the serial otherwise keeps
    // the grid from flashing another member's hits.
    void applyChainReport(const ChainReport& cr, int my_index)
    {
        report.position = (cr.active == my_index) ? cr.position : -1;
        if (cr.fired_member == my_index)
        {
            report.fired_mask = cr.fired_mask;
            report.fired_serial = cr.fired_serial;
        }
    }

    // ── Lifecycle ────────────────────────────────────────────────────────────

    void onSampleRateChange(const SampleRateChangeEvent& e) override
    {
        seq.clock.rescale(last_sample_rate, e.sampleRate);
        last_sample_rate = e.sampleRate;
    }

    // Initialize: SEED in memory 1, everything else empty, no mutes, memory 1
    // selected, 1 ms triggers, the transport on step 1 (an implicit reset:
    // nothing fires); then the params via the base. The window length comes
    // from the engine's current rate: this runs on the UI thread, with no
    // ProcessArgs in hand (Ghosts.hpp:286 idiom).
    void onReset(const ResetEvent& e) override
    {
        Bank b;
        seedBank(b);
        publishBank(b);
        mute = 0;
        memory_slot = 0;
        trigger_length_index = 0;
        chance_mode = false;
        random_lanes = 0x7F;
        tweak_lanes = 0x7F;
        tweak_level = 0;
        seq.rewind(vgLib_v2::ResetClockWindow::samplesFor(APP->engine->getSampleRate()));
        chain_active = 0;
        handoff_pending = false;
        loaded = false;                  // the next process() call repeats the rewind (idempotent) and decides the hand-off for a one-step head
        Module::onReset(e);
    }

    // Randomize: a new beat into the CURRENT (effective) memory, then the params
    // like every other module. Rack wraps the whole thing in a ModuleChange, so
    // it is undoable without a bank action.
    void onRandomize(const RandomizeEvent& e) override
    {
        Bank b = bankCopy();
        randomizeMemory(b.memories[rack::math::clamp(report.slot, 0, SLOTS - 1)], random_lanes);
        publishBank(b);
        Module::onRandomize(e);
    }

    // ── Persistence (DESIGN §4.5) ────────────────────────────────────────────
    //
    // Raw jansson, json_object_set_new / json_array_append_new EXCLUSIVELY
    // (PianoRoll.hpp:711-717). Params are serialized by Rack. The playhead, the
    // clock period and the chain state are deliberately not persisted: step 1
    // current (the load is an implicit reset) / unknown / head after load.
    //
    //   { "version": "1.1.0",
    //     "memories": [ { "steps": [7 lane objects], "length": 16 }, ... 16 ],
    //     "mute": 0, "memory_slot": 0, "trigger_length_index": 0, "chance_mode": false,
    //     "random_lanes": 127, "tweak_lanes": 127 }
    //
    // The memory body is the shared shape in VXDrumSequencerPattern.hpp
    // (memoryBodyToJson / memoryBodyFromJson), which also reads the 1.0.0
    // bit-mask form, so patches saved before 2026-09-07 load unchanged.
    //
    json_t* dataToJson() override
    {
        json_t* root = json_object();
        json_object_set_new(root, "version", json_string("1.1.0"));

        // UI thread, and the UI is the bank's only writer, so the live bank is
        // consistent here.
        const Bank& bank = liveBank();

        json_t* memories = json_array();
        for (int s = 0; s < SLOTS; s++)
        {
            json_t* memory = json_object();
            memoryBodyToJson(memory, bank.memories[s]);
            json_array_append_new(memories, memory);
        }
        json_object_set_new(root, "memories", memories);

        json_object_set_new(root, "mute", json_integer(mute));
        json_object_set_new(root, "memory_slot", json_integer(memory_slot));
        json_object_set_new(root, "trigger_length_index", json_integer(trigger_length_index));
        json_object_set_new(root, "chance_mode", json_boolean(chance_mode));
        json_object_set_new(root, "random_lanes", json_integer(random_lanes));
        json_object_set_new(root, "tweak_lanes", json_integer(tweak_lanes));

        return root;
    }

    void dataFromJson(json_t* root) override
    {
        if (!root) return;

        // A fresh seeded bank, overwritten by whatever the patch carries, with
        // every loop bounded (memories capped at 16; the body loader caps lanes
        // and steps and clamps values). A hand-edited patch can neither
        // allocate without limit nor index past the grid. This also runs on
        // module paste / preset load into a live module; the publish is the
        // atomic swap, and the transport is left alone exactly as PianoRoll
        // leaves its position (PianoRoll.hpp:756-862).
        Bank b;
        seedBank(b);

        json_t* memories = json_object_get(root, "memories");
        if (json_is_array(memories))
        {
            const size_t count = std::min(json_array_size(memories), (size_t)SLOTS);
            for (size_t s = 0; s < count; s++)
            {
                json_t* memory = json_array_get(memories, s);
                if (!json_is_object(memory)) continue;

                Memory m;   // every pad off, single, 100; length 16
                memoryBodyFromJson(memory, m);
                b.memories[s] = m;
            }
        }

        publishBank(b);

        // Probe every key: a missing key keeps the constructor default rather
        // than becoming zero (PianoRoll.hpp:764-767).
        json_t* mute_json = json_object_get(root, "mute");
        if (json_is_integer(mute_json)) mute = (uint8_t)(json_integer_value(mute_json) & 0x7F);

        json_t* slot_json = json_object_get(root, "memory_slot");
        if (json_is_integer(slot_json)) memory_slot = rack::math::clamp((int)json_integer_value(slot_json), 0, SLOTS - 1);

        json_t* trigger_length_json = json_object_get(root, "trigger_length_index");
        if (json_is_integer(trigger_length_json))
        {
            trigger_length_index = rack::math::clamp((int)json_integer_value(trigger_length_json),
                                                     0, (int)triggerLengths().size() - 1);
        }

        json_t* chance_mode_json = json_object_get(root, "chance_mode");
        if (json_is_boolean(chance_mode_json)) chance_mode = json_boolean_value(chance_mode_json);

        json_t* random_lanes_json = json_object_get(root, "random_lanes");
        if (json_is_integer(random_lanes_json)) random_lanes = (uint8_t)(json_integer_value(random_lanes_json) & 0x7F);

        json_t* tweak_lanes_json = json_object_get(root, "tweak_lanes");
        if (json_is_integer(tweak_lanes_json)) tweak_lanes = (uint8_t)(json_integer_value(tweak_lanes_json) & 0x7F);
    }
};

// ── Undo (DESIGN §4.7, house idiom from PianoRollEditorWidget.hpp:43-61) ─────
//
// Stores the whole bank before/after plus the mute mask (mute edits travel in
// the same action type). Actions must not hold module pointers: undoing a
// module delete recreates it with the same id, so a stored pointer would
// dangle. Resolve through APP->engine->getModule(moduleId) each time.
//
// Usage (UI thread): apply first (module->publishBank(after); module->mute = ...),
// then APP->history->push(action). Set `name` per gesture ("VX Drum Sequencer
// edit" / "paste memory" / "clear memory" / "random pattern" / "mute" /
// "length" / "ratchet"). Skip the push when isNoop().
//
struct VXDrumSequencerBankAction : history::ModuleAction
{
    Bank before;
    Bank after;
    uint8_t mute_before = 0;
    uint8_t mute_after = 0;

    VXDrumSequencerBankAction()
    {
        name = "VX Drum Sequencer edit";
    }

    bool isNoop() const
    {
        return mute_before == mute_after && before == after;   // elementwise, see VXDrumSequencerTypes.hpp
    }

    void apply(const Bank& bank, uint8_t mute)
    {
        VXDrumSequencer* m = dynamic_cast<VXDrumSequencer*>(APP->engine->getModule(moduleId));
        if (!m) return;
        m->publishBank(bank);
        m->mute = mute;
    }

    void undo() override { apply(before, mute_before); }
    void redo() override { apply(after, mute_after); }
};
