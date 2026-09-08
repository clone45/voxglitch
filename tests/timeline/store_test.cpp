// store_test.cpp — the per-lane pointer-swap store (LaneStore): publish,
// retire-by-generation, no use-after-free, no leak, and the soft node cap.
// Build WITH AddressSanitizer so the "no leak / no use-after-free" checks
// are enforced by the tool rather than by inspection:
//   g++ -std=c++11 -O1 -g -fsanitize=address -fno-omit-frame-pointer
//       -I ../../src/modules/Timeline -o store_test store_test.cpp -lm && ./store_test
// (one command line; wrapped here only for width)
#include <cstdio>
#include <cmath>
#include "TimelineEngine.hpp"
using namespace timeline_dsp;

static int fails = 0;
static void check(const char* what, bool ok, double got = 0.0, double want = 0.0)
{
    if (ok) { printf("  ok    %s\n", what); return; }
    printf("  FAIL  %s   (got %.3f want %.3f)\n", what, got, want);
    fails++;
}

static LaneData ramp(int n, double step)
{
    LaneData d;
    for (int i = 0; i < n; i++) d.add(i * step, (double)i);
    return d;
}

int main()
{
    printf("Publish swaps the live pointer and retires the old one\n");
    {
        LaneStore s;
        const LaneData* old = s.load(3);
        check("a fresh store holds an empty lane", old && old->empty());
        s.publishCopy(3, ramp(4, 1.0));
        const LaneData* now = s.load(3);
        check("the audio thread sees the new snapshot", now != old && now->count() == 4,
              now->count(), 4);
        check("the old snapshot is retired, not freed", s.retiredCount() == 1, s.retiredCount(), 1);
        // Still readable: an audio block that loaded `old` may be mid-way.
        check("the retired snapshot is still valid memory", old->count() == 0);
        check("other lanes are untouched", s.load(4)->empty() && s.load(2)->empty());
    }

    printf("\nRetire-by-generation: freed only once the audio thread has moved past\n");
    {
        LaneStore s;
        const LaneData* old = s.load(0);
        s.publishCopy(0, ramp(2, 1.0));          // retired at generation 0
        s.freeRetired();
        check("no generation has passed: nothing freed", s.retiredCount() == 1, s.retiredCount(), 1);
        s.bumpGeneration();                      // 1
        s.bumpGeneration();                      // 2
        s.freeRetired();
        check("generation == retire + LAG: still held", s.retiredCount() == 1, s.retiredCount(), 1);
        check("and still readable during the lag", old->count() == 0);
        s.bumpGeneration();                      // 3 > 0 + 2
        s.freeRetired();
        check("generation > retire + LAG: freed", s.retiredCount() == 0, s.retiredCount(), 0);
        // `old` is now dangling; ASan would flag any read of it here.
    }

    printf("\nEdits while the audio thread is idle pile up, then drain\n");
    {
        LaneStore s;
        for (int i = 0; i < 50; i++) s.publishCopy(5, ramp(i, 0.5));
        check("50 edits without a generation tick: 50 retired", s.retiredCount() == 50,
              s.retiredCount(), 50);
        check("the live snapshot is the last published", s.lane(5).count() == 49,
              s.lane(5).count(), 49);
        for (int g = 0; g < 3; g++) s.bumpGeneration();
        s.freeRetired();
        check("three generations later all 50 are freed", s.retiredCount() == 0,
              s.retiredCount(), 0);
    }

    printf("\nInterleaved audio/UI simulation keeps the backlog bounded\n");
    {
        LaneStore s;
        TimelineEngine e;
        e.setPlayingAll(1);
        size_t maxRetired = 0;
        for (int frame = 0; frame < 2000; frame++)
        {
            // UI: an edit on a rotating lane
            LaneData d = s.lane(frame % TL_LANES);
            d.add((double)frame, 1.0);
            s.publishCopy(frame % TL_LANES, d);
            // audio: one block reads every lane, then bumps
            const LaneData* ld[TL_LANES];
            s.loadAll(ld);
            for (int k = 0; k < 64; k++) e.tick(ld);
            s.bumpGeneration();
            // UI: housekeeping at the frame end
            s.freeRetired();
            if (s.retiredCount() > maxRetired) maxRetired = s.retiredCount();
        }
        check("retired backlog never exceeds LAG + 1", maxRetired <= LaneStore::RETIRE_LAG + 1,
              maxRetired, LaneStore::RETIRE_LAG + 1);
        check("edits accumulated in the live snapshots",
              s.lane(0).count() == 2000 / TL_LANES, s.lane(0).count(), 2000 / TL_LANES);
        check("the engine read the growing lanes without incident", e.lanes[0].value == 1.0);
    }

    printf("\nOwnership edge cases\n");
    {
        LaneStore s;
        s.publish(-1, new LaneData());           // out of range: must not leak
        s.publish(TL_LANES, new LaneData());
        s.publish(0, 0);                         // null: ignored
        check("bad publishes retire nothing", s.retiredCount() == 0, s.retiredCount(), 0);
        s.publishCopy(1, ramp(3, 1.0));
        s.freeRetired(true);
        check("freeRetired(all) frees regardless of generation", s.retiredCount() == 0);
        // Destructor frees live + retired: ASan's leak check at exit proves it.
        s.publishCopy(2, ramp(3, 1.0));
    }

    printf("\nThe soft cap\n");
    {
        LaneData d;
        int added = 0;
        for (int i = 0; i < TL_MAX_NODES + 100; i++) if (d.add((double)i, 0.0)) added++;
        check("add stops at TL_MAX_NODES", added == TL_MAX_NODES && d.full(), added, TL_MAX_NODES);
        check("insert refuses when full", d.insert(0.5, 1.0) == -1);
        d.erase(0);
        check("erasing one makes room for one", !d.full() && d.insert(0.5, 1.0) == 0);
        check("a lane holds far more than the old 256", TL_MAX_NODES >= 8192);
    }

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "ALL PASS", fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
