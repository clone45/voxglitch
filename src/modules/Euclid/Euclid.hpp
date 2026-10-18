#pragma once
//
// Euclid — a Euclidean rhythm sequencer: HITS hits spread as evenly as
// Bresenham allows over STEPS steps, shifted by ROT, stepped by an external
// clock. TRIG is one pulse per hit, and the ring on the panel shows the live
// pattern. (The source's GATE output was dropped on 2026-09-18, owner's call:
// the engine still computes the level, the panel no longer carries the jack.)
//
// The Rack port of the vxsynth "Euclidean Sequencer" (euclid.c), itself a port
// of Groovebox Advanced's GAEuclid. See:
//   docs/modules/euclid/rack-port-design.md          <- what diverges from euclid.c and why; §4.1 is the reset rule
//   src/modules/Euclid/EuclidEngine.hpp              <- the CV law, the pattern and the playhead, Rack-free, tested in tests/euclid/
//   src/vgLib-2.0/sequencer/ResetClockWindow.hpp     <- the VCV reset standard the rule follows
//
// This file is the RACK ADAPTER (VXDrumSequencer.hpp is the model): it reads
// the knobs, the attenuverters and the jacks, runs the two Schmitt triggers,
// calls euclid::Sequencer::process once per sample, and shapes what comes
// back into the TRIG jack. Nothing here computes a pattern or moves the
// playhead; that is the engine's, and the engine never sees a sample rate.
// The two rate-dependent quantities, the 1 ms clock-ignore window and the
// TRIG pulse, are computed from args.sampleRate at the moment they are armed
// and never cached, so onSampleRateChange has no work to do (design §4.3).
//
// RESET is the VCV standard (design §4.1, SEQ3's order): a rising edge on RST
// puts the playhead ON step 1 and plays it at the reset instant; clock edges
// inside the 1 ms window that follows are the reset's own clock and are
// swallowed; the next clock after the window is step 2's. Patch load and
// Rack's Initialize are IMPLICIT resets: step 1 current, the window armed,
// nothing fires. RST is processed BEFORE CLK in the code below, and neither
// Schmitt is ever reset(): a line that is high is high, not an edge.
//
// TRIG is a sample countdown, not a dsp::PulseGenerator, whose trigger()
// only EXTENDS a pulse and can never emit a second edge inside one (design
// §4.2). A strike that lands while TRIG is still high forces exactly ONE
// sample at 0 V and starts the fresh pulse on the next sample: the VX Drum
// Sequencer gap rule (rack-port-design.md §5; TriggerShaper in
// VXDrumSequencerEngine.hpp), single lane.
//
// THREADS: `snapshot` is written on the audio thread, only when something
// changed, and read by the ring in draw(); `trigger_length_index` is written
// by the menu and read by process(). Both are plain aligned ints and bools
// (PianoRoll.hpp:157-159): a torn read of the snapshot costs one frame of a
// stale dot and nothing else.
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

struct Euclid : VoxglitchModule
{
    enum ParamIds
    {
        HITS_PARAM,
        STEPS_PARAM,
        ROT_PARAM,
        HITS_ATTEN_PARAM,
        STEPS_ATTEN_PARAM,
        ROT_ATTEN_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        RESET_INPUT,
        HITS_CV_INPUT,
        STEPS_CV_INPUT,
        ROT_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        TRIG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    // ── What the ring reads (design §5.3) ────────────────────────────────────
    //
    // The EFFECTIVE values (knob + CV, clamped: decision 6), the playhead and
    // the hit table, so a CV-modulated pattern is visible. Audio writes, only
    // when a field changed; the ring copies it in draw(). `valid` is false
    // until the first process() call, and the ring draws the default 4/16
    // pattern until then (the browser preview never gets a process() call).
    //
    struct Snapshot
    {
        int hits;
        int steps;
        int rot;
        int position;
        bool hit[euclid::MAX_STEPS];
        bool valid;

        Snapshot() : hits(euclid::DEFAULT_HITS), steps(euclid::DEFAULT_STEPS), rot(euclid::DEFAULT_ROT),
                     position(0), valid(false)
        {
            for (int i = 0; i < euclid::MAX_STEPS; i++) hit[i] = false;
        }
    };
    Snapshot snapshot;

    // ── The engine ───────────────────────────────────────────────────────────
    euclid::Sequencer seq;

    dsp::SchmittTrigger reset_trigger;   // NEVER .reset() after construction (design §4.1 rule 5)
    dsp::SchmittTrigger clock_trigger;

    bool loaded = false;                 // the first process() call is the load's implicit reset

    // ── TRIG (design §4.2): the drum sequencer's TriggerShaper, one lane ─────
    int64_t trig_remaining = 0;          // samples of the current pulse still to emit
    bool trig_gap = false;               // force 0 V THIS sample; the pulse armed above it starts on the next (the gap rule)
    bool trig_was_high = false;          // TRIG was > 0 V on the previous sample

