// engine_test.cpp — the per-lane automation engine, checked against the
// behaviours the web guide promises PLUS lane independence (the 2026-08-28
// restructure: every lane owns its own transport). Store refactor
// 2026-09-03: node data is a LaneData per lane, handed to the engine as an
// array of const pointers (what the module loads from its LaneStore).
// Run: g++ -std=c++11 -O2 -I ../../src/modules/Timeline
#include <cstdio>
#include <cmath>
#include "TimelineEngine.hpp"
using namespace timeline_dsp;

static int fails = 0;
static void check(const char* what, bool ok, double got = 0.0, double want = 0.0)
{
    if (ok) { printf("  ok    %s\n", what); return; }
    printf("  FAIL  %s   (got %.9f want %.9f)\n", what, got, want);
    fails++;
}
static void near(const char* what, double got, double want, double tol = 1e-9)
{
    check(what, std::fabs(got - want) <= tol, got, want);
}

// 16 lanes of node data plus the pointer array the engine reads — what the
// module's store hands process() each sample.
struct Set
{
    LaneData d[TL_LANES];
    const LaneData* p[TL_LANES];
    Set() { for (int L = 0; L < TL_LANES; L++) p[L] = &d[L]; }
};

int main()
{
    Set ls;
    TimelineEngine e;
    e.setSampleRate(48000.0);

    printf("Lane evaluation\n");
    e.tick(ls.p);
    near("empty lane reads 0 V", e.lanes[0].value, 0.0);

    // a ramp 0 V @ beat 0 -> 10 V @ beat 4
    ls.d[0].add(0.0, 0.0);
    ls.d[0].add(4.0, 10.0);
    e.lanes[0].seek(2.0); e.lanes[0].eval(ls.p[0]);
    near("midpoint of a ramp interpolates", e.lanes[0].value, 5.0);
    e.lanes[0].seek(0.0); e.lanes[0].eval(ls.p[0]);
    near("at the first node", e.lanes[0].value, 0.0);
    e.lanes[0].seek(4.0); e.lanes[0].eval(ls.p[0]);
    near("at the last node", e.lanes[0].value, 10.0);
    e.lanes[0].seek(9.0); e.lanes[0].eval(ls.p[0]);
    near("holds after the last node", e.lanes[0].value, 10.0);

    ls.d[1].clear();
    ls.d[1].add(8.0, -7.0);
    ls.d[1].add(12.0, 7.0);
    e.lanes[1].seek(0.0); e.lanes[1].eval(ls.p[1]);
    near("holds before the first node", e.lanes[1].value, -7.0);

    e.lanes[2].seek(1.0); e.lanes[2].eval(0);
    near("a null snapshot reads as an empty lane", e.lanes[2].value, 0.0);

    printf("\nSegment bend (curves between nodes)\n");
    {
        // The exponential-approach family:
        //   f_b(x) = (1 - e^(-b x)) / (1 - e^(-b)), so
        //   f_b(0.5) = 1 / (1 + e^(-b/2)) — a logistic in b.
        TimelineEngine c;
        LaneData m;
        m.add(0.0, 0.0, 2.0);
        m.add(4.0, 10.0);
        double mid = 10.0 / (1.0 + std::exp(-1.0));   // b=2 -> 7.3106
        c.lanes[0].seek(2.0); c.lanes[0].eval(&m);
        near("bend +2 at the midpoint follows the exp family",
             c.lanes[0].value, mid, 1e-6);
        m.b[0] = -2.0f;
        c.lanes[0].seek(2.0); c.lanes[0].eval(&m);
        near("bend -2 mirrors it exactly (10 - the +2 value)",
             c.lanes[0].value, 10.0 - mid, 1e-6);
        m.b[0] = 0.0f;
        c.lanes[0].seek(2.0); c.lanes[0].eval(&m);
        near("bend 0 is exactly linear", c.lanes[0].value, 5.0);
        // endpoints are exact for any bend
        m.b[0] = 2.5f;
        c.lanes[0].seek(0.0); c.lanes[0].eval(&m);
        near("a bent segment still starts at v0", c.lanes[0].value, 0.0);
        c.lanes[0].seek(4.0); c.lanes[0].eval(&m);
        near("a bent segment still ends at v1", c.lanes[0].value, 10.0);

        // The bend travels with its left node through edits.
        LaneData ed;
        ed.add(0.0, 0.0, 1.5);
        ed.add(8.0, 8.0);
        ed.insert(4.0, 1.0);           // split the bent segment
        check("after a split the left keeps its bend, the new node is straight",
              ed.b[0] == 1.5f && ed.b[1] == 0.0f);
        ed.erase(1);
        check("erase shifts bends with their nodes",
              ed.count() == 2 && ed.b[0] == 1.5f);
        ed.t[0] = 9.0f; ed.resort();
        check("resort carries the bend with its node",
              ed.t[0] == 8.0f && ed.b[0] == 0.0f && ed.b[1] == 1.5f);
    }

    printf("\nCursor tracking (forward play and seek-back)\n");
    e.seekAll(0.0);
    e.setPlayingAll(1);
    for (int i = 0; i < 48000; i++) e.tick(ls.p);      // 1 s at 120 BPM
    near("one second at 120 BPM = 2 beats", e.lanes[0].playhead, 2.0, 1e-9);
    near("ramp at beat 2 after playing there", e.lanes[0].value, 5.0, 1e-6);
    e.lanes[0].seek(1.0); e.lanes[0].eval(ls.p[0]);
    near("seek back re-reads correctly", e.lanes[0].value, 2.5, 1e-9);

    printf("\nLane independence (the point of the restructure)\n");
    {
        TimelineEngine ind;
        Set m;
        m.d[0].add(0.0, 0.0); m.d[0].add(8.0, 8.0);
        m.d[5].add(0.0, 0.0); m.d[5].add(8.0, 8.0);
        ind.lanes[5].setPlaying(1);                    // only lane 5 runs
        for (int i = 0; i < 48000; i++) ind.tick(m.p); // 2 beats
        near("a stopped lane holds its position", ind.lanes[0].playhead, 0.0);
        near("the running lane advanced", ind.lanes[5].playhead, 2.0, 1e-9);
        near("the stopped lane's value holds", ind.lanes[0].value, 0.0);
        near("the running lane's value moved", ind.lanes[5].value, 2.0, 1e-6);

        ind.lanes[5].seek(0.0);
        check("a seek on lane 5 does not bump lane 0's serial",
              ind.lanes[0].seekSerial == 0, ind.lanes[0].seekSerial, 0);
        check("lane 5's own serial bumped", ind.lanes[5].seekSerial > 0);

        ind.lanes[0].setPlaying(1);
        ind.lanes[0].seek(1.0);
        ind.lanes[5].seek(3.0);
        for (int i = 0; i < 24000; i++) ind.tick(m.p); // +1 beat each
        near("two lanes hold separate positions (lane 0)", ind.lanes[0].playhead, 2.0, 1e-9);
        near("two lanes hold separate positions (lane 5)", ind.lanes[5].playhead, 4.0, 1e-9);
    }

    printf("\nSeek serial (user seek vs loop wrap, per lane)\n");
    {
        TimelineEngine w;
        Set m;
        w.setLoopEnd(4.0);
        w.lanes[2].setPlaying(1);
        int s1 = w.lanes[2].seekSerial;
        int sawWrap = 0;
        for (int i = 0; i < 48000 * 3 && !sawWrap; i++)
        { w.tick(m.p); if (w.lanes[2].wrapped) sawWrap = 1; }
        check("the loop wraps", sawWrap == 1);
        check("a loop wrap does NOT bump the serial", w.lanes[2].seekSerial == s1);
        check("the wrap lands on beat 0", w.lanes[2].playhead < 1e-3,
              w.lanes[2].playhead, 0.0);
        check("a lane that is not playing does not wrap", w.lanes[3].wrapped == 0);
    }

    printf("\nTempo changes keep musical position\n");
    e.setLoopEnd(0.0);
    e.lanes[0].seek(2.0);
    e.setBpm(240.0);
    e.tick(ls.p);
    check("BPM change does not move the playhead",
          std::fabs(e.lanes[0].playhead - 2.0) < 1e-4, e.lanes[0].playhead, 2.0);
    near("BPM change does not move a node", ls.d[0].t[1], 4.0);
    near("240 BPM doubles beats per sample",
         e.beatsPerSampleFromBpm(), 240.0 / (60.0 * 48000.0));
    e.setBpm(1000.0); near("BPM clamps high", e.bpm, TL_BPM_MAX);
    e.setBpm(1.0);    near("BPM clamps low",  e.bpm, TL_BPM_MIN);

    printf("\nGlobal gestures reach every lane\n");
    {
        TimelineEngine g;
        Set m;
        g.setPlayingAll(1);
        int all = 1;
        for (int L = 0; L < TL_LANES; L++) if (!g.lanes[L].playing) all = 0;
        check("setPlayingAll starts all 16", all == 1);
        check("anyPlaying sees it", g.anyPlaying() == 1);
        for (int i = 0; i < 4800; i++) g.tick(m.p);
        g.seekAll(7.0);
        all = 1;
        for (int L = 0; L < TL_LANES; L++)
            if (std::fabs(g.lanes[L].playhead - 7.0) > 1e-12) all = 0;
        check("seekAll lands every lane on the beat", all == 1);
        g.setPlayingAll(0);
        check("setPlayingAll(0) stops all", g.anyPlaying() == 0);
    }

    printf("\nLaneData editing\n");
    LaneData m;
    m.insert(4.0, 1.0);
    m.insert(1.0, 2.0);
    m.insert(2.5, 3.0);
    check("insert keeps ascending order",
          m.t[0] == 1.0f && m.t[1] == 2.5f && m.t[2] == 4.0f);
    check("insert count", m.count() == 3, m.count(), 3);
    int dup = m.insert(2.5, 9.0);
    check("insert at an existing beat lands to its right", dup == 2 && m.v[2] == 9.0f, dup, 2);
    m.erase(2);
    m.erase(1);
    check("erase removes the right node", m.count() == 2 && m.t[1] == 4.0f);
    m.t[0] = 9.0f; m.resort();
    check("resort fixes a dragged node", m.t[0] == 4.0f && m.t[1] == 9.0f);
    check("indexOf follows the node", m.indexOf(9.0, 2.0) == 1 && m.indexOf(9.0, 5.0) == -1);
    near("lastBeat finds the furthest node", m.lastBeat(), 9.0);
    m.clear();
    check("clear empties", m.count() == 0 && m.empty());

    LaneData rng;
    for (int i = 0; i <= 8; i++) rng.add((double)i, (double)i);
    int removed = rng.eraseRange(2.0, 5.0);
    check("eraseRange removes (lo, hi]: 3,4,5", removed == 3 && rng.count() == 6
          && rng.t[2] == 2.0f && rng.t[3] == 6.0f, removed, 3);
    removed = rng.eraseRange(2.0, 2.0, true);
    check("eraseRange with incLo removes [lo, hi]", removed == 1 && rng.t[2] == 6.0f, removed, 1);
    removed = rng.eraseRange(100.0, 200.0);
    check("an empty range removes nothing", removed == 0, removed, 0);

    Set sb;
    sb.d[3].add(5.0, 0.0); sb.d[9].add(12.0, 0.0);
    near("lastBeatOf scans every lane", lastBeatOf(sb.p), 12.0);

    LaneData full;
    for (int i = 0; i < TL_MAX_NODES + 10; i++) full.add((double)i, 0.0);
    check("node ceiling holds", full.count() == TL_MAX_NODES, full.count(), TL_MAX_NODES);
    check("insert refuses a full lane", full.insert(0.5, 0.0) == -1 && full.full());

    printf("\nGuards\n");
    e.lanes[0].seek(-5.0);  near("seek clamps below zero", e.lanes[0].playhead, 0.0);
    e.lanes[0].seek(1e9);   near("seek clamps to the ceiling", e.lanes[0].playhead, TL_MAX_BEAT);
    e.lanes[0].setPlaying(1);
    e.lanes[0].playhead = TL_MAX_BEAT - 1e-6;
    e.lanes[0].tick(ls.p[0], 1.0, 0.0);
    check("the runaway ceiling stops that lane", e.lanes[0].playing == 0);

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASS", fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
