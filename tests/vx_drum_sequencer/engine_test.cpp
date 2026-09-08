// VX Drum Sequencer engine: the playhead, the clock follower, resolveStep,
// the ratchets, and Sequencer::process end to end for a lone module.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o engine_test engine_test.cpp && ./engine_test

#include "VXDrumSequencerEngine.hpp"
#include "check.hpp"

using namespace vx_drum_sequencer;

static const float SR = 48000.f;

// Drive the engine `n` samples with no clock edge.
static void idle(Sequencer& s, const PlaySource& src, int n, int64_t pulse = 48)
{
    float out[LANES];
    for (int i = 0; i < n; i++) s.process(false, src, SR, pulse, out);
}

static void clock(Sequencer& s, const PlaySource& src, int64_t pulse = 48)
{
    float out[LANES];
    s.process(true, src, SR, pulse, out);
}

static void playheadTests()
{
    std::printf("Playhead\n");
    Playhead p;
    checkEq("starts at -1", p.position, -1);
    checkEq("first advance lands on step 0", p.advance(16), 0);
    checkEq("second advance", p.advance(16), 1);
    p.position = 15;
    checkEq("wraps at length", p.advance(16), 0);
    p.position = 7;
    checkEq("a length that shrank below the position wraps", p.advance(4), 0);
    p.position = 2;
    checkEq("length 1 always plays step 0", p.advance(1), 0);
    p.rewind();
    checkEq("rewind returns to -1", p.position, -1);
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
    float out[LANES];
    s.process(true, src, SR, 48, out);
    check("a step whose only pad failed still counts as a fire", s.fired_this_sample);
    checkEq("but flashes nothing", s.report.fired_mask, 0);
    check("and outputs nothing", out[0] == 0.f);
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
    float out[LANES];

    // Load: nothing plays until the first edge.
    idle(s, src, 100);
    checkEq("no fire before a clock", s.report.fired_serial, 0);
    checkEq("position stays -1", s.report.position, -1);

    // The first edge plays step 1 (position 0) on that very sample.
    s.process(true, src, SR, 48, out);
    check("fired on the edge sample", s.fired_this_sample);
    checkEq("position 0", s.report.position, 0);
    checkEq("BD and the accent fired", s.report.fired_mask, (1u << 0) | (1u << ACCENT_LANE));
    check("BD output is high on the fire sample", out[0] == 10.f);
    check("ACC output is high on the fire sample", out[ACCENT_LANE] == 10.f);
    check("SD is low", out[1] == 0.f);

    // Pulse length: 48 samples high, then low.
    for (int i = 0; i < 47; i++) { s.process(false, src, SR, 48, out); check("pulse holds", out[0] == 10.f); }
    s.process(false, src, SR, 48, out);
    check("pulse ends after pulse_len samples", out[0] == 0.f);

    // Three more edges: steps 2 3 4, then a wrap.
    idle(s, src, 52);
    clock(s, src); checkEq("step 2", s.report.position, 1);
    checkEq("nothing on step 2", s.report.fired_mask, 0);
    idle(s, src, 99);
    clock(s, src); checkEq("step 3", s.report.position, 2);
    idle(s, src, 99);
    clock(s, src); checkEq("step 4", s.report.position, 3);
    idle(s, src, 99);
    clock(s, src); checkEq("wraps to step 1 at length 4", s.report.position, 0);
    checkEq("five fires so far", s.report.fired_serial, 5);
    check("the period was measured", s.clock.period_samples == 100.f);

    // Reset then a same-sample clock plays step 1 on that sample (§6.b).
    idle(s, src, 10);
    s.rewind();
    checkEq("rewind: position -1", s.report.position, -1);
    checkEq("rewind: lamp bits cleared", s.report.fired_mask, 0);
    check("rewind keeps the period", s.clock.period_samples == 100.f);
    check("rewind drops the anchor", !s.clock.anchor_valid);
    clock(s, src);
    checkEq("the clock with the reset plays step 1", s.report.position, 0);
    check("and it fired", s.fired_this_sample);

    // Ratchets run across the step; a rewind cancels them.
    Memory r;
    r.at(4, 0).on = true;
    r.at(4, 0).ratchet = 3;   // CH x4 on step 1
    r.length = 1;
    PlaySource rs; rs.memory = &r; rs.mute = 0;
    Sequencer t;
    clock(t, rs); idle(t, rs, 99); clock(t, rs);    // period 100, step 1 fired with ratchets armed
    check("ratchets armed on the fire", t.ratchets.any());
    int restrikes = 0;
    for (int i = 0; i < 99; i++)
    {
        t.process(false, rs, SR, 2, out);
        // A re-strike shows as a fresh rise on CH; count rises.
        static bool prev = true;
        if (out[4] == 10.f && !prev) restrikes++;
        prev = out[4] == 10.f;
    }
    checkEq("three re-strikes inside the period", restrikes, 3);
    clock(t, rs);
    t.rewind();
    check("rewind cancels ratchets", !t.ratchets.any());

    // A null memory source: the edge measures the clock but plays nothing.
    Sequencer u;
    PlaySource none;
    clock(u, none);
    checkEq("no memory, no fire", u.report.fired_serial, 0);
    check("but the clock anchored", u.clock.anchor_valid);
}

int main()
{
    playheadTests();
    clockFollowerTests();
    resolveStepTests();
    chanceTests();
    ratchetTests();
    sequencerTests();
    return finish("engine_test");
}
