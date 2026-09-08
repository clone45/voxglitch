// transport_test.cpp — the module's process() logic (per-lane transport,
// poly START/STOP/RESET, per-lane song-clock pulses) mirrored outside Rack.
// A line-for-line copy of Timeline::process's decision structure with params
// and jacks stubbed; if it and the module diverge, re-sync THIS file.
// Per-lane restructure 2026-08-28; per-lane snapshot store 2026-09-03.
#include <cstdio>
#include <cmath>
#include "TimelineEngine.hpp"
using namespace timeline_dsp;

static int fails = 0;
static void check(const char* what, bool ok, double got = 0, double want = 0)
{
    if (ok) { printf("  ok    %s\n", what); return; }
    printf("  FAIL  %s   (got %.9f want %.9f)\n", what, got, want); fails++;
}

struct Trig
{
    bool state = true;
    bool process(double in, double lo, double hi)
    {
        if (state) { if (in <= lo) state = false; }
        else if (in >= hi) { state = true; return true; }
        return false;
    }
};

struct Sim
{
    TimelineEngine engine;
    // The node store as process() sees it: one snapshot per lane, read
    // through a const pointer array (what LaneStore::loadAll fills).
    LaneData laneData[TL_LANES];
    const LaneData* lanes[TL_LANES];
    double SR = 48000.0;
    // params
    double bpm = 120.0, div = 0.25;
    bool playSwitch = false, loop = false;
    double loopEndUser = 0.0;
    // poly inputs: channel counts + per-sample voltages
    int chStart = 0, chStop = 0, chReset = 0;      // 0 = unpatched
    double vStart[TL_LANES] = {}, vStop[TL_LANES] = {}, vReset[TL_LANES] = {};
    Trig startTrig[TL_LANES], stopTrig[TL_LANES], resetTrig[TL_LANES];
    bool playSwitchHeld = false;
    // per-lane songclock state
    int lastSeekSerialL[TL_LANES] = {};
    double lastBeatL[TL_LANES] = {};
    int clkPulse[TL_LANES] = {}, rstPulse[TL_LANES] = {},
        rwndPulse[TL_LANES] = {}, loopPulse[TL_LANES] = {};
    int clkPulseLen, trigPulseLen;
    // observed
    int clkFires[TL_LANES] = {}, rstFires[TL_LANES] = {},
        rwndFires[TL_LANES] = {}, loopFires[TL_LANES] = {};

    Sim()
    {
        engine.setSampleRate(SR);
        clkPulseLen = (int)(SR * 0.01); trigPulseLen = (int)(SR * 0.001);
        for (int L = 0; L < TL_LANES; L++) { lastBeatL[L] = -1e-9; lanes[L] = &laneData[L]; }
    }

    double effectiveLoopEnd()
    {
        if (loopEndUser > 0.0) return loopEndUser;
        double last = lastBeatOf(lanes);
        if (last <= 0.0) return 16.0;
        return std::ceil(last / 4.0) * 4.0;
    }

