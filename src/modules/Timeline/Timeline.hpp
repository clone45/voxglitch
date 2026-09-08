#pragma once
// Timeline — the vxsynth automation timeline as a self-contained Rack module.
// 16 breakpoint lanes on one musical playhead, out a 16-channel poly jack
// (lanes 1-8 also on mono jacks), plus the Song Clock jack family so anything
// clocked from here is sample-locked to the curves by construction.
// Design record: docs/implementation_plans/timeline-design.md.

#include "TimelineEngine.hpp"
#include <atomic>

// Rounds to the nearest half, so the BPM knob lands on 120.0, 120.5, 121.0.
//
// Snapping cannot be done from process(): Rack's Knob defaults to
// smooth = true, which writes through the ENGINE's parameter smoothing, and
// the engine then drives the value toward its target every sample. A rounded
// setValue() from process() is overwritten before the next frame draws, so
// the knob looks unsnapped. Rounding here catches the drag, typed entry and
// MIDI alike; the knob must also set smooth = false (see TimelineWidget's
// BpmKnob) so it calls setValue instead of setSmoothValue.
struct HalfStepQuantity : ParamQuantity
{
    void setValue(float value) override
    {
        ParamQuantity::setValue(std::round(value * 2.f) * 0.5f);
    }
};

struct Timeline : Module
{
    enum ParamIds {
        BPM_PARAM,
        SNAP_PARAM,
        DIV_PARAM,
        LOOP_PARAM,          // loop on/off
        CHASE_PARAM,         // view follows the playhead
        PLAY_PARAM,          // play/stop toggle
        REWIND_PARAM,        // momentary
        LANE_PARAM,          // which lane the editor shows
        REC_PARAM,           // record arm (appended 2026-09-03: ids above are
                             // in users' patches and keep their values)
        NUM_PARAMS
    };
    enum InputIds {
        START_INPUT,
        STOP_INPUT,
        RESET_INPUT,
        REC_INPUT,           // record CV (appended 2026-09-03)
        NUM_INPUTS
    };
    enum OutputIds {
        POLY_OUTPUT,         // all 16 lanes; the ONLY lane output. The eight
                             // mono jacks were removed 2026-08-28 after user
                             // testing said the poly cable was sufficient.
        CLK_OUTPUT,
        RST_OUTPUT,
        RWND_OUTPUT,
        LOOP_OUTPUT,
        RUN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
        // The RUN and LOOP LEDs were removed 2026-08-28: the switches show
        // their own state, and a moving readout already says "playing".
    };

    // ── the lane store: per-lane immutable snapshots, pointer-swapped ──
    // The audio thread loads one pointer per lane per sample and reads
    // through it; the UI thread copies the lane it touches, mutates the copy
    // and publishes it. Old snapshots are retired by generation and freed
    // from the widget's step() (housekeep()), never by the audio thread.
    // See LaneStore in TimelineEngine.hpp.
    timeline_dsp::LaneStore store;

    timeline_dsp::TimelineEngine engine;

    // Editor lock (the Tracks pattern): freezes the editor against mouse
    // edits. Navigation still works, so a locked timeline can be read.
    bool locked = false;

    // Chase: the view follows the playhead, which stays centred once it
    // reaches the middle. A real param, not a menu bool, so it is on the
    // panel and gets MIDI mapping, undo and presets for free. PER MODULE, not
    // the shared voxglitchSettings.chasePlayhead that Tracks uses, so two
    // Timelines can differ. Design:
    // docs/implementation_plans/timeline-chase-design.md.
    bool chaseOn() { return params[CHASE_PARAM].getValue() > 0.5f; }

    // Transport edges + pulse countdowns (songclock.c's contract), PER LANE
    // (2026-08-28): START/STOP/RESET are polyphonic — 1 channel drives every
    // lane, N channels drive lanes 1..N — and the five clock jacks answer in
    // kind, one channel per lane.
    dsp::SchmittTrigger startTrig[timeline_dsp::TL_LANES];
    dsp::SchmittTrigger stopTrig[timeline_dsp::TL_LANES];
    dsp::SchmittTrigger resetTrig[timeline_dsp::TL_LANES];
    bool rewindHeld = false;      // plain edge latch (no dsp::BooleanTrigger
                                  // dependency — this repo has never used it)
    // The PLAY switch is an EDGE, not a level: flipping it starts or stops
    // every lane, and between flips the poly inputs own each lane's state. A
    // level would overwrite per-lane control every sample.
    bool playSwitchHeld = false;
    int lastSeekSerialL[timeline_dsp::TL_LANES] = {};
    double lastBeatL[timeline_dsp::TL_LANES] = {};
    int clkPulse[timeline_dsp::TL_LANES] = {};
    int rstPulse[timeline_dsp::TL_LANES] = {};
    int rwndPulse[timeline_dsp::TL_LANES] = {};
    int loopPulse[timeline_dsp::TL_LANES] = {};
    int clkPulseLen = 480, trigPulseLen = 48;


