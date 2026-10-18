// VX Drum Sequencer engine: the playhead, the clock follower, resolveStep,
// the ratchets, Sequencer::process end to end for a lone module, and the
// reset rule — the VCV standard, one case per row of rack-port-design.md §4.3,
// plus the exact integer contract of the 1 ms window that the float-modelled
// seq3_parity_test.cpp deliberately steers around.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o engine_test engine_test.cpp && ./engine_test

#include "VXDrumSequencerEngine.hpp"
#include "check.hpp"

using namespace vx_drum_sequencer;

static const float SR = 48000.f;
static const int WINDOW = 48;    // vgLib_v2::ResetClockWindow::samplesFor(48000), pinned below

// The last sample's output voltages, written by every helper.
static float OUT[LANES];

// Drive the engine `n` samples with no clock edge.
static void idle(Sequencer& s, const PlaySource& src, int n, int64_t pulse = 48)
{
    for (int i = 0; i < n; i++) s.process(false, src, SR, pulse, OUT);
}

static void clock(Sequencer& s, const PlaySource& src, int64_t pulse = 48)
{
    s.process(true, src, SR, pulse, OUT);
}

// An RST edge (or the RWD button) in the module's order: reset() before
// process(), on one sample, with or without a clock edge on that sample.
static void reset(Sequencer& s, const PlaySource& src, bool clock_edge = false, int64_t pulse = 48)
{
    s.reset(src, WINDOW);
    s.process(clock_edge, src, SR, pulse, OUT);
}

// Load: the module's first process() call is an implicit reset.
static void load(Sequencer& s)
{
    s.rewind(WINDOW);
}

static void playheadTests()
{
    std::printf("Playhead\n");
    Playhead p;
    checkEq("step 1 is current from the start", p.position, 0);
    checkEq("the first advance lands on step 2", p.advance(16), 1);
    checkEq("the second on step 3", p.advance(16), 2);
    p.position = 15;
    checkEq("wraps at length", p.advance(16), 0);
    p.position = 7;
    checkEq("a length that shrank below the position wraps", p.advance(4), 0);
    p.position = 2;
    checkEq("length 1 always plays step 0", p.advance(1), 0);
    p.reset();
    checkEq("reset returns to step 1", p.position, 0);
    p.rewind();
    checkEq("rewind (the hand-off) goes to -1", p.position, -1);
    checkEq("and the next advance lands on step 1", p.advance(16), 0);
    p.rewind();
    checkEq("length 0 is clamped to 1", p.advance(0), 0);
}

static void clockFollowerTests()
{
    std::printf("ClockFollower\n");
    ClockFollower c;
    for (int i = 0; i < 100; i++) c.tick();
    c.edge(SR);
    check("the first edge anchors but does not measure", c.period_samples == 0.f);
    for (int i = 0; i < 100; i++) c.tick();
    c.edge(SR);
    check("the second edge measures", c.period_samples == 100.f);

    c.rearm();
    for (int i = 0; i < 37; i++) c.tick();
    c.edge(SR);
    check("the partial interval after a rearm is not taken as the period", c.period_samples == 100.f);
    for (int i = 0; i < 200; i++) c.tick();
    c.edge(SR);
    check("the edge after the anchor measures again", c.period_samples == 200.f);

    // Debounce window: a 1-sample interval is ignored, and so is > 4 s.
    c.tick();
    c.edge(SR);
    check("a 1-sample interval is ignored", c.period_samples == 200.f);
    for (int i = 0; i < (int)(SR * 5); i++) c.tick();
    c.edge(SR);
    check("an interval over 4 s is ignored", c.period_samples == 200.f);

    c.rescale(48000.f, 96000.f);
    check("rescale doubles the period", c.period_samples == 400.f);
    check("rescale re-anchors", !c.anchor_valid && c.counter == 0);
    c.rescale(0.f, 96000.f);
    check("rescale with an unknown old rate is a no-op", c.period_samples == 400.f);
}