    void step()
    {
        // ── transport: mono broadcast, poly per lane ──
        if (chStart == 1)
        { if (startTrig[0].process(vStart[0], 0.1, 1.0)) engine.setPlayingAll(1); }
        else if (chStart > 1)
            for (int c = 0; c < chStart && c < TL_LANES; c++)
                if (startTrig[c].process(vStart[c], 0.1, 1.0)) engine.lanes[c].setPlaying(1);
        if (chStop == 1)
        { if (stopTrig[0].process(vStop[0], 0.1, 1.0)) engine.setPlayingAll(0); }
        else if (chStop > 1)
            for (int c = 0; c < chStop && c < TL_LANES; c++)
                if (stopTrig[c].process(vStop[c], 0.1, 1.0)) engine.lanes[c].setPlaying(0);
        if (chReset == 1)
        { if (resetTrig[0].process(vReset[0], 0.1, 1.0)) engine.seekAll(0.0); }
        else if (chReset > 1)
            for (int c = 0; c < chReset && c < TL_LANES; c++)
                if (resetTrig[c].process(vReset[c], 0.1, 1.0)) engine.lanes[c].seek(0.0);

        // PLAY switch: an EDGE onto all lanes.
        if (playSwitch != playSwitchHeld) engine.setPlayingAll(playSwitch ? 1 : 0);
        playSwitchHeld = playSwitch;
        engine.setLoopEnd(loop ? effectiveLoopEnd() : 0.0);

        double bpmSnap = std::floor(bpm * 2.0 + 0.5) * 0.5;   // HalfStepQuantity's rule
        engine.setBpm(bpmSnap);
        engine.tick(lanes);

        // ── song-clock, per lane ──
        const double EPS = 1e-9;
        for (int L = 0; L < TL_LANES; L++)
        {
            Lane& ln = engine.lanes[L];
            double cur = ln.playhead;
            bool seeked = (ln.seekSerial != lastSeekSerialL[L]);
            lastSeekSerialL[L] = ln.seekSerial;
            if (seeked)
            {
                if (cur <= lastBeatL[L] + EPS)
                { rwndPulse[L] = trigPulseLen; rstPulse[L] = trigPulseLen;
                  rwndFires[L]++; rstFires[L]++; }
                lastBeatL[L] = cur - EPS;
            }
            else if (ln.playing)
            {
                if (ln.wrapped)
                { clkPulse[L] = clkPulseLen; rstPulse[L] = trigPulseLen;
                  loopPulse[L] = trigPulseLen;
                  clkFires[L]++; rstFires[L]++; loopFires[L]++; }
                else if (std::floor(cur / div) != std::floor(lastBeatL[L] / div))
                { clkPulse[L] = clkPulseLen; clkFires[L]++; }
                lastBeatL[L] = cur;
            }
            else lastBeatL[L] = cur - EPS;
            if (clkPulse[L] > 0) clkPulse[L]--;
            if (rstPulse[L] > 0) rstPulse[L]--;
            if (rwndPulse[L] > 0) rwndPulse[L]--;
            if (loopPulse[L] > 0) loopPulse[L]--;
        }
    }
};

