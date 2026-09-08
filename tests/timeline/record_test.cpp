// record_test.cpp — latch-mode recording: the audio-side Recorder (grid
// capture, take start/end, loop wraps, poly lane mapping) and the UI-side
// TakeAssembler (passed-over nodes replaced, untouched regions kept, the
// take-end simplifier). Mirrors the module's process() wiring in a small
// harness, so re-sync if that wiring changes.
//   g++ -std=c++11 -O2 -I ../../src/modules/Timeline -o record_test record_test.cpp -lm && ./record_test
#include <cstdio>
#include <cmath>
#include "TimelineEngine.hpp"
using namespace timeline_dsp;

static int fails = 0;
static void check(const char* what, bool ok, double got = 0.0, double want = 0.0)
{
    if (ok) { printf("  ok    %s\n", what); return; }
    printf("  FAIL  %s   (got %.4f want %.4f)\n", what, got, want);
    fails++;
}

// The module's recording path, stubbed: a store, an engine, the recorder,
// the ring, the assembler. `input(L)` is what REC IN carries for lane L.
struct Rig
{
    LaneStore store;
    TimelineEngine engine;
    Recorder rec;
    CaptureRing ring;
    TakeAssembler take;
    double SR = 48000.0;
    bool armed = false;
    int channels = 0;           // REC IN channel count (0 = unpatched)
    int selected = 0;           // LANE_PARAM
    int rate = 2;               // record-rate index (2 = 1/4)
    float volts[TL_LANES];
    int captures[TL_LANES];
    bool laneFull[TL_LANES];
    std::vector<Capture> log;   // every capture drained, in order

    struct LaneEditRecord
    {
        int lanes = 0;
        LaneData before[TL_LANES];
        LaneData after[TL_LANES];
        int touched[TL_LANES];
    };
    LaneEditRecord undo;
    LaneEditRecord prevUndo;    // the push before `undo`
    int undoPushes = 0;

    Rig()
    {
        engine.setSampleRate(SR);
        for (int L = 0; L < TL_LANES; L++) { volts[L] = 0.f; captures[L] = 0; laneFull[L] = false; }
    }

    // What the module does per sample.
    void sample()
    {
        const LaneData* ld[TL_LANES];
        store.loadAll(ld);
        engine.tick(ld);
        unsigned want = Recorder::targetMask(channels, selected);
        rec.process(armed, want, engine, recRateBeats(rate), volts, ld, ring);
        store.bumpGeneration();
    }

    void run(int samples) { for (int i = 0; i < samples; i++) sample(); }
    void runBeats(double beats) { run((int)std::floor(beats * 60.0 * SR / engine.bpm + 0.5)); }

    // What the editor's finishTake() does: one undo push per finished take.
    void finishTake()
    {
        if (!take.complete()) return;
        prevUndo = undo;
        undo = LaneEditRecord();
        for (int L = 0; L < TL_LANES; L++)
        {
            if (!take.lanes[L].open) continue;
            undo.touched[L] = 1;
            undo.before[L] = take.lanes[L].before;
            undo.after[L] = store.lane(L);
            undo.lanes++;
        }
        undoPushes++;
        take.reset();
    }

    // What the editor's step() does per frame.
    void drain()
    {
        Capture c;
        while (ring.pop(c))
        {
            captures[c.lane]++;
            log.push_back(c);
            if (c.kind == CAP_START) finishTake();
            LaneData d = store.lane(c.lane);
            if (take.apply(c, d))
            {
                store.publishCopy(c.lane, d);
                laneFull[c.lane] = take.lanes[c.lane].full != 0;
            }
        }
        finishTake();
        store.freeRetired();
    }

    // The output the module would put on the poly jack for lane L.
    float output(int L)
    {
        if (rec.active && rec.targets(L)) return volts[L];
        return (float)engine.lanes[L].value;
    }
};

static bool hasBeat(const LaneData& d, double beat, double tol = 1e-4)
{
    for (int i = 0; i < d.count(); i++) if (std::fabs(d.t[i] - beat) <= tol) return true;
    return false;
}