    static const int SNAP_COUNT = 6;
    static const int DIV_COUNT = 6;

    // ── recording (design: timeline-design.md, "Recording and the per-lane
    //    store") ──
    // The audio thread captures into the ring; the editor's step() drains
    // it into the lane store. `recordRate` is a module setting, not a
    // param: it is a menu choice persisted as "record_rate".
    timeline_dsp::CaptureRing captureRing;
    timeline_dsp::Recorder recorder;
    int recordRate = 2;                             // 1/4
    float recVolts[timeline_dsp::TL_LANES] = {};    // REC IN per lane, this sample

    double recordRateBeats() const { return timeline_dsp::recRateBeats(recordRate); }
    bool recording() const { return recorder.active != 0; }

    Timeline()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam<HalfStepQuantity>(BPM_PARAM, (float)timeline_dsp::TL_BPM_MIN,
                    (float)timeline_dsp::TL_BPM_MAX, 120.f, "Tempo", " BPM");
        configSwitch(SNAP_PARAM, 0.f, 5.f, 3.f, "Snap",
            { "Off", "1 bar", "1/2", "1/4", "1/8", "1/16" });
        configSwitch(DIV_PARAM, 0.f, 5.f, 4.f, "Clock division",
            { "1/1 (bar)", "1/2", "1/4", "1/8", "1/16", "1/32" });
        configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", { "Off", "On" });
        configSwitch(CHASE_PARAM, 0.f, 1.f, 0.f, "Chase playhead", { "Off", "On" });
        configSwitch(PLAY_PARAM, 0.f, 1.f, 0.f, "Play", { "Stopped", "Playing" });
        configButton(REWIND_PARAM, "Rewind");
        configParam(LANE_PARAM, 0.f, (float)(timeline_dsp::TL_LANES - 1), 0.f, "Lane");
        paramQuantities[LANE_PARAM]->snapEnabled = true;
        configSwitch(REC_PARAM, 0.f, 1.f, 0.f, "Record", { "Off", "Armed" });

        configInput(START_INPUT, "Start (poly: channel n starts lane n; mono starts all)");
        configInput(STOP_INPUT, "Stop (poly: channel n stops lane n; mono stops all)");
        configInput(RESET_INPUT, "Reset (poly: channel n rewinds lane n; mono rewinds all)");
        configInput(REC_INPUT, "Record CV (mono: into the selected lane; poly: channel c into lane c)");

        configOutput(POLY_OUTPUT, "All 16 lanes (polyphonic)");
        configOutput(CLK_OUTPUT, "Clock, one channel per lane (division of that lane's beat)");
        configOutput(RST_OUTPUT, "Reset, one channel per lane (rewind or loop wrap)");
        configOutput(RWND_OUTPUT, "Rewind, one channel per lane (user rewind only)");
        configOutput(LOOP_OUTPUT, "Loop, one channel per lane (wrap only)");
        configOutput(RUN_OUTPUT, "Run gate, one channel per lane");

