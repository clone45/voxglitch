#pragma once
//
// VX Drum Sequencer — a 16-step x 6-lane trigger sequencer with an accent lane,
// per-step ratchets and sixteen pattern memories, driven by an external clock.
//
// The sequencer half of the vxsynth "VX Drum Machine" (vxdrums.c). The voices
// live in the VXDrums kit module; the two panels are linked by a polyphonic
// TRIG cable (channel c = voice c: BD SD CP PERC CH OH) and a separate mono ACC
// gate cable. See:
//   docs/modules/vx-drum-sequencer/rack-port-design.md   <- what diverges from vxsynth and why
//   scratchpad brief-clock-reset.md §5-§7               <- the clock/reset algorithm this follows
//
// FIRST-STEP CONVENTION (brief-clock-reset §3, §5): advance-then-play from
// position -1. Every clock does position = (position + 1) % len, then fires.
// "Has anything played yet?" is position >= 0. There is NO clock-ignore window
// after a reset and the clock Schmitt is NEVER reset (): the clock edge that
// accompanies a reset IS the step-0 edge. Swallowing it is the #278 defect
// (ArpSeq 30dcc68) and the source's own 1 ms window bug (vxdrums.c:593).
//
// External clock only (owner decision, 2026-09-02): the source's internal
// clock, RUN transport and swing are not ported. Song mode (clockmode 2) is
// not ported either: Rack has no host playhead.
//
// THREADS: the pattern Bank is UI-owned and double-buffered for the audio
// thread (PianoRoll.hpp:107-129 idiom). Widgets mutate a copy and publish; the
// audio thread takes ONE reference per process() call. `mute`, `memory_slot`,
// `current_slot`, `position`, `fired_*` and `trigger_length_index` are single
// aligned words shared across threads, accepted as-is (PianoRoll.hpp:157-159).
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
    int current_slot = 0;                // effective slot this sample (MEM CV override else memory_slot); audio writes
    int position = -1;                   // current step, -1 = nothing played since load/reset/rewind; NOT persisted
    uint32_t fired_mask = 0;             // lanes struck at the last step fire, bit 6 = accent lamp (source report.fired)
    uint32_t fired_serial = 0;           // incremented on every step fire; widgets flash lanes when it changes
    int trigger_length_index = 0;        // index into triggerLengths() (ArpSeq's list), persisted
    std::atomic<bool> random_request{false};   // audio sets on RANDOM_PARAM edge; the widget's step() consumes it

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

    // ── Engine state (brief-clock-reset §5.1) ────────────────────────────────

    dsp::SchmittTrigger clock_trigger;   // NEVER .reset() after construction (§1.2, §6.c)
    dsp::SchmittTrigger reset_trigger;

    dsp::BooleanTrigger memory_button_triggers[SLOTS];
    dsp::BooleanTrigger rewind_button_trigger;
    dsp::BooleanTrigger random_button_trigger;

    // Clock period measurement (samples), the sync-effects convention (§6.j).
    // int64_t, not the house `long`: `long` is 32-bit under MinGW-w64 and, with
    // no clock patched, nothing zeroes this counter — it would overflow (UB)
    // after 2^31 samples, 3-13 h depending on the rate.
    int64_t clock_sample_counter = 0;    // samples since the last DETECTED edge (accepted or not)
    bool clock_anchor_valid = false;     // false after load/reset/rewind: the next edge anchors, does not measure
    float step_samples = 0.f;            // measured clock period, 0 = unknown (only at load). Survives reset.
    float last_sample_rate = 0.f;        // for rescaling step_samples across a rate change

    // Ratchet re-strikes in flight, per voice lane (source: ratLeft/ratInterval/ratCount/ratAmp)
    int rat_left[VOICES] = {};
    long rat_interval[VOICES] = {};
    long rat_count[VOICES] = {};
    float rat_amp[VOICES] = {};
    bool rat_accented[VOICES] = {};      // so a re-strike of an accented step re-raises ACC (DESIGN §2)

    // Trigger outputs, per lane, index 6 = ACC (brief-clock-reset §7.2).
    uint32_t strike_mask = 0;            // lanes struck THIS sample; resolved into pulses by writeTriggerOutputs()
    long pulse_remaining[LANES] = {};    // samples of pulse left
    bool gap_sample[LANES] = {};         // force 0 V this sample (the re-strike gap)
    bool was_high[LANES] = {};           // output was > 0 V on the previous sample (the gap rule keys on this, not on the countdown)

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
        configInput(RESET_INPUT, "Reset (restarts the pattern; the next clock plays step 1)");
        configInput(MEM_CV_INPUT, "Memory select CV (0-10 V = memory 1-16; overrides the buttons)");
        configOutput(TRIG_OUTPUT, "Triggers (poly: 1 BD, 2 SD, 3 CP, 4 PERC, 5 CH, 6 OH)");
        configOutput(ACC_OUTPUT, "Accent (10 V pulse with every accented hit)");

        Bank b;
        seedBank(b);
        publishBank(b);
    }

    // ── Helpers (brief-clock-reset §5.2) ─────────────────────────────────────

    // The effective memory's length, 1..16 (source: vxd_len, vxdrums.c:335-338).
    int lengthOf(const Bank& bank) const
    {
        return rack::math::clamp(bank.memories[current_slot].length, 1, STEPS);
    }

    // Cut everything in flight (source: vxd_silence, vxdrums.c:424-430). Only
    // the ratchet re-strikes are cancelled. Trigger pulses already high are
    // left to finish (§6.k); voices ring out in the kit.
    void silence()
    {
        for (int l = 0; l < VOICES; l++) rat_left[l] = 0;
        fired_mask = 0;   // source :427 report.fired = 0 (the serial is untouched: nothing fired)
    }

    // Reset = rewind (source: vxd_reset_pos, :542-549; §6.h). NO ignore window,
    // NO clock_trigger.reset(): a clock line that is still high stays HIGH in
    // the Schmitt and cannot produce a phantom edge (§6.c), and an edge on this
    // very sample is still reported by step 5 (§6.b).
    void rewind()
    {
        silence();
        position = -1;
        clock_anchor_valid = false;      // ArpSeq.hpp:879-881: the next edge anchors without measuring (§6.d)
        clock_sample_counter = 0;        // the counter, NOT step_samples (PianoRoll.hpp:629-631; rack-port-design.md:206-208)
    }

    // Record a strike on `lane` for this sample; an accented strike also raises
    // the ACC lane. The pulses themselves are resolved once per sample in
    // writeTriggerOutputs(), so that every lane struck on one sample rises
    // together (see the gap rule there). `amp` is the amplitude the source
    // struck its voice with (vxd_fire :374); the sequencer emits 10 V triggers
    // and the kit applies the accent boost from the ACC gate (DESIGN §2), so it
    // is carried for source parity only.
    void strike(int lane, float amp, bool accented)
    {
        (void)amp;
        strike_mask |= (1u << lane);
        if (accented) strike_mask |= (1u << ACCENT_LANE);
    }

    // Fire the triggers for step `pos` NOW (source: vxd_fire, vxdrums.c:369-396).
    // Strikes every unmuted voice lane whose bit is set and arms its ratchet
    // re-strikes. Writes fired_mask / fired_serial — step fires only, never
    // re-strikes (source report.fired).
    void fire(const Bank& bank, int pos)
    {
        const Memory& m = bank.memories[current_slot];

        uint32_t fired = 0;

        // A muted accent lane removes the boost AND the accent lamp bit (:373-375).
        const bool acc_on = !(mute & (1u << ACCENT_LANE)) && ((m.lanes[ACCENT_LANE] >> pos) & 1u);
        const float amp = acc_on ? 2.f : 1.f;   // source: 1.0 + m_accent (accent lives in the kit)
        if (acc_on)
        {
            fired |= (1u << ACCENT_LANE);
            strike_mask |= (1u << ACCENT_LANE);   // ACC rises on the step fire of an accented step (DESIGN §2)
        }

        for (int l = 0; l < VOICES; l++)
        {
            if (mute & (1u << l)) continue;               // a muted lane neither strikes nor pulses (:379)
            if (!((m.lanes[l] >> pos) & 1u)) continue;

            strike(l, amp, acc_on);
            fired |= (1u << l);

            // Ratchet: 2 bits per step -> 0..3 extra hits, evenly subdividing the
            // MEASURED clock period (source: vxd_step_samples). Without a known
            // period (before the second clock edge, §6.e) the extra hits are
            // dropped rather than guessed (:383-393).
            const int extra = (int)((m.ratchets[l] >> (pos * 2)) & 3u);
            if (extra > 0 && step_samples > 4.f)
            {
                rat_left[l] = extra;
                rat_interval[l] = std::max(2L, (long)(step_samples / (float)(extra + 1)));
                rat_count[l] = rat_interval[l];
                rat_amp[l] = amp;
                rat_accented[l] = acc_on;
            }
        }

        fired_mask = fired;
        fired_serial++;
    }

    // A step boundary was reached (source: vxd_step_at, vxdrums.c:399-415,
    // minus swing): move the playhead and fire.
    void stepAt(const Bank& bank, int pos)
    {
        const int len = lengthOf(bank);
        position = (pos < 0 || pos >= len) ? 0 : pos;
        fire(bank, position);
    }

    // Advance one step (source: vxd_step, vxdrums.c:417-421). From -1 this
    // lands on step 0: the first clock after load/reset plays step 1. Named
    // advance, not step: a step(const Bank&) would hide the SDK's deprecated
    // engine::Module::step() (clang -Woverloaded-virtual on the macOS build).
    void advance(const Bank& bank)
    {
        int next = position + 1;
        if (next >= lengthOf(bank)) next = 0;
        stepAt(bank, next);
    }

    // ── Trigger outputs (brief-clock-reset §7.2) ─────────────────────────────
    //
    // 10 V pulses of triggerLengths()[trigger_length_index] seconds, as sample
    // countdowns (Timeline.hpp:101-105 form, not dsp::PulseGenerator — whose
    // trigger() only EXTENDS a pulse and can never emit a second edge inside one,
    // digital.hpp:189-194, which is why GrooveBox ratchets merge at the expander).
    //
    // GAP RULE: a strike on a lane whose output was HIGH on the previous sample
    // forces exactly ONE sample at 0 V, then a fresh pulse (one sample is enough
    // for every Rack Schmitt and survives the >= 2-sample ratchet floor). It keys
    // on the previous sample's OUTPUT (was_high), not on the countdown: a pulse
    // of pulse_len samples has pulse_remaining == 0 on its last high sample, so a
    // strike landing exactly pulse_len after the previous one (100 ms triggers
    // with a clock whose period equals the pulse) would otherwise go
    // 10 V -> 10 V and merge every hit into one long pulse. The rule is applied
    // to the sample's whole strike SET at once: if any struck lane (ACC included)
    // needs the gap, every struck lane takes it, so the ACC gate and the trigger
    // it accents always rise on the SAME sample — the kit samples ACC at the
    // trigger instant (DESIGN §2, VXDrums.hpp:203). Per-lane gaps would let a
    // still-high ACC lag its trigger by a sample with a long pulse length and
    // lose the accent.
    //
    // ACC CUT: an un-accented strike set ends any ACC gate still high from an
    // earlier accented strike on the sample its voice lanes RISE (this sample,
    // or the next one when the set takes the gap), so the kit reads 0 V at the
    // trigger edge. The source accents per step (vxdrums.c:373-374); with a
    // pulse longer than the clock period (200 ms above 75 BPM, 100 ms above
    // 150 BPM at one pulse per 16th) the gate would otherwise carry one step's
    // accent onto the next hit. Cutting at the rise rather than at the strike
    // keeps an accented set that took the gap on the previous sample rising
    // with ACC high. One case stays unresolvable: an accented lane and an
    // un-accented lane rising on the SAME sample (an accented re-strike landing
    // on an un-accented step fire, or one gap-delayed onto the other's fire).
    // The mono ACC cable of DESIGN §2 cannot serve both; the un-accented strike
    // wins and ACC is low.
    //
    void writeTriggerOutputs(float sample_rate)
    {
        if (strike_mask)
        {
            const int index = rack::math::clamp(trigger_length_index, 0, (int)triggerLengths().size() - 1);
            const long pulse_len = std::max(1L, (long)(sample_rate * triggerLengths()[index]));

            bool need_gap = false;
            for (int lane = 0; lane < LANES; lane++)
            {
                if ((strike_mask & (1u << lane)) && was_high[lane]) need_gap = true;
            }

            // ACC CUT (see above): with the ACC bit clear, at least one voice lane
            // is striking un-accented, so the gate must be low on the sample those
            // lanes RISE: now, or after the gap. With a gap the gate keeps this one
            // sample, so an accented set that took the gap on the previous sample
            // still rises with ACC high.
            if (!(strike_mask & (1u << ACCENT_LANE)) && pulse_remaining[ACCENT_LANE] > 0)
            {
                pulse_remaining[ACCENT_LANE] = need_gap ? 1 : 0;
            }

            for (int lane = 0; lane < LANES; lane++)
            {
                if (!(strike_mask & (1u << lane))) continue;
                gap_sample[lane] = need_gap;
                pulse_remaining[lane] = pulse_len + (need_gap ? 1 : 0);
            }

            strike_mask = 0;
        }

        // EVERY sample: setChannels() is a no-op while the port is disconnected
        // (PianoRoll.hpp:670-675), so a one-shot push would be lost.
        outputs[TRIG_OUTPUT].setChannels(VOICES);

        for (int lane = 0; lane < LANES; lane++)
        {
            float v;
            if (gap_sample[lane])
            {
                v = 0.f;
                gap_sample[lane] = false;
            }
            else
            {
                v = (pulse_remaining[lane] > 0) ? 10.f : 0.f;
            }
            was_high[lane] = v > 0.f;
            if (pulse_remaining[lane] > 0) pulse_remaining[lane]--;

            if (lane < VOICES) outputs[TRIG_OUTPUT].setVoltage(v, lane);
            else outputs[ACC_OUTPUT].setVoltage(v);
        }
    }

    // ── process() — brief-clock-reset §5.3, in this order, every sample ──────

    void process(const ProcessArgs& args) override
    {
        // ONE reference per call (the double-buffer contract, see the header).
        const Bank& bank = banks[live_bank.load()];
        const float sample_rate = args.sampleRate;

        // 1. Buttons. Memory buttons: CV locks them (presses ignored while MEM
        //    is patched).
        const bool mem_cv = memCvConnected();
        for (int i = 0; i < SLOTS; i++)
        {
            if (memory_button_triggers[i].process(params[MEMORY_PARAMS + i].getValue() > 0.5f))
            {
                if (!mem_cv) memory_slot = i;
            }
        }

        // 2. Effective slot (source: ov_selector, core_head.c:624 — truncation, clamped).
        if (mem_cv)
        {
            const int cv_slot = (int)((inputs[MEM_CV_INPUT].getVoltage() / 10.f) * (float)SLOTS);
            current_slot = rack::math::clamp(cv_slot, 0, SLOTS - 1);
        }
        else
        {
            current_slot = rack::math::clamp(memory_slot, 0, SLOTS - 1);
        }

        for (int i = 0; i < SLOTS; i++)
        {
            lights[MEMORY_LIGHTS + i].setBrightness(i == current_slot ? 1.f : 0.f);
        }

        const bool rewind_pressed = rewind_button_trigger.process(params[REWIND_PARAM].getValue() > 0.5f);
        if (random_button_trigger.process(params[RANDOM_PARAM].getValue() > 0.5f))
        {
            random_request.store(true);   // the widget does the undo-wrapped randomize on the UI thread
        }

        // 3. Rewind button (source: vxdrums_fire_rewind, :542-549; §6.h).
        if (rewind_pressed) rewind();

        // 4. RESET input — BEFORE the clock, so a same-sample reset+clock fires
        //    step 0 on this sample (§6.b; GrooveBox.hpp:1028/:1085). A reset while
        //    the clock line is high does not fire (§6.c). No window (§6.a, §6.d).
        if (reset_trigger.process(inputs[RESET_INPUT].getVoltage(),
                                  constants::gate_low_trigger, constants::gate_high_trigger))
        {
            rewind();
        }

        // 5. CLOCK input — poll the Schmitt EVERY sample (§1.2). Period
        //    measurement per §6.j; an edge always steps.
        clock_sample_counter++;

        const bool clock_edge = clock_trigger.process(inputs[CLOCK_INPUT].getVoltage(),
                                                      constants::gate_low_trigger, constants::gate_high_trigger);
        if (clock_edge)
        {
            // Measure only from a valid anchor, inside the sync-effects debounce
            // window (> 1 sample, < 4 s). The partial interval after a reset is
            // never taken as the period; step_samples keeps its last good value
            // so ratchets on step 0 still subdivide sensibly (§6.d).
            if (clock_anchor_valid
                && clock_sample_counter > 1
                && clock_sample_counter < (int64_t)(sample_rate * 4.f))
            {
                step_samples = (float)clock_sample_counter;
            }

            // ALWAYS zero the counter — measured or not (PianoRoll.hpp:646-652:
            // an edge that does not zero it makes the next measured edge report
            // two intervals).
            clock_sample_counter = 0;
            clock_anchor_valid = true;

            advance(bank);   // -1 -> 0 on the first edge after reset/load (§6.a, §6.e)
        }

        // 6. Ratchet re-strikes come due (source :674-682) — the same strike path
        //    as a step, so the ACC pulse rides along when the step was accented.
        //    In-flight re-strikes of a lane muted mid-step still fire (source parity).
        for (int l = 0; l < VOICES; l++)
        {
            if (rat_left[l] > 0 && --rat_count[l] <= 0)
            {
                strike(l, rat_amp[l], rat_accented[l]);
                rat_left[l]--;
                rat_count[l] = rat_interval[l];
            }
        }

        // 7. Voices: none here — the kit is VXDrums.

        // 8. Trigger outputs.
        writeTriggerOutputs(sample_rate);
    }

    // ── Lifecycle ────────────────────────────────────────────────────────────

    // Nothing in the clock path stores seconds as samples except the measured
    // clock period, which by design outlives its measurement (it survives
    // resets). Rescale it so ratchets stay right until the next edge
    // re-measures, and re-anchor: the sample counter now straddles two rates
    // and must not be read as a period. Ratchet intervals and pulse lengths
    // are computed from args.sampleRate at the moment they are armed.
    void onSampleRateChange(const SampleRateChangeEvent& e) override
    {
        if (last_sample_rate > 0.f && e.sampleRate > 0.f && e.sampleRate != last_sample_rate)
        {
            step_samples *= e.sampleRate / last_sample_rate;
            clock_anchor_valid = false;
            clock_sample_counter = 0;
        }
        last_sample_rate = e.sampleRate;
    }

    // Initialize: SEED in memory 1, everything else empty, no mutes, memory 1
    // selected, 1 ms triggers, transport rewound; then the params via the base.
    void onReset(const ResetEvent& e) override
    {
        Bank b;
        seedBank(b);
        publishBank(b);
        mute = 0;
        memory_slot = 0;
        trigger_length_index = 0;
        rewind();
        Module::onReset(e);
    }

    // Randomize: a new beat into the CURRENT (effective) memory, then the params
    // like every other module. Rack wraps the whole thing in a ModuleChange, so
    // it is undoable without a bank action.
    void onRandomize(const RandomizeEvent& e) override
    {
        Bank b = bankCopy();
        randomizeMemory(b.memories[rack::math::clamp(current_slot, 0, SLOTS - 1)]);
        publishBank(b);
        Module::onRandomize(e);
    }

    // ── Persistence (DESIGN §4.5) ────────────────────────────────────────────
    //
    // Raw jansson, json_object_set_new / json_array_append_new EXCLUSIVELY
    // (PianoRoll.hpp:711-717). Params are serialized by Rack. `position` and the
    // clock period are deliberately not persisted: -1 / unknown after load.
    //
    //   { "version": "1.0.0",
    //     "memories": [ { "lanes": [7 ints], "ratchets": [6 ints], "length": 16 }, ... 16 ],
    //     "mute": 0, "memory_slot": 0, "trigger_length_index": 0 }
    //
    json_t* dataToJson() override
    {
        json_t* root = json_object();
        json_object_set_new(root, "version", json_string("1.0.0"));

        // UI thread, and the UI is the bank's only writer, so the live bank is
        // consistent here.
        const Bank& bank = liveBank();

        json_t* memories = json_array();
        for (int s = 0; s < SLOTS; s++)
        {
            const Memory& m = bank.memories[s];
            json_t* memory = json_object();

            json_t* lanes = json_array();
            for (int l = 0; l < LANES; l++) json_array_append_new(lanes, json_integer((json_int_t)m.lanes[l]));
            json_object_set_new(memory, "lanes", lanes);

            json_t* ratchets = json_array();
            for (int l = 0; l < VOICES; l++) json_array_append_new(ratchets, json_integer((json_int_t)m.ratchets[l]));
            json_object_set_new(memory, "ratchets", ratchets);

            json_object_set_new(memory, "length", json_integer(m.length));
            json_array_append_new(memories, memory);
        }
        json_object_set_new(root, "memories", memories);

        json_object_set_new(root, "mute", json_integer(mute));
        json_object_set_new(root, "memory_slot", json_integer(memory_slot));
        json_object_set_new(root, "trigger_length_index", json_integer(trigger_length_index));

        return root;
    }

    void dataFromJson(json_t* root) override
    {
        if (!root) return;

        // A fresh seeded bank, overwritten by whatever the patch carries, with
        // every loop bounded (arrays capped at 16/7/6, length clamped 1..16). A
        // hand-edited patch can neither allocate without limit nor index past
        // the grid. This also runs on module paste / preset load into a live
        // module; the publish is the atomic swap, and the transport is left
        // alone exactly as PianoRoll leaves its position (PianoRoll.hpp:756-862).
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

                Memory m;   // zero masks, length 16

                json_t* lanes = json_object_get(memory, "lanes");
                if (json_is_array(lanes))
                {
                    const size_t lane_count = std::min(json_array_size(lanes), (size_t)LANES);
                    for (size_t l = 0; l < lane_count; l++)
                    {
                        json_t* v = json_array_get(lanes, l);
                        if (json_is_integer(v)) m.lanes[l] = (uint32_t)json_integer_value(v);
                    }
                }

                json_t* ratchets = json_object_get(memory, "ratchets");
                if (json_is_array(ratchets))
                {
                    const size_t ratchet_count = std::min(json_array_size(ratchets), (size_t)VOICES);
                    for (size_t l = 0; l < ratchet_count; l++)
                    {
                        json_t* v = json_array_get(ratchets, l);
                        if (json_is_integer(v)) m.ratchets[l] = (uint32_t)json_integer_value(v);
                    }
                }

                json_t* length = json_object_get(memory, "length");
                if (json_is_integer(length)) m.length = rack::math::clamp((int)json_integer_value(length), 1, STEPS);

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
        return mute_before == mute_after && before == after;   // memcmp, see VXDrumSequencerPattern.hpp
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