    int trigger_length_index = 0;        // index into triggerLengths() (ArpSeq's list), persisted

    // Trigger pulse lengths in seconds, ArpSeq's list (ArpSeq.hpp:120); index 0
    // = 1 ms, the VCV trigger standard (decision 9).
    static const std::vector<float>& triggerLengths()
    {
        static const std::vector<float> lengths = {0.001f, 0.002f, 0.005f, 0.010f, 0.020f, 0.050f, 0.100f, 0.200f};
        return lengths;
    }

    Euclid()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // The three integer knobs snap (design §4.4); the attenuverters read in
        // percent like the other Voxglitch trimpots (ByteBeat.hpp:97-99).
        configParam(HITS_PARAM, 0.f, (float)euclid::MAX_STEPS, (float)euclid::DEFAULT_HITS, "Hits");
        configParam(STEPS_PARAM, 1.f, (float)euclid::MAX_STEPS, (float)euclid::DEFAULT_STEPS, "Steps");
        configParam(ROT_PARAM, 0.f, (float)(euclid::MAX_STEPS - 1), (float)euclid::DEFAULT_ROT, "Rotation");
        paramQuantities[HITS_PARAM]->snapEnabled = true;
        paramQuantities[STEPS_PARAM]->snapEnabled = true;
        paramQuantities[ROT_PARAM]->snapEnabled = true;

        // Default 0 (decision 4): a patched jack is silent until its trimpot is
        // turned up, for parity with vxsynth patches.
        configParam(HITS_ATTEN_PARAM, -1.f, 1.f, 0.f, "Hits CV", "%", 0.f, 100.f);
        configParam(STEPS_ATTEN_PARAM, -1.f, 1.f, 0.f, "Steps CV", "%", 0.f, 100.f);
        configParam(ROT_ATTEN_PARAM, -1.f, 1.f, 0.f, "Rotation CV", "%", 0.f, 100.f);

        configInput(CLOCK_INPUT, "Clock (one pulse per step)");
        configInput(RESET_INPUT, "Reset (jumps to step 1 and plays it; the next clock is step 2)");
        configInput(HITS_CV_INPUT, "Hits CV (5 V = 16 steps, through the Hits CV attenuverter)");
        configInput(STEPS_CV_INPUT, "Steps CV (5 V = 16 steps, through the Steps CV attenuverter)");
        configInput(ROT_CV_INPUT, "Rotation CV (5 V = 16 steps, through the Rotation CV attenuverter)");
        configOutput(TRIG_OUTPUT, "Trigger (10 V pulse on every hit; length in the context menu)");
    }

    // A snapping knob holds an integer, but a MIDI map or a CV-to-param
    // bridge can leave it fractional; round rather than truncate so 3.9999
    // is 4.
    int knob(int param_id)
    {
        return (int)std::round(params[param_id].getValue());
    }

