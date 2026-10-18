// Euclid engine: Sequencer::process under the VCV reset standard, one case per
// row of the table in docs/implementation_plans/euclid-migration.md §4.1, at
// 48 kHz (a 48-sample window), plus the window-length and struck invariants.
//
//   g++ -std=c++11 -O2 -I ../../src -o engine_test engine_test.cpp && ./engine_test
//
// The engine takes the window as a sample count; these tests pass 48 (the
// standard's 1 ms at 48 kHz) as a literal, so they pin the ENGINE's contract.
// ResetClockWindow::samplesFor is pinned in seq3_parity_test.cpp.

#include "modules/Euclid/EuclidEngine.hpp"
#include "check.hpp"

#include <cstdio>

using namespace euclid;

static const int W = 48;                     // 1 ms at 48 kHz

typedef Sequencer::Out Out;

// A loaded sequencer with its knob values as state, so a case reads as a
// sequence of samples: idle(), clock(), reset(), both().
struct Rig
{
    Sequencer seq;
    int hits, steps, rot, window;

    Rig(int h, int s, int r, int w = W) : hits(h), steps(s), rot(r), window(w)
    {
        seq.load(w);
    }

    Out idle()  { return seq.process(false, false, window, hits, steps, rot); }
    Out clock() { return seq.process(false, true,  window, hits, steps, rot); }
    Out reset() { return seq.process(true,  false, window, hits, steps, rot); }
    Out both()  { return seq.process(true,  true,  window, hits, steps, rot); }

    void idleFor(int n) { for (int i = 0; i < n; i++) idle(); }

    // Let the load / reset window expire so a clock is accepted.
    void settle() { idleFor(window); }

    // n accepted clocks, one sample apart (no window is open once settled).
    void clocks(int n) { for (int i = 0; i < n; i++) clock(); }
};

static void loadTests()
{
    std::printf("patch load\n");
    Sequencer fresh;
    checkEq("a fresh Sequencer is on step 1", fresh.position, 0);
    check("a fresh Sequencer has gate_high false", !fresh.gate_high);
    check("a fresh Sequencer has no window open", !fresh.window.open());

    Rig r(4, 16, 0);
    check("load arms the window", r.seq.window.open());
    checkEq("load leaves step 1 current", r.seq.position, 0);
    Out o = r.idle();
    checkEq("the first sample is on step 1", o.position, 0);
    check("GATE is high (step 1 of 4/16 is a hit)", o.gate);
    check("nothing fires at load", !o.struck);
    check("no gate gap at load", !o.gate_gap);

    // Impromptu: "the clock is also ignored during 1ms when the module is
    // initialized" — a clock on the sample after load is the implicit reset's.
    o = r.clock();
    checkEq("a clock one sample after load is dropped", o.position, 0);
    check("and strikes nothing", !o.struck);

    // load() arms the window before any sample runs (it is "sample -1"), so
    // the first 48 samples are inside it: the 48th is still swallowed and a
    // clock after 48 samples of silence is the first one accepted.
    Rig r2(4, 16, 0);
    r2.idleFor(W - 1);
    o = r2.clock();
    checkEq("the 48th sample after load is still inside the window", o.position, 0);
    Rig r2b(4, 16, 0);
    r2b.idleFor(W);
    o = r2b.clock();
    checkEq("a clock after 48 samples of silence advances to step 2", o.position, 1);

    Rig r3(0, 16, 0);
    o = r3.idle();
    check("load with hits 0 shows GATE low", !o.gate);

    // load() on a running sequencer is a full reload: position 0, no gap.
    Rig r4(16, 16, 0);
    r4.settle();
    r4.clocks(5);
    o = r4.idle();
    check("(running with GATE high)", o.gate && o.position == 5);
    r4.seq.load(W);
    checkEq("load on a running sequencer rewinds to step 1", r4.seq.position, 0);
    check("and clears gate_high", !r4.seq.gate_high);
    o = r4.idle();
    check("so the next sample carries no gap", !o.gate_gap && o.gate);
}

