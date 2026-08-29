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
        NUM_PARAMS
    };
    enum InputIds {
        START_INPUT,
        STOP_INPUT,
        RESET_INPUT,
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

    // ── the lane store: double-buffered, single writer (the UI thread) ──
    // The audio thread only ever dereferences `live`, so it never observes a
    // half-applied edit. beginEdit() hands the UI a scratch copy; commitEdit()
    // publishes it with one atomic store.
    timeline_dsp::LaneSet laneBuf[2];
    std::atomic<timeline_dsp::LaneSet*> live;
    int editBuf = 1;

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

        configInput(START_INPUT, "Start (poly: channel n starts lane n; mono starts all)");
        configInput(STOP_INPUT, "Stop (poly: channel n stops lane n; mono stops all)");
        configInput(RESET_INPUT, "Reset (poly: channel n rewinds lane n; mono rewinds all)");

        configOutput(POLY_OUTPUT, "All 16 lanes (polyphonic)");
        configOutput(CLK_OUTPUT, "Clock, one channel per lane (division of that lane's beat)");
        configOutput(RST_OUTPUT, "Reset, one channel per lane (rewind or loop wrap)");
        configOutput(RWND_OUTPUT, "Rewind, one channel per lane (user rewind only)");
        configOutput(LOOP_OUTPUT, "Loop, one channel per lane (wrap only)");
        configOutput(RUN_OUTPUT, "Run gate, one channel per lane");

        live.store(&laneBuf[0]);
        setRates(APP->engine->getSampleRate());
    }

    // ── lane editing (UI thread) ──
    timeline_dsp::LaneSet* beginEdit()
    {
        timeline_dsp::LaneSet* cur = live.load();
        timeline_dsp::LaneSet* scratch = &laneBuf[editBuf];
        *scratch = *cur;                    // copy, then mutate the copy
        return scratch;
    }

    void commitEdit()
    {
        timeline_dsp::LaneSet* scratch = &laneBuf[editBuf];
        editBuf ^= 1;                       // the old live buffer becomes scratch
        live.store(scratch);
    }

    // Replace the whole store (undo/redo, JSON load).
    void setLanes(const timeline_dsp::LaneSet& src)
    {
        timeline_dsp::LaneSet* scratch = &laneBuf[editBuf];
        *scratch = src;
        commitEdit();
    }

    const timeline_dsp::LaneSet& lanes() { return *live.load(); }

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
        timeline_dsp::LaneSet empty;
        setLanes(empty);
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
        double last = lanes().lastBeat();
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
        const LaneSet& ls = lanes();
        engine.tick(ls);

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
            outputs[POLY_OUTPUT].setVoltage((float)engine.lanes[L].value, L);
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

    }

    // ── persistence: the lanes and the loop end (params carry the rest) ──
    json_t* dataToJson() override
    {
        using namespace timeline_dsp;
        const LaneSet& ls = lanes();
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "loopEnd", json_real(loopEndUser));
        json_object_set_new(rootJ, "locked", json_boolean(locked));
        json_t* lanesJ = json_array();
        for (int L = 0; L < TL_LANES; L++)
        {
            json_t* nodesJ = json_array();
            for (int i = 0; i < ls.count[L]; i++)
            {
                json_t* nJ = json_object();
                json_object_set_new(nJ, "t", json_real(ls.t[L][i]));
                json_object_set_new(nJ, "v", json_real(ls.v[L][i]));
                if (ls.b[L][i] != 0.0)     // straight segments stay compact
                    json_object_set_new(nJ, "b", json_real(ls.b[L][i]));
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

        json_t* lanesJ = json_object_get(rootJ, "lanes");
        if (!lanesJ || !json_is_array(lanesJ)) return;
        LaneSet ls;
        int nL = (int)json_array_size(lanesJ);
        if (nL > TL_LANES) nL = TL_LANES;
        for (int L = 0; L < nL; L++)
        {
            json_t* nodesJ = json_array_get(lanesJ, L);
            if (!nodesJ || !json_is_array(nodesJ)) continue;
            int n = (int)json_array_size(nodesJ);
            for (int i = 0; i < n; i++)
            {
                json_t* nJ = json_array_get(nodesJ, i);
                if (!nJ || !json_is_object(nJ)) continue;
                json_t* tJ = json_object_get(nJ, "t");
                json_t* vJ = json_object_get(nJ, "v");
                if (!tJ || !vJ) continue;
                json_t* bJ = json_object_get(nJ, "b");   // optional: old
                                                         // patches lack it
                ls.add(L, json_number_value(tJ), json_number_value(vJ),
                       (bJ && json_is_number(bJ)) ? json_number_value(bJ) : 0.0);
            }
            ls.resort(L);       // a hand-edited patch may be out of order
        }
        setLanes(ls);
    }
};