int main()
{
    printf("PLAY switch drives all lanes, as an edge\n");
    {
        Sim s; s.div = 1.0;
        s.playSwitch = true;
        for (int i = 0; i < 96000; i++) s.step();     // 2 s = 4 beats
        int allAt4 = 1;
        for (int L = 0; L < TL_LANES; L++)
            if (std::fabs(s.engine.lanes[L].playhead - 4.0) > 1e-6) allAt4 = 0;
        check("all 16 lanes travelled 4 beats", allAt4 == 1);
        // 4 beats of travel = 5 pulses: the parked-playhead rule fires the
        // downbeat at beat 0 the moment play starts (songclock.c behaviour).
        check("CLK fired per beat incl. the beat-0 downbeat on lane 0",
              s.clkFires[0] == 5, s.clkFires[0], 5);
        check("and identically on lane 15", s.clkFires[15] == 5, s.clkFires[15], 5);
        // The edge rule: a poly stop must not be overwritten by the held switch.
        // (Prime with one low sample first: a Schmitt trigger starts HIGH, so
        // the first-ever sample on a fresh channel cannot fire — Rack's guard
        // against false triggers when patching a high gate.)
        s.chStop = 3;
        s.step();
        s.vStop[1] = 10.0;                             // stop lane 1 only
        s.step();
        s.vStop[1] = 0.0;
        for (int i = 0; i < 4800; i++) s.step();
        check("a poly STOP on lane 1 sticks while the switch stays up",
              s.engine.lanes[1].playing == 0 && s.engine.lanes[0].playing == 1);
    }

    printf("\nMono inputs broadcast\n");
    {
        Sim s; s.chStart = 1;
        s.step();                                      // prime the trigger low
        s.vStart[0] = 10.0; s.step(); s.vStart[0] = 0.0;
        check("one START channel starts every lane", s.engine.anyPlaying() == 1
              && s.engine.lanes[15].playing == 1);
        s.chReset = 1;
        for (int i = 0; i < 48000; i++) s.step();
        s.vReset[0] = 10.0; s.step(); s.vReset[0] = 0.0;
        int allZero = 1;
        for (int L = 0; L < TL_LANES; L++)
            if (s.engine.lanes[L].playhead > 1e-3) allZero = 0;
        check("one RESET channel rewinds every lane", allZero == 1);
        check("RWND pulsed on every lane", s.rwndFires[0] >= 1 && s.rwndFires[15] >= 1);
    }

    printf("\nPoly inputs address lanes independently\n");
    {
        Sim s; s.chStart = 4; s.div = 1.0;
        s.step();                                      // prime the triggers low
        s.vStart[2] = 10.0;                            // start lane 2 only
        s.step();
        s.vStart[2] = 0.0;
        for (int i = 0; i < 48000; i++) s.step();      // 2 beats
        // Tolerance: the priming and pulse steps each advance one sample
        // (~4.2e-5 beats at 120 BPM), so within a millibeat is exact here.
        check("lane 2 runs", std::fabs(s.engine.lanes[2].playhead - 2.0) < 1e-3,
              s.engine.lanes[2].playhead, 2.0);
        check("lane 0 does not", s.engine.lanes[0].playhead == 0.0);
        check("lanes beyond the channel count are untouched",
              s.engine.lanes[10].playing == 0);
        check("only lane 2's CLK fired (incl. its beat-0 downbeat)",
              s.clkFires[2] == 3 && s.clkFires[0] == 0, s.clkFires[2], 3);
        // Reset just lane 2, poly.
        s.chReset = 4;
        s.step();                                      // prime the trigger low
        s.vReset[2] = 10.0; s.step(); s.vReset[2] = 0.0;
        check("poly RESET rewinds only lane 2",
              s.engine.lanes[2].playhead < 1e-3 && s.rwndFires[2] == 1);
        check("no RWND anywhere else", s.rwndFires[0] == 0 && s.rwndFires[3] == 0);
    }

    printf("\nLoop wraps per lane, at each lane's own arrival\n");
    {
        Sim s; s.div = 1.0; s.loop = true; s.loopEndUser = 4.0;
        s.chStart = 8;
        s.step();                                          // prime the triggers
        s.vStart[0] = 10.0; s.step(); s.vStart[0] = 0.0;   // lane 0 now
        for (int i = 0; i < 48000; i++) s.step();          // 2 beats later...
        s.vStart[5] = 10.0; s.step(); s.vStart[5] = 0.0;   // ...lane 5 starts
        for (int i = 0; i < 48000 + 4800; i++) s.step();   // past lane 0's wrap
        check("lane 0 wrapped", s.loopFires[0] >= 1, s.loopFires[0], 1);
        check("lane 5 has not wrapped yet", s.loopFires[5] == 0, s.loopFires[5], 0);
        check("lane 0's wrap did not RWND anyone", s.rwndFires[0] == 0);
        for (int i = 0; i < 96000; i++) s.step();
        check("lane 5 wraps in its own time", s.loopFires[5] >= 1, s.loopFires[5], 1);
    }

    printf("\nBPM snapping (HalfStepQuantity's rule, mirrored)\n");
    {
        Sim s; s.playSwitch = true; s.bpm = 137.3; s.step();
        check("137.3 snaps to 137.5", std::fabs(s.engine.bpm - 137.5) < 1e-9,
              s.engine.bpm, 137.5);
        s.bpm = 137.24; s.step();
        check("137.24 snaps to 137.0", std::fabs(s.engine.bpm - 137.0) < 1e-9,
              s.engine.bpm, 137.0);
    }

    printf("\nEmpty lanes output 0 V while running\n");
    {
        Sim s; s.playSwitch = true;
        for (int i = 0; i < 4800; i++) s.step();
        int allZero = 1;
        for (int L = 0; L < TL_LANES; L++)
            if (s.engine.lanes[L].value != 0.0) allZero = 0;
        check("all 16 lanes read 0 V when empty", allZero == 1);
    }

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASS", fails,
           fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