static void resetTests()
{
    std::printf("reset strikes step 1 and arms the window\n");
    Rig r(4, 16, 0);
    r.settle();
    r.clocks(3);
    checkEq("(on step 4)", r.seq.position, 3);
    check("(no window open)", !r.seq.window.open());

    Out o = r.reset();
    checkEq("reset puts step 1 current", o.position, 0);
    check("reset strikes step 1 on that sample", o.struck);
    check("GATE follows step 1", o.gate);
    check("the window is armed", r.seq.window.open());
    checkEq("the window has 47 samples left after the reset sample", r.seq.window.remaining, W - 1);

    o = r.idle();
    check("the sample after a reset strikes nothing", !o.struck);
    checkEq("and stays on step 1", o.position, 0);

    // A reset never counts as a clock: three resets in a row stay on step 1.
    Rig r2(16, 16, 0);
    r2.settle();
    r2.clocks(4);
    for (int i = 0; i < 3; i++)
    {
        o = r2.reset();
        checkEq("a reset never advances", o.position, 0);
        check("each reset strikes step 1", o.struck);
    }

    // The window never blocks a reset, and a reset inside a window re-arms it
    // to the full length.
    Rig r3(16, 16, 0);
    r3.settle();
    r3.clocks(2);
    r3.reset();
    r3.idleFor(10);
    checkEq("(37 samples left)", r3.seq.window.remaining, W - 11);
    o = r3.reset();
    checkEq("a reset inside the window is honoured", o.position, 0);
    check("and strikes", o.struck);
    checkEq("and re-arms the window to the full length", r3.seq.window.remaining, W - 1);

    // A reset with step 1 a rest strikes nothing.
    Rig r4(0, 8, 0);
    r4.settle();
    o = r4.reset();
    check("a reset onto a rest strikes nothing", !o.struck);
    check("and GATE is low", !o.gate);
}

static void windowTests()
{
    std::printf("the 1 ms window\n");

    // Row 2: reset direct, clock one sample later (clock routed through a utility).
    Rig r1(16, 16, 0);
    r1.settle();
    r1.clocks(5);
    r1.reset();
    Out o = r1.clock();
    checkEq("a clock 1 sample after the reset is dropped", o.position, 0);
    check("and strikes nothing", !o.struck);

    // The last sample inside the window.
    Rig r2(16, 16, 0);
    r2.settle();
    r2.clocks(5);
    r2.reset();
    r2.idleFor(W - 2);
    o = r2.clock();
    checkEq("a clock 47 samples after the reset is dropped", o.position, 0);
    check("and strikes nothing", !o.struck);
    check("(the window is now closed)", !r2.seq.window.open());

    // The first sample outside it.
    Rig r3(16, 16, 0);
    r3.settle();
    r3.clocks(5);
    r3.reset();
    r3.idleFor(W - 1);
    o = r3.clock();
    checkEq("a clock 48 samples after the reset advances to step 2", o.position, 1);
    check("and strikes step 2", o.struck);

    // Every clock inside the window is dropped, not just one.
    Rig r4(16, 16, 0);
    r4.settle();
    r4.reset();
    for (int i = 1; i < W; i++)
    {
        o = r4.clock();
        if (o.position != 0 || o.struck) { check("every clock at +1..+47 is dropped", false); break; }
        if (i == W - 1) check("every clock at +1..+47 is dropped", true);
    }
    o = r4.clock();
    checkEq("the clock at +48 is accepted", o.position, 1);
}

static void sameSampleTests()
{
    std::printf("same-sample reset and clock (Clocked restarting its cycle)\n");
    Rig r(16, 16, 0);
    r.settle();
    r.clocks(5);
    checkEq("(on step 6)", r.seq.position, 5);

    Out o = r.both();
    checkEq("stays on step 1", o.position, 0);
    check("strikes once, at the reset", o.struck);

    int strikes = 0;
    for (int i = 0; i < W - 1; i++)
    {
        o = r.idle();
        if (o.struck) strikes++;
    }
    checkEq("no second strike follows", strikes, 0);
    checkEq("still on step 1 through the window", o.position, 0);

    o = r.clock();
    checkEq("the next edge after the window is step 2", o.position, 1);
    check("and strikes it", o.struck);
}

static void clockThenResetTests()
{
    std::printf("clock direct, reset one sample later (the standard's known limitation)\n");
    Rig r(16, 16, 0);
    r.settle();
    r.clocks(3);
    checkEq("(on step 4)", r.seq.position, 3);

    Out o = r.clock();
    checkEq("the clock plays step N+1", o.position, 4);
    check("and strikes it", o.struck);

    o = r.reset();
    checkEq("the reset one sample later plays step 1", o.position, 0);
    check("and strikes it: two hits one sample apart, as SEQ3", o.struck);
    check("GATE was high, so the gap is reported", o.gate_gap);
}