int main()
{
    printf("Grid capture at 1/4\n");
    {
        Rig r; r.rate = 2; r.selected = 0; r.channels = 1;
        r.volts[0] = 3.f;
        r.armed = true;
        r.run(10);
        check("arming while stopped starts no take", !r.rec.active && r.ring.pending() == 0);
        r.engine.lanes[0].setPlaying(1);
        r.runBeats(4.0);
        check("the take started on PLAY", r.rec.active == 1);
        r.armed = false;
        r.run(1);
        check("disarming ends the take", r.rec.active == 0);
        r.drain();
        // START at 0, grid lines 1,2,3,4, END just past 4
        check("START + 4 grid lines + END = 6 captures", r.captures[0] == 6, r.captures[0], 6);
        check("one undo step for the take", r.undoPushes == 1, r.undoPushes, 1);
        const LaneData& d = r.store.lane(0);
        check("a constant input simplifies to the two protected endpoints", d.count() == 2,
              d.count(), 2);
        check("first node at the take start", hasBeat(d, 0.0));
        check("last node at the take end", std::fabs(d.t[d.count() - 1] - 4.0) < 1e-3,
              d.t[d.count() - 1], 4.0);
        check("recorded value is the input", d.v[0] == 3.f && d.v[1] == 3.f);
        check("the undo record holds the empty lane before", r.undo.lanes == 1
              && r.undo.before[0].empty() && r.undo.after[0].count() == 2);
    }

    printf("\nGrid capture at 1/32, on absolute grid positions\n");
    {
        Rig r; r.rate = 5; r.selected = 2; r.channels = 0;   // unpatched: records 0 V
        r.engine.lanes[2].seek(0.7);                         // off-grid start
        r.engine.lanes[2].setPlaying(1);
        r.armed = true;
        r.runBeats(2.0);                                     // 0.7 -> 2.7
        r.armed = false; r.run(1);
        // grid lines crossed: 0.75, 0.875, ..., 2.625 = 16 lines
        int expect = 2 + 16;
        r.drain();
        check("START + 16 grid lines + END", r.captures[2] == expect, r.captures[2], expect);
        check("nothing captured on other lanes", r.captures[0] == 0 && r.captures[1] == 0);
        check("an unpatched REC IN records 0 V", r.store.lane(2).v[0] == 0.f);
        check("the take's first node sits at the off-grid start beat",
              std::fabs(r.store.lane(2).t[0] - 0.7) < 1e-4, r.store.lane(2).t[0], 0.7);
    }
    {
        // Grid positions are absolute beats: a capture lands ON the line.
        Rig r; r.rate = 3; r.selected = 0; r.channels = 1;   // 1/8 = 0.5 beat
        r.engine.lanes[0].seek(1.3);
        r.engine.lanes[0].setPlaying(1);
        r.armed = true;
        r.runBeats(1.0);
        r.armed = false; r.run(1);
        r.drain();
        // Interior nodes need distinct values to survive the simplifier, so
        // check the capture beats through the assembler's region instead.
        check("captures at 1.5 and 2.0 were the grid lines crossed",
              r.captures[0] == 4, r.captures[0], 4);
    }

    printf("\nLatch: passed-over nodes replaced, the rest untouched\n");
    {
        Rig r; r.rate = 2; r.selected = 0; r.channels = 1;
        LaneData pre;
        pre.add(0.0, 1.0); pre.add(2.0, 2.0); pre.add(3.5, 3.0); pre.add(6.0, 4.0); pre.add(8.0, 5.0);
        r.store.publishCopy(0, pre);
        r.engine.lanes[0].seek(1.0);
        r.engine.lanes[0].setPlaying(1);
        r.volts[0] = -5.f;
        r.armed = true;
        r.runBeats(3.0);                       // 1 -> 4
        r.armed = false; r.run(1);
        r.drain();
        const LaneData& d = r.store.lane(0);
        check("the node before the take (beat 0) survives", hasBeat(d, 0.0) && d.v[0] == 1.f);
        check("nodes inside the sweep (2.0, 3.5) were replaced",
              !(hasBeat(d, 2.0) && d.v[1] == 2.f) && !hasBeat(d, 3.5));
        check("nodes after the take (6, 8) survive", hasBeat(d, 6.0) && hasBeat(d, 8.0));
        check("the recorded value sits between", hasBeat(d, 1.0) && d.v[1] == -5.f);
        // Undo restores the pre-take lane exactly.
        check("undo record's before == the pre-take lane", r.undo.before[0].count() == 5
              && r.undo.before[0].t[2] == 3.5f);
    }

    printf("\nLoop wrap replaces (prev, loopEnd] and [0, B]\n");
    {
        Rig r; r.rate = 2; r.selected = 0; r.channels = 1;
        LaneData pre;
        pre.add(0.0, 9.0); pre.add(0.5, 9.0); pre.add(2.5, 9.0); pre.add(3.5, 9.0);
        r.store.publishCopy(0, pre);
        r.engine.setLoopEnd(4.0);
        r.engine.lanes[0].seek(3.0);
        r.engine.lanes[0].setPlaying(1);
        r.volts[0] = 1.f;
        r.armed = true;
        r.runBeats(2.0);                       // 3 -> wrap at 4 -> 1
        check("the lane wrapped during the take", r.engine.lanes[0].playhead < 1.5);
        r.armed = false; r.run(1);
        r.drain();
        const LaneData& d = r.store.lane(0);
        check("node at 3.5 (between prev and the loop end) was replaced", !hasBeat(d, 3.5));
        check("node at 0.5 (in [0, B] after the wrap) was replaced", !hasBeat(d, 0.5)
              || d.v[d.count() - 1] != 9.f);
        check("node at 2.5 (never passed over) survives with its value",
              hasBeat(d, 2.5));
        bool kept25 = false;
        for (int i = 0; i < d.count(); i++) if (std::fabs(d.t[i] - 2.5) < 1e-4 && d.v[i] == 9.f) kept25 = true;
        check("... and its 9 V value", kept25);
        check("a static input writes no node at the wrap (beat 0)", !hasBeat(d, 0.0));
        check("the take end pins the latch edge just past beat 1",
              d.count() >= 2 && hasBeat(d, r.engine.lanes[0].playhead, 1e-3) && d.v[0] == 1.f);
        check("the take's start node at 3 is protected", hasBeat(d, 3.0));
    }

    printf("\nThe simplifier\n");
    {
        LaneData d;
        // A ramp with sub-tolerance noise, plus a real corner at beat 4.
        for (int i = 0; i <= 8; i++)
            d.add((double)i, (i <= 4 ? (double)i : 8.0 - (double)i) + ((i % 2) ? 0.005 : -0.005));
        float keep[2] = { 0.f, 8.f };
        std::vector<float> rec(d.t);              // every node was recorded
        simplifyRegion(d, 0.0, 8.0, TL_REC_TOL, keep, 2, rec);
        check("collinear nodes within 0.02 V are removed", d.count() == 3, d.count(), 3);
        check("the endpoints are kept", d.t[0] == 0.f && d.t[2] == 8.f);
        check("the corner survives", d.t[1] == 4.f, d.t[1], 4.0);

        LaneData e;
        for (int i = 0; i <= 4; i++) e.add((double)i, 2.0);
        e.add(10.0, 7.0); e.add(11.0, 7.0);
        float keep2[2] = { 1.f, 3.f };
        std::vector<float> rec2; rec2.push_back(1.f); rec2.push_back(2.f); rec2.push_back(3.f);
        simplifyRegion(e, 1.0, 3.0, TL_REC_TOL, keep2, 2, rec2);
        check("only the region is touched: nodes outside it stay",
              hasBeat(e, 0.0) && hasBeat(e, 4.0) && hasBeat(e, 10.0) && hasBeat(e, 11.0));
        check("the region's interior collapsed onto its protected ends",
              hasBeat(e, 1.0) && hasBeat(e, 3.0) && !hasBeat(e, 2.0));

        LaneData f;
        f.add(0.0, 0.0); f.add(1.0, 0.0, 2.0); f.add(2.0, 0.0); f.add(3.0, 0.0);
        float keep3[2] = { 0.f, 3.f };
        std::vector<float> rec3(f.t);
        simplifyRegion(f, 0.0, 3.0, TL_REC_TOL, keep3, 2, rec3);
        check("a node carrying a bend is never removed", hasBeat(f, 1.0) && !hasBeat(f, 2.0));

        LaneData g;
        g.add(0.0, 0.0); g.add(1.0, 5.0);
        std::vector<float> recg(g.t);
        simplifyRegion(g, 0.0, 1.0, TL_REC_TOL, keep3, 2, recg);
        check("two nodes are left alone", g.count() == 2);

        // Only recorded nodes are candidates: a hand-placed collinear node
        // inside the region survives even though it lies on the chord.
        LaneData h;
        for (int i = 0; i <= 4; i++) h.add((double)i, 1.0);
        float keeph[2] = { 0.f, 4.f };
        std::vector<float> rech; rech.push_back(0.f); rech.push_back(1.f); rech.push_back(3.f); rech.push_back(4.f);
        simplifyRegion(h, 0.0, 4.0, TL_REC_TOL, keeph, 2, rech);
        check("a node the take did not insert is never removed",
              hasBeat(h, 2.0) && !hasBeat(h, 1.0) && !hasBeat(h, 3.0), h.count(), 3);
    }

    printf("\nPoly lane mapping\n");
    {
        check("mono -> the selected lane", Recorder::targetMask(1, 7) == (1u << 7));
        check("unpatched -> the selected lane", Recorder::targetMask(0, 7) == (1u << 7));
        check("4 channels -> lanes 0..3", Recorder::targetMask(4, 7) == 0xFu);
        check("16 channels -> all 16 lanes", Recorder::targetMask(16, 0) == 0xFFFFu);
        check("selected lane clamps", Recorder::targetMask(1, 99) == (1u << 15));

        Rig r; r.rate = 2; r.channels = 3; r.selected = 9;
        r.volts[0] = 1.f; r.volts[1] = 2.f; r.volts[2] = 3.f; r.volts[9] = 9.f;
        LaneData parked; parked.add(0.0, 7.0); parked.add(4.0, 7.0);
        r.store.publishCopy(2, parked);
        r.engine.lanes[2].seek(2.0);           // lane 2 parked at beat 2, not playing
        r.engine.lanes[0].setPlaying(1);
        r.engine.lanes[1].setPlaying(1);       // lanes 0 and 1 run
        r.armed = true;
        r.runBeats(2.0);
        check("a stopped target lane is not bypassed during the take",
              r.rec.active == 1 && !r.rec.targets(2) && r.output(2) == 7.f, r.output(2), 7.0);
        r.armed = false; r.run(1);
        r.drain();
        check("the playing lanes took part (START + 2 grid lines + END)",
              r.captures[0] == 4 && r.captures[1] == 4, r.captures[1], 4);
        check("the stopped lane recorded nothing", r.captures[2] == 0, r.captures[2], 0);
        check("the stopped lane's nodes are untouched",
              r.store.lane(2).count() == 2 && r.store.lane(2).v[0] == 7.f && r.store.lane(2).t[1] == 4.f);
        check("the selected lane is NOT a target when poly", r.captures[9] == 0);
        check("each lane recorded its own channel",
              r.store.lane(0).v[0] == 1.f && r.store.lane(1).v[0] == 2.f);
        check("one undo covering the two playing lanes", r.undoPushes == 1 && r.undo.lanes == 2, r.undo.lanes, 2);
    }

    printf("\nBypass and the take boundary\n");
    {
        Rig r; r.rate = 2; r.selected = 0; r.channels = 1;
        LaneData pre; pre.add(0.0, 7.0); pre.add(4.0, 7.0);
        r.store.publishCopy(0, pre);
        r.volts[0] = -2.f;
        r.engine.lanes[0].setPlaying(1);
        r.run(100);
        check("before arming the lane plays its nodes", r.output(0) == 7.f, r.output(0), 7.0);
        r.armed = true; r.run(1);
        check("during the take the output is the live input", r.output(0) == -2.f, r.output(0), -2.0);
        check("an untargeted lane keeps playing its nodes", r.output(1) == 0.f);
        r.armed = false; r.run(1);
        r.drain();
        r.run(1);
        // The take lasted a sample, so the lane is now interpolating from
        // the recorded -2 V node toward the old 7 V node at beat 4.
        check("after the take the lane plays its nodes again (not the input)",
              std::fabs(r.output(0) - (-2.f)) < 0.01 && r.output(0) != -2.f, r.output(0), -2.0);

        // STOP ends the take too.
        Rig s; s.rate = 2; s.channels = 1;
        s.engine.lanes[0].setPlaying(1);
        s.armed = true; s.run(10);
        check("take running", s.rec.active == 1);
        s.engine.lanes[0].setPlaying(0); s.run(1);
        check("stopping the transport ends the take while still armed", s.rec.active == 0);
        s.engine.lanes[0].setPlaying(1); s.run(1);
        check("playing again while armed starts a new take", s.rec.active == 1);
    }

    printf("\nLane full\n");
    {
        Rig r; r.rate = 5; r.selected = 0; r.channels = 1;
        LaneData big;
        for (int i = 0; i < TL_MAX_NODES; i++) big.add(1000.0 + i * 0.001, 0.0);
        r.store.publishCopy(0, big);
        r.engine.lanes[0].setPlaying(1);
        r.armed = true;
        r.runBeats(1.0);
        r.armed = false; r.run(1);
        r.drain();
        check("a full lane stops capturing (only START and END)", r.captures[0] == 2, r.captures[0], 2);
        check("the lane-full flag is raised for the editor", r.laneFull[0]);
        check("the cap was never exceeded", r.store.lane(0).count() <= TL_MAX_NODES);
    }

    printf("\nRing overflow drops the NEWEST and counts it (no torn records)\n");
    {
        CaptureRing ring;
        Capture c;
        for (int i = 0; i < TL_CAPTURE_RING + 10; i++)
        {
            c.beat = (float)i; c.volt = (float)i; c.lane = i % TL_LANES;   // self-consistent record
            ring.push(c);
        }
        check("10 drops counted", ring.dropped.load() == 10u, ring.dropped.load(), 10);
        Capture out;
        bool ok = ring.pop(out);
        check("the oldest entry survives intact", ok && out.beat == 0.f, out.beat, 0.0);
        int n = 1;
        bool intact = (out.beat == out.volt && out.lane == 0);
        while (ring.pop(out))
        {
            if (out.beat != (float)n || out.volt != (float)n || out.lane != n % TL_LANES) intact = false;
            n++;
        }
        check("every record reads back exactly as pushed", intact);
        check("and the ring drained exactly its capacity", n == TL_CAPTURE_RING, n, TL_CAPTURE_RING);
        c.beat = 99.f; ring.push(c);
        check("space frees as the consumer catches up", ring.pop(out) && out.beat == 99.f);
    }

    printf("\nReview regressions\n");
    {
        // 1. A take's END and the next take's START drained in one frame:
        //    both takes get their own undo push with the right befores.
        Rig r; r.rate = 2; r.channels = 1; r.volts[0] = 3.f;
        r.engine.lanes[0].setPlaying(1); r.armed = true; r.runBeats(2.0);
        r.engine.lanes[0].setPlaying(0); r.run(1);          // take 1 ends
        r.engine.lanes[0].setPlaying(1); r.volts[0] = -3.f; r.run(10);   // take 2 begins, same frame
        r.drain();
        check("take 1 pushed its undo before take 2's START was applied",
              r.undoPushes == 1 && r.undo.before[0].empty() && r.undo.after[0].count() == 2,
              r.undoPushes, 1);
        check("take 2 is open on that lane", r.take.inTake(0));
        r.armed = false; r.run(1); r.drain();
        check("take 2 pushed its own undo", r.undoPushes == 2, r.undoPushes, 2);
        check("take 2's before is take 1's after",
              r.undo.before[0].count() == 2 && r.undo.before[0].v[0] == 3.f
              && r.undo.after[0].v[r.undo.after[0].count() - 1] == -3.f);
    }
    {
        // 2. A wrap take: the middle the playhead never reached keeps its
        //    hand-placed nodes through the simplifier.
        Rig r; r.rate = 2; r.channels = 1;
        LaneData pre; pre.add(1.5, 5.0); pre.add(2.0, 5.0); pre.add(2.5, 5.0);
        r.store.publishCopy(0, pre);
        r.engine.setLoopEnd(4.0); r.engine.lanes[0].seek(3.0); r.engine.lanes[0].setPlaying(1);
        r.volts[0] = 5.f; r.armed = true;
        r.runBeats(2.0);                                    // 3 -> wrap -> 1
        r.armed = false; r.run(1); r.drain();
        const LaneData& d = r.store.lane(0);
        check("untouched nodes at 1.5, 2.0, 2.5 survive the take-end simplifier",
              hasBeat(d, 1.5) && hasBeat(d, 2.0) && hasBeat(d, 2.5), d.count(), 0);
        check("the take's own collinear nodes were still simplified", d.count() <= 7, d.count(), 7);
    }
    {
        // 3. A stopped poly target lane: nothing recorded, nothing bypassed.
        Rig r; r.rate = 2; r.channels = 3;
        LaneData pre; pre.add(0.0, 7.0); pre.add(4.0, 7.0); r.store.publishCopy(2, pre);
        r.engine.lanes[2].seek(2.0);
        r.engine.lanes[1].setPlaying(1); r.volts[2] = -1.f; r.armed = true; r.runBeats(1.0);
        check("the parked lane still outputs its nodes mid-take", r.output(2) == 7.f, r.output(2), 7.0);
        r.armed = false; r.run(1); r.drain();
        check("no coincident START/END nodes on the parked lane",
              r.store.lane(2).count() == 2 && r.captures[2] == 0, r.store.lane(2).count(), 2);
    }
    {
        // 4. A forward scrub mid-take: the skipped nodes are not passed over.
        Rig r; r.rate = 2; r.channels = 1; r.volts[0] = 1.f;
        LaneData pre; for (int i = 10; i < 20; i++) pre.add((double)i, 9.0); r.store.publishCopy(0, pre);
        r.engine.lanes[0].setPlaying(1); r.armed = true; r.runBeats(0.5);
        r.engine.lanes[0].seek(30.0); r.run(10);
        r.armed = false; r.run(1); r.drain();
        const LaneData& d = r.store.lane(0);
        bool all = true;
        for (int i = 10; i < 20; i++) if (!hasBeat(d, (double)i)) all = false;
        check("nodes 10..19 survive a seek from 0.5 to 30", all, d.count(), 12);
        bool seekAt30 = false, anyAt29 = false;
        for (size_t i = 0; i < r.log.size(); i++)
        {
            if (r.log[i].seek && r.log[i].beat == 30.f) seekAt30 = true;
            if (r.log[i].beat == 29.f) anyAt29 = true;
        }
        check("the seek capture landed on the seek target (flagged), not the grid cell below",
              seekAt30 && !anyAt29);
        check("the take end sits just past 30", d.t[d.count() - 1] > 30.f && d.t[d.count() - 1] < 30.01f);
    }
    {
        // 4b. A seek BACK then forward again re-records only what it passes.
        Rig r; r.rate = 2; r.channels = 1; r.volts[0] = 1.f;
        LaneData pre; pre.add(6.0, 9.0); pre.add(7.0, 9.0); r.store.publishCopy(0, pre);
        r.engine.lanes[0].seek(5.0); r.engine.lanes[0].setPlaying(1); r.armed = true; r.runBeats(0.5);
        r.engine.lanes[0].seek(1.0); r.runBeats(0.5);      // 1 -> 1.5, never near 6..7
        r.armed = false; r.run(1); r.drain();
        check("a backward seek leaves the nodes ahead of it alone",
              hasBeat(r.store.lane(0), 6.0) && hasBeat(r.store.lane(0), 7.0));
    }

    printf("\nNodes only where the value changes\n");
    {
        // A static input across 8 grid steps: START and END only.
        Rig r; r.rate = 2; r.channels = 1; r.volts[0] = 2.f;
        r.engine.lanes[0].setPlaying(1); r.armed = true; r.runBeats(8.2);
        r.armed = false; r.run(1); r.drain();
        const LaneData& d = r.store.lane(0);
        check("static input: 10 captures but only START and END nodes",
              r.captures[0] == 10 && d.count() == 2, d.count(), 2);
        check("both carry the input", d.v[0] == 2.f && d.v[1] == 2.f && d.t[0] < 0.001f && d.t[1] > 8.f);
    }
    {
        // A step at grid 4: the hold node at grid 3 and the new node at 4.
        Rig r; r.rate = 2; r.channels = 1; r.volts[0] = 1.f;
        r.engine.lanes[0].setPlaying(1); r.armed = true; r.runBeats(3.5);
        r.volts[0] = 5.f; r.runBeats(4.5);                 // crosses 4..8 at 5 V
        r.armed = false; r.run(1); r.drain();
        const LaneData& d = r.store.lane(0);
        check("step change: START, hold at 3, change at 4, END = 4 nodes", d.count() == 4, d.count(), 4);
        check("the hold node sits at grid 3 with the OLD value", hasBeat(d, 3.0) && d.v[1] == 1.f && d.t[1] == 3.f);
        check("the change node sits at grid 4 with the new value", d.t[2] == 4.f && d.v[2] == 5.f);
        check("no node at grids 1, 2, 5, 6, 7", !hasBeat(d, 1.0) && !hasBeat(d, 2.0) && !hasBeat(d, 5.0)
              && !hasBeat(d, 6.0) && !hasBeat(d, 7.0));
        check("the END pins the latch edge at 5 V", d.v[3] == 5.f && d.t[3] > 8.f);
    }
    {
        // A (curved) ramp: every grid step changes the value, so every
        // grid step gets a node.
        Rig r; r.rate = 2; r.channels = 1; r.volts[0] = 0.f;
        r.engine.lanes[0].setPlaying(1); r.armed = true; r.run(10);
        for (int k = 1; k <= 8; k++) { r.volts[0] = 0.1f * k * k; r.runBeats(1.0); }
        r.armed = false; r.run(1); r.drain();
        const LaneData& d = r.store.lane(0);
        bool all = true;
        for (int k = 1; k <= 7; k++)
        {
            bool found = false;
            for (int i = 0; i < d.count(); i++)
                if (d.t[i] == (float)k && d.v[i] == 0.1f * k * k) found = true;
            if (!found) all = false;
        }
        check("ramp: a node at every grid step with that step's value", all, d.count(), 9);
        // The END pin lands one sample after grid 8 with grid 8's value, so
        // the simplifier folds the grid-8 node into it: START + 7 + END.
        check("the last step is carried by the END pin", d.count() == 9
              && d.t[8] > 8.f && d.t[8] < 8.001f && d.v[8] == 0.1f * 64, d.count(), 9);
    }
    {
        // A hold spanning the loop wrap: pinned at the last grid point
        // before the loop end, nothing at the wrap, then the change.
        Rig r; r.rate = 2; r.channels = 1; r.volts[0] = 1.f;
        r.engine.setLoopEnd(4.0); r.engine.lanes[0].seek(2.0); r.engine.lanes[0].setPlaying(1);
        r.armed = true; r.runBeats(2.5);                    // 2 -> wrap -> 0.5, still 1 V
        r.volts[0] = 5.f; r.runBeats(1.0);                  // crosses grid 1 at 5 V
        r.armed = false; r.run(1); r.drain();
        const LaneData& d = r.store.lane(0);
        bool hold3 = false, hold0 = false, change1 = false;
        for (int i = 0; i < d.count(); i++)
        {
            if (d.t[i] == 3.f && d.v[i] == 1.f) hold3 = true;
            if (d.t[i] == 0.f && d.v[i] == 1.f) hold0 = true;
            if (d.t[i] == 1.f && d.v[i] == 5.f) change1 = true;
        }
        check("hold pinned at grid 3, the last grid point before the loop end", hold3);
        check("the wrap capture (beat 0) became the hold node before the change", hold0);
        check("the change node at grid 1", change1);
        check("START(2), 3, 0, 1, END = 5 nodes", d.count() == 5, d.count(), 5);
    }

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASS", fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