    // ── process() — design §4.1 order, every sample ──────────────────────────
    void process(const ProcessArgs& args) override
    {
        const float sample_rate = args.sampleRate;

        // 1. Knobs, attenuverters and CV, read every sample as the source does
        //    (design §4.4); the engine's dirty cache makes the pattern free
        //    when nothing changed. The CV law is euclid_process_inline's:
        //    knob + (int)(cv * 3.2 * atten), truncated toward zero, clamped;
        //    ROT to the effective steps, then HITS to the effective steps (§3.2).
        const int steps = euclid::effectiveValue(knob(STEPS_PARAM), inputs[STEPS_CV_INPUT].getVoltage(),
                                                 params[STEPS_ATTEN_PARAM].getValue(), 1, euclid::MAX_STEPS);
        int hits = euclid::effectiveValue(knob(HITS_PARAM), inputs[HITS_CV_INPUT].getVoltage(),
                                          params[HITS_ATTEN_PARAM].getValue(), 0, euclid::MAX_STEPS);
        const int rot = euclid::effectiveValue(knob(ROT_PARAM), inputs[ROT_CV_INPUT].getVoltage(),
                                               params[ROT_ATTEN_PARAM].getValue(), 0, steps - 1);
        hits = std::min(hits, steps);

        // 2. The standard's 1 ms clock-ignore window, in samples at the live
        //    rate, computed afresh each time it is armed and never cached
        //    (design §4.3). Patch load / module add is an IMPLICIT reset
        //    (Impromptu's rule, §4.1 rule 4): step 1 current, the window
        //    armed, nothing fires. Armed on the first process() call, not in
        //    the constructor, because the window needs the engine's live rate.
        const int window_samples = vgLib_v2::ResetClockWindow::samplesFor(sample_rate);
        if (!loaded)
        {
            loaded = true;
            seq.load(window_samples);
        }

        // 3. RST before CLK (SEQ3's order, matched by the engine). Both
        //    Schmitts are polled every sample and never reset().
        const bool reset_edge = reset_trigger.process(inputs[RESET_INPUT].getVoltage(),
                                                      constants::gate_low_trigger, constants::gate_high_trigger);
        const bool clock_edge = clock_trigger.process(inputs[CLOCK_INPUT].getVoltage(),
                                                      constants::gate_low_trigger, constants::gate_high_trigger);

        // 4. The engine: the reset, the (possibly swallowed) clock, the window
        //    tick, the outputs, in that order.
        const euclid::Sequencer::Out out = seq.process(reset_edge, clock_edge, window_samples, hits, steps, rot);

        // 5. TRIG: a strike arms a pulse of the menu length. If TRIG was high
        //    on the previous sample the pulse is armed one sample longer and
        //    THIS sample is forced to 0 V, so the fresh pulse rises on the
        //    next sample (TriggerShaper::tick, VXDrumSequencerEngine.hpp).
        //    Keyed on the previous sample's OUTPUT, not the countdown: a
        //    pulse has trig_remaining == 0 on its last high sample.
        const int index = rack::math::clamp(trigger_length_index, 0, (int)triggerLengths().size() - 1);
        const int64_t pulse_len = std::max((int64_t)1, (int64_t)(sample_rate * triggerLengths()[index]));

        if (out.struck)
        {
            trig_gap = trig_was_high;
            trig_remaining = pulse_len + (trig_gap ? 1 : 0);
        }

        float trig_v;
        if (trig_gap)
        {
            trig_v = 0.f;
            trig_gap = false;
        }
        else
        {
            trig_v = (trig_remaining > 0) ? 10.f : 0.f;
        }
        trig_was_high = trig_v > 0.f;
        if (trig_remaining > 0) trig_remaining--;

        // 6. The jack. (out.gate / out.gate_gap are computed by the engine and
        //    pinned by its tests, but no longer reach a jack: 2026-09-18.)
        outputs[TRIG_OUTPUT].setVoltage(trig_v);

        // 7. The ring's snapshot.
        publishSnapshot(out.position);
    }

    // Write the snapshot only when a field changed. The hit table changes
    // only with the (hits, steps, rot) triple, which Pattern::compute caches
    // after its own clamps, so those three are the effective display values.
    void publishSnapshot(int position)
    {
        const euclid::Pattern& p = seq.pattern;

        const bool pattern_changed = !snapshot.valid
            || p.hits != snapshot.hits || p.steps != snapshot.steps || p.rot != snapshot.rot;

        if (pattern_changed)
        {
            snapshot.hits = p.hits;
            snapshot.steps = p.steps;
            snapshot.rot = p.rot;
            for (int i = 0; i < euclid::MAX_STEPS; i++) snapshot.hit[i] = p.hit[i];
        }
        if (pattern_changed || position != snapshot.position)
        {
            snapshot.position = position;
        }
        snapshot.valid = true;
    }

    // ── Lifecycle ────────────────────────────────────────────────────────────

    // Initialize: 1 ms triggers, the playhead on step 1 with the window armed
    // (an implicit reset: nothing fires) and no pulse in flight; then the
    // params via the base. The window length comes from the engine's current
    // rate: this runs on the UI thread, with no ProcessArgs in hand
    // (Ghosts.hpp:286 idiom; VXDrumSequencer.hpp onReset).
    void onReset(const ResetEvent& e) override
    {
        trigger_length_index = 0;
        seq.load(vgLib_v2::ResetClockWindow::samplesFor(APP->engine->getSampleRate()));
        trig_remaining = 0;
        trig_gap = false;
        trig_was_high = false;
        Module::onReset(e);
    }

    // ── Persistence (design §5.4) ────────────────────────────────────────────
    //
    // Raw jansson, json_object_set_new EXCLUSIVELY (PianoRoll.hpp:711-717).
    // Params are serialized by Rack. The playhead is deliberately not
    // persisted: step 1 is current after every load (the load is an implicit
    // reset).
    //
    //   { "version": "1.0.0", "trigger_length_index": 0 }
    //
    json_t* dataToJson() override
    {
        json_t* root = json_object();
        json_object_set_new(root, "version", json_string("1.0.0"));
        json_object_set_new(root, "trigger_length_index", json_integer(trigger_length_index));
        return root;
    }

    void dataFromJson(json_t* root) override
    {
        if (!root) return;

        // Probe the key: a missing key keeps the constructor default rather
        // than becoming zero (PianoRoll.hpp:764-767).
        json_t* trigger_length_json = json_object_get(root, "trigger_length_index");
        if (json_is_integer(trigger_length_json))
        {
            trigger_length_index = rack::math::clamp((int)json_integer_value(trigger_length_json),
                                                     0, (int)triggerLengths().size() - 1);
        }
    }
};