static void gateGapTests()
{
    std::printf("reset while GATE is high\n");
    Rig r(4, 16, 0);
    Out o = r.idle();
    check("(GATE high on step 1 of 4/16)", o.gate);

    o = r.reset();
    check("a reset while GATE is high reports gate_gap", o.gate_gap);
    check("the logical gate is still step 1's", o.gate);
    o = r.idle();
    check("the gap lasts one sample", !o.gate_gap);
    check("and GATE follows step 1", o.gate);

    // Step 2 of 4/16 is a rest: a reset from there needs no gap.
    r.settle();
    o = r.clock();
    check("(on step 2, a rest)", o.position == 1 && !o.gate);
    o = r.reset();
    check("a reset while GATE is low reports no gap", !o.gate_gap);
    check("but still strikes step 1", o.struck);

    // A reset while the playhead sits on a hit other than step 1.
    Rig r2(5, 8, 0);                          // x.x.xx.x
    r2.settle();
    r2.clocks(2);
    o = r2.idle();
    check("(on step 3, a hit)", o.position == 2 && o.gate);
    o = r2.reset();
    check("a reset from a later hit step reports the gap", o.gate_gap);
}

static void wrapTests()
{
    std::printf("wrap at the effective length\n");
    Rig r(8, 8, 0);
    r.settle();
    r.clocks(7);
    checkEq("(on step 8)", r.seq.position, 7);
    Out o = r.clock();
    checkEq("the eighth clock wraps to step 1", o.position, 0);
    check("and strikes it", o.struck);

    Rig r5(5, 5, 0);
    r5.settle();
    int seen[6] = { 0, 0, 0, 0, 0, 0 };
    for (int i = 0; i < 6; i++) seen[i] = r5.clock().position;
    check("5 steps cycle 1 2 3 4 5 1 (0-based 1 2 3 4 0 1)",
          seen[0] == 1 && seen[1] == 2 && seen[2] == 3 && seen[3] == 4 && seen[4] == 0 && seen[5] == 1);

    Rig r1(1, 1, 0);
    r1.settle();
    o = r1.clock();
    checkEq("1 step always plays step 1", o.position, 0);
    check("and strikes it on every clock", o.struck);
    o = r1.clock();
    check("(again)", o.position == 0 && o.struck);
}

static void shrinkTests()
{
    std::printf("STEPS shrinks below the playhead\n");
    Rig r(16, 16, 0);
    r.settle();
    r.clocks(10);
    Out o = r.idle();
    check("(on step 11, GATE high)", o.position == 10 && o.gate);

    r.steps = 4;
    r.hits = 4;
    o = r.idle();
    checkEq("the playhead stays where it was", o.position, 10);
    check("GATE reads a cleared slot", !o.gate);
    check("nothing strikes", !o.struck);
    check("no gap is reported (that is a reset's)", !o.gate_gap);

    o = r.clock();
    checkEq("the next clock wraps to step 1", o.position, 0);
    check("and strikes it", o.struck);
    check("GATE follows step 1", o.gate);

    // Growing STEPS back does not move the playhead either.
    r.steps = 16;
    r.hits = 16;
    o = r.clock();
    checkEq("after growing back, the clock advances normally", o.position, 1);
}

static void rotationTests()
{
    std::printf("ROT changed between clocks\n");
    // E(3,8) rot 0 is x..x..x.: step 3 (index 2) is a rest.
    // E(3,8) rot 1 is ..x..x.x: step 3 is a hit.
    Rig r(3, 8, 0);
    r.settle();
    Out o = r.clock();
    check("(on step 2, a rest in both rotations)", o.position == 1 && !o.gate);

    r.rot = 1;
    o = r.idle();
    check("the new pattern shows at once on a non-edge sample (still a rest here)", !o.gate && !o.struck);
    checkEq("the playhead did not move", o.position, 1);

    o = r.clock();
    checkEq("the next edge lands on step 3", o.position, 2);
    check("which is a hit under rot 1", o.struck && o.gate);

    // The same walk without the change lands on a rest.
    Rig r0(3, 8, 0);
    r0.settle();
    r0.clock();
    o = r0.clock();
    check("under rot 0 step 3 is a rest", o.position == 2 && !o.struck && !o.gate);

    // A ROT change while the playhead sits on a hit that becomes a rest drops
    // GATE without a clock (the source reads the pattern every sample).
    Rig r2(3, 8, 0);
    o = r2.idle();
    check("(step 1 is a hit under rot 0)", o.gate);
    r2.rot = 1;
    o = r2.idle();
    check("step 1 becomes a rest under rot 1 and GATE drops", !o.gate);
    check("without a strike", !o.struck);
}