static void resolveStepTests()
{
    std::printf("resolveStep\n");
    Memory m;
    m.at(0, 3).on = true;            // BD on step 4
    m.at(1, 3).on = true;            // SD on step 4
    m.at(ACCENT_LANE, 3).on = true;  // accented
    m.at(1, 3).ratchet = 2;          // SD x3 on step 4
    Rng rng;

    StepFire f = resolveStep(m, 0, 3, rng);
    checkEq("BD, SD and the accent strike", f.struck, (1u << 0) | (1u << 1) | (1u << ACCENT_LANE));
    check("accented", f.accented);
    checkEq("SD arms two extra hits", f.extra[1], 2);
    checkEq("BD arms none", f.extra[0], 0);

    f = resolveStep(m, (uint8_t)(1u << 1), 3, rng);
    checkEq("a muted SD neither strikes", f.struck & (1u << 1), 0);
    checkEq("nor arms", f.extra[1], 0);
    checkEq("BD and accent still strike", f.struck, (1u << 0) | (1u << ACCENT_LANE));

    f = resolveStep(m, (uint8_t)(1u << ACCENT_LANE), 3, rng);
    check("a muted accent lane removes the accent", !f.accented);
    checkEq("and its lamp bit", f.struck, (1u << 0) | (1u << 1));

    f = resolveStep(m, 0, 2, rng);
    checkEq("an empty step strikes nothing", f.struck, 0);
    f = resolveStep(m, 0, 16, rng);
    checkEq("a position past the grid strikes nothing", f.struck, 0);
    f = resolveStep(m, 0, -1, rng);
    checkEq("a negative position strikes nothing", f.struck, 0);
}

static void chanceTests()
{
    std::printf("chance\n");
    Rng rng;
    rng.seed(12345);

    Memory m;
    m.at(0, 0).on = true;  m.at(0, 0).chance = 100;
    m.at(1, 0).on = true;  m.at(1, 0).chance = 50;
    m.at(2, 0).on = true;  m.at(2, 0).chance = 0;
    m.at(3, 0).on = true;  m.at(3, 0).chance = 10;
    m.at(3, 0).ratchet = 3;
    m.at(ACCENT_LANE, 0).on = true;  m.at(ACCENT_LANE, 0).chance = 50;

    const int N = 4000;
    int bd = 0, sd = 0, cp = 0, perc = 0, acc = 0, perc_armed = 0;
    for (int i = 0; i < N; i++)
    {
        const StepFire f = resolveStep(m, 0, 0, rng);
        if (f.struck & (1u << 0)) bd++;
        if (f.struck & (1u << 1)) sd++;
        if (f.struck & (1u << 2)) cp++;
        if (f.struck & (1u << 3)) { perc++; if (f.extra[3] == 3) perc_armed++; }
        if (f.struck & (1u << ACCENT_LANE)) acc++;
        if (!(f.struck & (1u << 3)) && f.extra[3] != 0) { check("a pad that fails its roll arms no ratchets", false); break; }
    }
    checkEq("100 % always plays", bd, N);
    checkEq("0 % never plays", cp, 0);
    check("50 % plays about half the time", sd > N * 0.45 && sd < N * 0.55);
    check("10 % plays about a tenth of the time", perc > N * 0.07 && perc < N * 0.13);
    checkEq("a pad that passes its roll arms its ratchets", perc_armed, perc);
    check("the accent lane rolls too", acc > N * 0.45 && acc < N * 0.55);

    // Determinism: the same seed gives the same sequence.
    Rng a, b;
    a.seed(7); b.seed(7);
    bool same = true;
    for (int i = 0; i < 100; i++) if (a.percent() != b.percent()) same = false;
    check("a seeded Rng is deterministic", same);
    a.seed(0);
    check("seed 0 does not stick the generator", a.next() != 0 && a.next() != a.next());

    // A step through the Sequencer: a failed roll does not flash the lane.
    Memory z;
    z.at(0, 0).on = true;  z.at(0, 0).chance = 0;
    z.length = 1;
    PlaySource src; src.memory = &z; src.mute = 0;
    Sequencer s;
    load(s);
    idle(s, src, 100);
    clock(s, src);
    check("a step whose only pad failed still counts as a fire", s.fired_this_sample);
    checkEq("but flashes nothing", s.report.fired_mask, 0);
    check("and outputs nothing", OUT[0] == 0.f);
}