        setRates(APP->engine->getSampleRate());
    }

    // ── lane editing (UI thread) ──
    // Read the live snapshot of one lane. The reference stays valid for the
    // rest of the frame: only housekeep() frees, and only from the widget's
    // step(). Re-fetch after a publish.
    const timeline_dsp::LaneData& lane(int L) const { return store.lane(L); }

    // A private copy to mutate, then hand back to publishLane().
    timeline_dsp::LaneData laneCopy(int L) const { return store.lane(L); }

    // Publish an edited lane: heap-allocates the snapshot, swaps it in, and
    // retires the old one. Any edit clears that lane's "full" indicator.
    void publishLane(int L, const timeline_dsp::LaneData& d)
    {
        if (L < 0 || L >= timeline_dsp::TL_LANES) return;
        store.publishCopy(L, d);
        laneFull[L] = false;
    }

    void clearAllLanes()
    {
        timeline_dsp::LaneData empty;
        for (int L = 0; L < timeline_dsp::TL_LANES; L++) publishLane(L, empty);
    }

    double lastBeat() const
    {
        double m = 0.0;
        for (int L = 0; L < timeline_dsp::TL_LANES; L++)
        {
            double lb = store.lane(L).lastBeat();
            if (lb > m) m = lb;
        }
        return m;
    }

    // Free retired snapshots the audio thread has moved past. Called from
    // the widget's step() once per frame.
    void housekeep() { store.freeRetired(false); }

    // Removed from the engine: process() can no longer run, so every retired
    // snapshot is safe to free now rather than leaking until destruction.
    void onRemove(const RemoveEvent& e) override
    {
        Module::onRemove(e);
        store.freeRetired(true);
    }

    // "Lane full": set when a recording take hits the soft cap on that lane;
    // cleared by the next take or edit on it. UI thread only.
    bool laneFull[timeline_dsp::TL_LANES] = {};

    void setRates(float sr)
    {
        engine.setSampleRate(sr);
        clkPulseLen = (int)(sr * 0.01f);   if (clkPulseLen  < 1) clkPulseLen  = 1;
        trigPulseLen = (int)(sr * 0.001f); if (trigPulseLen < 1) trigPulseLen = 1;
    }

    void onSampleRateChange(const SampleRateChangeEvent& e) override
    {
        setRates(e.sampleRate);
    }

    void onReset(const ResetEvent& e) override
    {
        Module::onReset(e);
        clearAllLanes();
        engine.seekAll(0.0);
        engine.setPlayingAll(0);
        engine.setLoopEnd(0.0);
    }

    int currentLane()
    {
        int L = (int)(params[LANE_PARAM].getValue() + 0.5f);
        if (L < 0) L = 0;
        if (L >= timeline_dsp::TL_LANES) L = timeline_dsp::TL_LANES - 1;
        return L;
    }

    // Snap grid in beats; 0 = off.
    double snapBeats()
    {
        static const double SNAP[SNAP_COUNT] = { 0.0, 4.0, 2.0, 1.0, 0.5, 0.25 };
        int i = (int)(params[SNAP_PARAM].getValue() + 0.5f);
        if (i < 0) i = 0;
        if (i >= SNAP_COUNT) i = SNAP_COUNT - 1;
        return SNAP[i];
    }

    double divBeats()
    {
        static const double DIV[DIV_COUNT] = { 4.0, 2.0, 1.0, 0.5, 0.25, 0.125 };
        int i = (int)(params[DIV_PARAM].getValue() + 0.5f);
        if (i < 0) i = 0;
        if (i >= DIV_COUNT) i = DIV_COUNT - 1;
        return DIV[i];
    }

    // The loop end: the drawn content rounded up to a bar, or 4 bars if empty,
    // unless the user has dragged a handle (loopEndUser > 0).
    double loopEndUser = 0.0;

    double effectiveLoopEnd()
    {
        if (loopEndUser > 0.0) return loopEndUser;
        double last = lastBeat();
        if (last <= 0.0) return 16.0;                   // 4 bars
        return std::ceil(last / 4.0) * 4.0;
    }

    void process(const ProcessArgs& args) override
    {
        using namespace timeline_dsp;

        // ── transport ──
        // Poly rule: one connected channel is a BROADCAST to every lane; N
        // channels address lanes 1..N and leave the rest alone.
        {
            int chS = inputs[START_INPUT].getChannels();
            if (chS <= 1)
            {
                if (startTrig[0].process(inputs[START_INPUT].getVoltage(0), 0.1f, 1.f))
                    engine.setPlayingAll(1);
            }
            else for (int c = 0; c < chS && c < TL_LANES; c++)
                if (startTrig[c].process(inputs[START_INPUT].getVoltage(c), 0.1f, 1.f))
                    engine.lanes[c].setPlaying(1);

            int chP = inputs[STOP_INPUT].getChannels();
            if (chP <= 1)
            {
                if (stopTrig[0].process(inputs[STOP_INPUT].getVoltage(0), 0.1f, 1.f))
                    engine.setPlayingAll(0);
            }
            else for (int c = 0; c < chP && c < TL_LANES; c++)
                if (stopTrig[c].process(inputs[STOP_INPUT].getVoltage(c), 0.1f, 1.f))
                    engine.lanes[c].setPlaying(0);

            int chR = inputs[RESET_INPUT].getChannels();
            if (chR <= 1)
            {
                if (resetTrig[0].process(inputs[RESET_INPUT].getVoltage(0), 0.1f, 1.f))
                    engine.seekAll(0.0);
            }
            else for (int c = 0; c < chR && c < TL_LANES; c++)
                if (resetTrig[c].process(inputs[RESET_INPUT].getVoltage(c), 0.1f, 1.f))
                    engine.lanes[c].seek(0.0);
        }
        // Panel gestures are GLOBAL (Bret's call): the PLAY switch edge
        // starts/stops all lanes, RWND rewinds all.
        bool rewindNow = params[REWIND_PARAM].getValue() > 0.5f;
        if (rewindNow && !rewindHeld) engine.seekAll(0.0);
        rewindHeld = rewindNow;
        bool playNow = params[PLAY_PARAM].getValue() > 0.5f;
        if (playNow != playSwitchHeld) engine.setPlayingAll(playNow ? 1 : 0);
        playSwitchHeld = playNow;
        engine.setLoopEnd(params[LOOP_PARAM].getValue() > 0.5f ? effectiveLoopEnd() : 0.0);

        // ── tempo: the BPM knob owns it, snapped to 0.5 BPM ──
        // (The web original has no clock input: the dock's BPM field is the
        // only tempo source, and Song Clock EMITS clock rather than taking
        // it. An input was tried and removed 2026-08-27.)
        engine.setBpm(params[BPM_PARAM].getValue());   // already half-snapped
                                                       // by HalfStepQuantity

        // ── advance + evaluate: every lane, one sample ──
        // One acquire load per lane; the snapshots stay valid for the whole
        // sample because nothing is freed until the generation moves on.
        const LaneData* ld[TL_LANES];
        store.loadAll(ld);
        engine.tick(ld);

        // ── recording: the take runs while REC is armed and a target lane
        //    plays; captures go to the ring for the editor to drain ──
        {
            int ch = inputs[REC_INPUT].getChannels();
            float v0 = inputs[REC_INPUT].getVoltage(0);
            // Mono (or unpatched) is a broadcast, like the transport inputs;
            // the recorder's mask decides which lanes actually record it.
            for (int L = 0; L < TL_LANES; L++)
                recVolts[L] = (ch <= 1) ? v0 : (L < ch ? inputs[REC_INPUT].getVoltage(L) : 0.f);
            bool armed = params[REC_PARAM].getValue() > 0.5f;
            unsigned want = Recorder::targetMask(ch, currentLane());
            recorder.process(armed, want, engine, recordRateBeats(), recVolts, ld, captureRing);
        }

        // ── song-clock jacks (songclock.c's user-seek vs loop-wrap logic),
        //    now PER LANE: each channel of the five jacks derives from its
        //    own lane's playhead ──
        const double EPS = 1e-9;
        double d = divBeats();
        for (int L = 0; L < TL_LANES; L++)
        {
            timeline_dsp::Lane& ln = engine.lanes[L];
            double cur = ln.playhead;
            bool seeked = (ln.seekSerial != lastSeekSerialL[L]);
            lastSeekSerialL[L] = ln.seekSerial;
            if (seeked)
            {
                if (cur <= lastBeatL[L] + EPS)   // at-or-behind = a rewind
                {
                    rwndPulse[L] = trigPulseLen;
                    rstPulse[L] = trigPulseLen;
                }
                lastBeatL[L] = cur - EPS;
            }
            else if (ln.playing)
            {
                if (ln.wrapped)
                {
                    clkPulse[L] = clkPulseLen;   // beat 0 IS a downbeat
                    rstPulse[L] = trigPulseLen;
                    loopPulse[L] = trigPulseLen;
                }
                else if (std::floor(cur / d) != std::floor(lastBeatL[L] / d))
                {
                    clkPulse[L] = clkPulseLen;
                }
                lastBeatL[L] = cur;
            }
            else
            {
                lastBeatL[L] = cur - EPS;
            }
        }

        // ── outputs: every jack polyphonic, one channel per lane ──
        outputs[POLY_OUTPUT].setChannels(TL_LANES);
        outputs[CLK_OUTPUT].setChannels(TL_LANES);
        outputs[RST_OUTPUT].setChannels(TL_LANES);
        outputs[RWND_OUTPUT].setChannels(TL_LANES);
        outputs[LOOP_OUTPUT].setChannels(TL_LANES);
        outputs[RUN_OUTPUT].setChannels(TL_LANES);
        for (int L = 0; L < TL_LANES; L++)
        {
            // A lane being recorded is BYPASSED to its live input, so the
            // user hears what they record with no UI latency; when the take
            // ends the lane returns to playing its nodes.
            float out = (recorder.active && recorder.targets(L))
                        ? Recorder::clampV(recVolts[L])
                        : (float)engine.lanes[L].value;
            outputs[POLY_OUTPUT].setVoltage(out, L);
            outputs[CLK_OUTPUT].setVoltage(clkPulse[L]  > 0 ? 10.f : 0.f, L);
            if (clkPulse[L]  > 0) clkPulse[L]--;
            outputs[RST_OUTPUT].setVoltage(rstPulse[L]  > 0 ? 10.f : 0.f, L);
            if (rstPulse[L]  > 0) rstPulse[L]--;
            outputs[RWND_OUTPUT].setVoltage(rwndPulse[L] > 0 ? 10.f : 0.f, L);
            if (rwndPulse[L] > 0) rwndPulse[L]--;
            outputs[LOOP_OUTPUT].setVoltage(loopPulse[L] > 0 ? 10.f : 0.f, L);
            if (loopPulse[L] > 0) loopPulse[L]--;
            outputs[RUN_OUTPUT].setVoltage(engine.lanes[L].playing ? 10.f : 0.f, L);
        }

        // Last thing in the block: every read of this sample's snapshots is
        // behind us, so the UI may now count this generation as passed.
        store.bumpGeneration();
    }

    // Bypassed, process() never runs and the generation would stand still,
    // parking every retired snapshot until the module is un-bypassed. Keep
    // the clock ticking so edits made while bypassed are freed on time.
    void processBypass(const ProcessArgs& args) override
    {
        Module::processBypass(args);
        store.bumpGeneration();
    }

    // ── persistence: the lanes and the loop end (params carry the rest) ──
    // Same top-level shape as v2.44 ("loopEnd", "locked", "lanes": 16 arrays
    // of {t, v[, b]}), so every patch saved by the current version loads.
    // Arrays may now run to TL_MAX_NODES entries.
    json_t* dataToJson() override
    {
        using namespace timeline_dsp;
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "loopEnd", json_real(loopEndUser));
        json_object_set_new(rootJ, "locked", json_boolean(locked));
        json_object_set_new(rootJ, "record_rate", json_integer(recordRate));
        json_t* lanesJ = json_array();
        for (int L = 0; L < TL_LANES; L++)
        {
            const LaneData& ld = lane(L);
            json_t* nodesJ = json_array();
            int n = ld.count();
            for (int i = 0; i < n; i++)
            {
                json_t* nJ = json_object();
                json_object_set_new(nJ, "t", json_real(ld.t[i]));
                json_object_set_new(nJ, "v", json_real(ld.v[i]));
                if (ld.b[i] != 0.f)        // straight segments stay compact
                    json_object_set_new(nJ, "b", json_real(ld.b[i]));
                json_array_append_new(nodesJ, nJ);
            }
            json_array_append_new(lanesJ, nodesJ);
        }
        json_object_set_new(rootJ, "lanes", lanesJ);
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override
    {
        using namespace timeline_dsp;
        json_t* loopJ = json_object_get(rootJ, "loopEnd");
        if (loopJ && json_is_number(loopJ)) loopEndUser = json_number_value(loopJ);

        json_t* lockedJ = json_object_get(rootJ, "locked");
        if (lockedJ) locked = json_boolean_value(lockedJ);

        json_t* rateJ = json_object_get(rootJ, "record_rate");   // absent in
                                                                 // v2.44 patches
        if (rateJ && json_is_integer(rateJ))
        {
            int r = (int)json_integer_value(rateJ);
            if (r < 0) r = 0;
            if (r >= TL_REC_RATE_COUNT) r = TL_REC_RATE_COUNT - 1;
            recordRate = r;
        }

        json_t* lanesJ = json_object_get(rootJ, "lanes");
        if (!lanesJ || !json_is_array(lanesJ)) return;
        int nL = (int)json_array_size(lanesJ);
        if (nL > TL_LANES) nL = TL_LANES;
        for (int L = 0; L < TL_LANES; L++)
        {
            LaneData ld;
            json_t* nodesJ = (L < nL) ? json_array_get(lanesJ, L) : NULL;
            if (nodesJ && json_is_array(nodesJ))
            {
                int n = (int)json_array_size(nodesJ);
                if (n > TL_MAX_NODES) n = TL_MAX_NODES;    // the soft cap bounds the load
                ld.t.reserve(n); ld.v.reserve(n); ld.b.reserve(n);
                for (int i = 0; i < n; i++)
                {
                    json_t* nJ = json_array_get(nodesJ, i);
                    if (!nJ || !json_is_object(nJ)) continue;
                    json_t* tJ = json_object_get(nJ, "t");
                    json_t* vJ = json_object_get(nJ, "v");
                    if (!tJ || !vJ) continue;
                    json_t* bJ = json_object_get(nJ, "b");   // optional: old
                                                             // patches lack it
                    ld.add(json_number_value(tJ), json_number_value(vJ),
                           (bJ && json_is_number(bJ)) ? json_number_value(bJ) : 0.0);
                }
                ld.resort();    // a hand-edited patch may be out of order
            }
            publishLane(L, ld);
        }
    }
};