static void consecutiveHitTests()
{
    std::printf("GATE across consecutive hit steps\n");
    // E(5,8) is x.x.xx.x: steps 5 and 6 (indices 4 and 5) are consecutive hits.
    Rig r(5, 8, 0);
    r.settle();
    r.clocks(3);
    Out o = r.clock();
    check("landing on step 5 strikes", o.position == 4 && o.struck && o.gate);

    bool held = true;
    for (int i = 0; i < 10; i++)
    {
        o = r.idle();
        if (!o.gate || o.struck) held = false;
    }
    check("GATE holds through step 5 with no strike", held);

    o = r.clock();
    check("landing on step 6 strikes again", o.position == 5 && o.struck);
    check("GATE stays high (no re-edge from the engine)", o.gate);
    check("and no gap is reported: the gap is a reset's", !o.gate_gap);

    held = true;
    for (int i = 0; i < 10; i++)
    {
        o = r.idle();
        if (!o.gate || o.struck) held = false;
    }
    check("GATE holds through step 6 with no strike", held);

    o = r.clock();
    check("step 7 is a rest: GATE drops, nothing strikes", o.position == 6 && !o.gate && !o.struck);
}

static void struckInvariantTests()
{
    std::printf("struck only on edge and reset samples\n");
    Rig r(16, 16, 0);
    int strikes = 0;
    for (int i = 0; i < 2000; i++) if (r.idle().struck) strikes++;
    checkEq("2000 idle samples after load strike nothing", strikes, 0);

    strikes = 0;
    int edges = 0;
    for (int i = 0; i < 40; i++)
    {
        Out o = r.clock();
        edges++;
        if (o.struck) strikes++;
        for (int j = 0; j < 99; j++) if (r.idle().struck) strikes++;
    }
    checkEq("40 clocks 100 samples apart strike exactly 40 times (all hits)", strikes, edges);

    r.reset();
    strikes = 0;
    for (int i = 0; i < 500; i++) if (r.idle().struck) strikes++;
    checkEq("the 500 samples after a reset strike nothing", strikes, 0);

    // Out.position always mirrors the engine's.
    bool mirrors = true;
    for (int i = 0; i < 100; i++)
    {
        Out o = (i % 7 == 0) ? r.clock() : r.idle();
        if (o.position != r.seq.position) mirrors = false;
    }
    check("Out.position mirrors Sequencer::position", mirrors);
}

static void windowLengthTests()
{
    std::printf("the window length is the caller's\n");

    // 45 samples, the standard's 1 ms at 44.1 kHz.
    Rig r1(16, 16, 0, 45);
    r1.settle();
    r1.clocks(3);
    r1.reset();
    r1.idleFor(43);
    Out o = r1.clock();
    checkEq("with a 45-sample window, a clock at +44 is dropped", o.position, 0);

    Rig r2(16, 16, 0, 45);
    r2.settle();
    r2.clocks(3);
    r2.reset();
    r2.idleFor(44);
    o = r2.clock();
    checkEq("with a 45-sample window, a clock at +45 is accepted", o.position, 1);

    // The minimum window (ResetClockWindow::MIN_SAMPLES): only the reset sample.
    Rig r3(16, 16, 0, 1);
    r3.settle();
    o = r3.both();
    checkEq("with a 1-sample window, a same-sample clock is still swallowed", o.position, 0);
    o = r3.clock();
    checkEq("and a clock at +1 is accepted", o.position, 1);

    // The value passed on the reset sample is the one used; later calls can
    // pass anything (a rate change mid-window is the caller's business).
    Rig r4(16, 16, 0, 96);
    r4.settle();
    r4.reset();
    r4.window = 48;
    r4.idleFor(60);
    o = r4.clock();
    checkEq("a 96-sample window armed at the reset stays 96 long", o.position, 0);
    r4.idleFor(34);
    o = r4.clock();
    checkEq("(and closes at +96)", o.position, 1);
}

int main()
{
    loadTests();
    resetTests();
    windowTests();
    sameSampleTests();
    clockThenResetTests();
    gateGapTests();
    wrapTests();
    shrinkTests();
    rotationTests();
    consecutiveHitTests();
    struckInvariantTests();
    windowLengthTests();
    return finish("engine_test");
}