static void ratchetTests()
{
    std::printf("Ratchets\n");
    StepFire f;
    f.struck = (1u << 4) | (1u << ACCENT_LANE);
    f.extra[4] = 1;      // CH x2
    f.accented = true;

    Ratchets r;
    r.arm(f, 0.f);
    check("no period, no re-strikes", !r.any());

    r.arm(f, 100.f);
    check("armed with a period", r.any());
    int fired_at = -1;
    uint32_t bits = 0;
    for (int i = 1; i <= 100; i++)
    {
        bits = r.tick();
        if (bits) { fired_at = i; break; }
    }
    checkEq("x2 re-strikes at half the period", fired_at, 50);
    checkEq("with the ACC bit because the step was accented", bits, (1u << 4) | (1u << ACCENT_LANE));
    check("then nothing is left", !r.any());

    f.extra[4] = 3;      // x4
    f.accented = false;
    r.arm(f, 100.f);
    int hits = 0;
    for (int i = 0; i < 100; i++) if (r.tick()) hits++;
    checkEq("x4 gives three re-strikes inside one period", hits, 3);

    r.arm(f, 100.f);
    r.cancel();
    check("cancel clears them", !r.any());
    checkEq("and ticks fire nothing", r.tick(), 0);

    // The 2-sample floor: a tiny period still spaces re-strikes.
    r.arm(f, 5.f);
    check("a period above 4 samples arms", r.any());
    checkEq("interval floor is 2", r.interval[4], 2);
}

// Count the rising edges on lane `lane` over `n` idle samples.
static int countRises(Sequencer& s, const PlaySource& src, int lane, int n, int64_t pulse)
{
    int rises = 0;
    bool prev = OUT[lane] == 10.f;
    for (int i = 0; i < n; i++)
    {
        s.process(false, src, SR, pulse, OUT);
        if (OUT[lane] == 10.f && !prev) rises++;
        prev = OUT[lane] == 10.f;
    }
    return rises;
}

static void sequencerTests()
{
    std::printf("Sequencer\n");
    Memory m;
    for (int s = 0; s < STEPS; s += 4) m.at(0, s).on = true;   // BD on 1 5 9 13
    m.at(ACCENT_LANE, 0).on = true;                             // step 1 accented
    m.length = 4;
    PlaySource src;
    src.memory = &m;
    src.mute = 0;

    Sequencer s;
    load(s);

    // Load: step 1 is current, nothing plays until an edge or a reset.
    idle(s, src, 100);
    checkEq("no fire before a clock", s.report.fired_serial, 0);
    checkEq("step 1 is current after load", s.report.position, 0);
    check("nothing is high", OUT[0] == 0.f && OUT[ACCENT_LANE] == 0.f);

    // A reset plays step 1 (position 0) on that very sample.
    reset(s, src);
    check("fired on the reset sample", s.fired_this_sample);
    checkEq("position 0", s.report.position, 0);
    checkEq("BD and the accent fired", s.report.fired_mask, (1u << 0) | (1u << ACCENT_LANE));
    check("BD output is high on the fire sample", OUT[0] == 10.f);
    check("ACC output is high on the fire sample", OUT[ACCENT_LANE] == 10.f);
    check("SD is low", OUT[1] == 0.f);

    // Pulse length: 48 samples high, then low.
    for (int i = 0; i < 47; i++) { s.process(false, src, SR, 48, OUT); check("pulse holds", OUT[0] == 10.f); }
    s.process(false, src, SR, 48, OUT);
    check("pulse ends after pulse_len samples", OUT[0] == 0.f);

    // Four edges at a 100-sample period: steps 2 3 4, then a wrap to step 1.
    idle(s, src, 51);
    clock(s, src); checkEq("the first clock after the reset plays step 2", s.report.position, 1);
    checkEq("nothing on step 2", s.report.fired_mask, 0);
    check("a fire with nothing lit still fired", s.fired_this_sample);
    idle(s, src, 99);
    clock(s, src); checkEq("step 3", s.report.position, 2);
    idle(s, src, 99);
    clock(s, src); checkEq("step 4", s.report.position, 3);
    idle(s, src, 99);
    clock(s, src); checkEq("wraps to step 1 at length 4", s.report.position, 0);
    checkEq("five fires so far: the reset's and four clocks'", s.report.fired_serial, 5);
    check("the period was measured", s.clock.period_samples == 100.f);
    check("step 1 lit BD again on the wrap", OUT[0] == 10.f);

    // Ratchets run across the step; an implicit reset cancels them.
    Memory r;
    r.at(4, 0).on = true;
    r.at(4, 0).ratchet = 3;   // CH x4 on step 1
    r.length = 1;
    PlaySource rs; rs.memory = &r; rs.mute = 0;
    Sequencer t;
    load(t);
    idle(t, rs, 100);
    clock(t, rs); idle(t, rs, 99); clock(t, rs);    // period 100, step 1 fired with ratchets armed
    check("ratchets armed on the fire", t.ratchets.any());
    checkEq("three re-strikes inside the period", countRises(t, rs, 4, 99, 2), 3);
    clock(t, rs);
    check("armed again", t.ratchets.any());
    t.rewind(WINDOW);
    check("rewind cancels ratchets", !t.ratchets.any());
    checkEq("and clears the lamp bits", t.report.fired_mask, 0);
    check("but keeps the period", t.clock.period_samples == 100.f);
    check("and drops the anchor", !t.clock.anchor_valid);

    // A null memory source: the edge measures the clock but plays nothing.
    Sequencer u;
    load(u);
    idle(u, PlaySource(), 100);
    PlaySource none;
    clock(u, none);
    checkEq("no memory, no fire", u.report.fired_serial, 0);
    check("but the clock anchored", u.clock.anchor_valid);
}

// The VCV standard, one case per row of rack-port-design.md §4.3. The engine
// takes edge BOOLEANS: the Schmitts are the module's, so "reset while the
// clock line is high" is simply a reset with no clock edge, which the module
// guarantees by never reset()ing its Schmitt.
static void resetTests()
{
    std::printf("reset: the VCV standard\n");
    Memory m;
    for (int st = 0; st < 8; st++) m.at(0, st).on = true;   // BD on every step
    m.length = 8;
    PlaySource src; src.memory = &m; src.mute = 0;

    // The window length, from the shared helper, at the rates that matter:
    // exactly 1 ms where 1 ms is a whole number of samples, rounded up
    // where it is not (the helper's documented contract, and the span of
    // SEQ3's resetPulse read through PulseGenerator::process).
    checkEq("ResetClockWindow::samplesFor(48000) is 48 (1 ms exactly)", vgLib_v2::ResetClockWindow::samplesFor(48000.f), 48);
    checkEq("ResetClockWindow::samplesFor(44100) is 45 (44.1 rounded up)", vgLib_v2::ResetClockWindow::samplesFor(44100.f), 45);
    checkEq("ResetClockWindow::samplesFor(96000) is 96", vgLib_v2::ResetClockWindow::samplesFor(96000.f), 96);
    checkEq("ResetClockWindow::samplesFor(192000) is 192", vgLib_v2::ResetClockWindow::samplesFor(192000.f), 192);
    checkEq("ResetClockWindow::samplesFor is never shorter than one sample", vgLib_v2::ResetClockWindow::samplesFor(1.f), 1);

    // Row: patch load. Step 1 current, nothing fires, the window is armed.
    {
        Sequencer s;
        load(s);
        checkEq("load: step 1 current", s.report.position, 0);
        check("load: window armed", s.window.open());
        idle(s, src, 100);
        checkEq("load: nothing fires", s.report.fired_serial, 0);
        check("load: the window closes on its own", !s.window.open());
        clock(s, src);
        check("the first clock after load fires", s.fired_this_sample);
        checkEq("and plays step 2", s.report.position, 1);
    }

    // Row: first clock after load, when the clock arrives at once. Load is an
    // implicit reset, so a clock inside its window is swallowed (Impromptu).
    {
        Sequencer s;
        load(s);
        clock(s, src);                                     // the load sample
        check("a clock on the load sample is swallowed", !s.fired_this_sample);
        checkEq("position stays on step 1", s.report.position, 0);
    }

    // Row: a reset after load with no clock. Step 1 plays at the reset.
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        reset(s, src);
        check("a reset fires step 1 on that sample", s.fired_this_sample);
        checkEq("on position 0", s.report.position, 0);
        checkEq("BD flashed", s.report.fired_mask, 1u << 0);
        checkEq("the serial counted it", s.report.fired_serial, 1);
        check("BD is high on the reset sample", OUT[0] == 10.f);
        check("the window is armed", s.window.open());
    }

    // The integer contract of the window at 48 kHz: a clock 47 samples after
    // the reset is swallowed, 48 samples after is accepted and plays step 2.
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        reset(s, src);                                     // sample R
        idle(s, src, 46);                                  // R+1 .. R+46
        clock(s, src);                                     // R+47: the window's last sample
        check("a clock 47 samples after a reset is swallowed", !s.fired_this_sample);
        checkEq("and the position stays on step 1", s.report.position, 0);
        clock(s, src);                                     // R+48
        check("a clock 48 samples after a reset is accepted", s.fired_this_sample);
        checkEq("and plays step 2", s.report.position, 1);
        checkEq("two fires: the reset's and that clock's", s.report.fired_serial, 2);
    }

    // The same contract at 44.1 kHz: 44 swallowed, 45 accepted.
    {
        Sequencer s;
        const int w = vgLib_v2::ResetClockWindow::samplesFor(44100.f);
        s.rewind(w);
        idle(s, src, 100);
        s.reset(src, w); s.process(false, src, 44100.f, 44, OUT);   // R
        idle(s, src, w - 2);                               // R+1 .. R+w-2
        s.process(true, src, 44100.f, 44, OUT);            // R+w-1 = R+44
        check("44.1 kHz: a clock 44 samples after a reset is swallowed", !s.fired_this_sample);
        s.process(true, src, 44100.f, 44, OUT);            // R+45
        check("44.1 kHz: a clock 45 samples after a reset plays step 2", s.fired_this_sample && s.report.position == 1);
    }

    // Row: reset and clock edge on the same sample (Clocked restarting its
    // cycle). Step 1 plays once, at the reset; the clock is swallowed.
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        clock(s, src); idle(s, src, 99);                   // step 2
        clock(s, src); idle(s, src, 99);                   // step 3
        const uint32_t before = s.report.fired_serial;
        reset(s, src, true);
        check("same-sample reset + clock fires", s.fired_this_sample);
        checkEq("once", s.report.fired_serial, before + 1);
        checkEq("on step 1", s.report.position, 0);
        idle(s, src, 99);
        clock(s, src);
        checkEq("the next clock is step 2", s.report.position, 1);
    }

    // Row: reset direct, clock one sample later (the clock through a
    // utility module). Same result: the late clock is inside the window.
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        clock(s, src); idle(s, src, 99);                   // step 2
        const uint32_t before = s.report.fired_serial;
        reset(s, src);
        clock(s, src);
        checkEq("reset then clock a sample later: one fire", s.report.fired_serial, before + 1);
        checkEq("on step 1", s.report.position, 0);
        check("the late clock did not fire", !s.fired_this_sample);
    }

    // Row: clock direct, reset one sample later (the reset through a
    // utility). Step N+1 fires, then step 1: two hits. The standard's known
    // limitation, documented, not "fixed"; SEQ3 does the same.
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        clock(s, src); idle(s, src, 99);                   // step 2
        clock(s, src); idle(s, src, 99);                   // step 3
        const uint32_t before = s.report.fired_serial;
        clock(s, src);
        checkEq("the clock plays step 4", s.report.position, 3);
        reset(s, src);
        checkEq("the reset a sample later plays step 1", s.report.position, 0);
        checkEq("two fires", s.report.fired_serial, before + 2);
    }

    // Row: reset while the clock line is high (no edge from the master). Step
    // 1 at the reset; the next rising edge is step 2.
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        clock(s, src); idle(s, src, 99);                   // step 2
        clock(s, src); idle(s, src, 10);                   // step 3, the line still high
        reset(s, src);                                     // no edge: the Schmitt saw none
        checkEq("reset in the clock's high half plays step 1", s.report.position, 0);
        idle(s, src, 89);
        clock(s, src);                                     // the master's next rise
        checkEq("the next rising edge is step 2", s.report.position, 1);
    }

    // Row: mid-period reset, free-running clock. Step 1 plays at the reset
    // and gets the remainder of the period; the ratchets in flight from the
    // interrupted step are cancelled; step 1's arm from the surviving period;
    // the next edge is step 2 and anchors without measuring. 2-sample pulses
    // throughout so every strike is its own rise.
    {
        Memory r;
        r.at(4, 0).on = true;
        r.at(4, 0).ratchet = 3;                            // CH x4 on step 1
        r.at(5, 1).on = true;
        r.at(5, 1).ratchet = 1;                            // OH x2 on step 2
        r.length = 2;
        PlaySource rs; rs.memory = &r; rs.mute = 0;

        Sequencer s;
        load(s);
        idle(s, rs, 100, 2);
        clock(s, rs, 2); idle(s, rs, 99, 2);               // step 2, anchors
        clock(s, rs, 2); idle(s, rs, 99, 2);               // step 1, measures 100
        check("period 100 measured", s.clock.period_samples == 100.f);
        clock(s, rs, 2);                                   // step 2 with OH x2 armed
        check("step 2's ratchets are in flight", s.ratchets.any());
        idle(s, rs, 29, 2);                                // mid-period: 30 samples in, OH's re-strike still 20 away
        reset(s, rs, false, 2);
        checkEq("the mid-period reset plays step 1", s.report.position, 0);
        check("CH rises on the reset sample", OUT[4] == 10.f);
        check("its ratchets arm from the surviving period", s.ratchets.left[4] == 3);
        checkEq("the interrupted step's ratchets were cancelled", s.ratchets.left[5], 0);
        check("the period survived", s.clock.period_samples == 100.f);
        int ch_rises = 0, oh_rises = 0;
        bool ch_prev = OUT[4] == 10.f, oh_prev = OUT[5] == 10.f;
        for (int i = 0; i < 99; i++)
        {
            s.process(false, rs, SR, 2, OUT);
            if (OUT[4] == 10.f && !ch_prev) ch_rises++;
            if (OUT[5] == 10.f && !oh_prev) oh_rises++;
            ch_prev = OUT[4] == 10.f;
            oh_prev = OUT[5] == 10.f;
        }
        checkEq("three x4 re-strikes follow inside one period", ch_rises, 3);
        checkEq("and no re-strike from the interrupted step", oh_rises, 0);
        clock(s, rs, 2);                                   // 100 samples after the reset
        checkEq("the next edge is step 2", s.report.position, 1);
        check("it anchored without measuring the partial interval", s.clock.period_samples == 100.f && s.clock.anchor_valid);
    }

    // A reset before any period is known arms no ratchets (the "dropped
    // rather than guessed" rule holds for the reset's fire too).
    {
        Memory r;
        r.at(4, 0).on = true;
        r.at(4, 0).ratchet = 3;
        r.length = 2;
        PlaySource rs; rs.memory = &r; rs.mute = 0;
        Sequencer s;
        load(s);
        idle(s, rs, 100);
        reset(s, rs);
        check("a reset with no period known plays step 1", s.fired_this_sample);
        check("but arms no ratchets", !s.ratchets.any());
    }

    // A reset inside an open window re-arms it to the full length (SEQ3's
    // PulseGenerator::trigger keeps the longer pulse; so does arm()).
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        reset(s, src);                                     // R
        idle(s, src, 29);                                  // R+1 .. R+29
        reset(s, src);                                     // R+30: a second reset
        idle(s, src, 46);                                  // R+31 .. R+76
        clock(s, src);                                     // R+77 = 47 after the second reset
        check("a clock 47 samples after the SECOND reset is swallowed", !s.fired_this_sample);
        clock(s, src);                                     // R+78 = 48 after the second reset
        check("48 samples after it is accepted", s.fired_this_sample && s.report.position == 1);
    }

    // Row: clock faster than the window (above 1 kHz at 48 kHz). Edges inside
    // the window are lost; nothing else breaks.
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        reset(s, src);                                     // R
        int fired = 0;
        for (int i = 1; i <= 60; i++)                      // an edge every 10 samples: R+10 .. R+60
        {
            if (i % 10 == 0) { clock(s, src); if (s.fired_this_sample) fired++; }
            else idle(s, src, 1);
        }
        checkEq("edges at 10, 20, 30, 40 samples are lost; 50 and 60 play", fired, 2);
        checkEq("so the position is step 3", s.report.position, 2);
    }

    // A reset with no memory (nothing to play): step 1 current, window
    // armed, nothing fires.
    {
        Sequencer s;
        load(s);
        idle(s, PlaySource(), 100);
        s.reset(PlaySource(), WINDOW);
        s.process(false, PlaySource(), SR, 48, OUT);
        check("a reset with no memory fires nothing", !s.fired_this_sample);
        checkEq("but step 1 is current", s.report.position, 0);
        check("and the window is armed", s.window.open());
    }

    // The hand-off arms no window: the next clock plays the new member's
    // step 1 at once (chain_test.cpp has the whole chain).
    {
        Sequencer s;
        load(s);
        idle(s, src, 100);
        clock(s, src); idle(s, src, 99);                   // step 2
        s.handOff();
        checkEq("handOff: position -1", s.report.position, -1);
        check("handOff: no window", !s.window.open());
        clock(s, src);
        checkEq("the next clock plays step 1", s.report.position, 0);
        check("and fired", s.fired_this_sample);
    }
}

int main()
{
    playheadTests();
    clockFollowerTests();
    resolveStepTests();
    chanceTests();
    ratchetTests();
    sequencerTests();
    resetTests();
    return finish("engine_test");
}
