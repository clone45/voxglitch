// Euclid engine against Fundamental SEQ3: the same reset/clock voltages into
// a float model of SEQ3's reset logic and into euclid::Sequencer, position
// compared after every sample. Plus the exact-boundary cases that pin OUR
// integer contract, which SEQ3's float arithmetic cannot.
//
//   g++ -std=c++11 -O2 -I ../../src -o seq3_parity_test seq3_parity_test.cpp && ./seq3_parity_test
//
// SEQ3 (https://github.com/VCVRack/Fundamental/blob/v2/src/SEQ3.cpp), per
// sample, external clock, running:
//
//   resetTriggered = resetTrigger.process(resetVolts, 0.1f, 2.f)   // dsp::SchmittTrigger
//   if (resetTriggered) { resetPulse.trigger(1e-3f); index = 0; }
//   resetGate = resetPulse.process(sampleTime)                    // dsp::PulseGenerator, float
//   clockTriggered = clockTrigger.process(clockVolts, 0.1f, 2.f)
//   clock = clockTriggered && !resetGate
//   if (clock) { index++; if (index >= numSteps) index = 0; }
//
// (That is SEQ3's own order: the reset Schmitt, then resetGate, then the clock
// Schmitt. The two Schmitts are independent, so which is read first is
// immaterial; the gating of the clock by resetGate is what matters.)
//
// The float boundary. resetGate is true while `remaining > 0`, and remaining
// is 1e-3f minus n sampleTimes. At 48 kHz that is 48 samples counting the
// reset sample; whether the 49th (a clock 48 samples after the reset) is
// gated depends on which side of zero 1e-3f - 48 * (1.f / 48000.f) lands.
// Measured, it lands below (-1.06e-10 here; -2.04e-5 at 44.1 kHz), so at
// both rates SEQ3 closes on the same sample as our integer window. That
// margin is too thin to promise for every compiler, so the random stream
// still keeps every reset-to-clock gap in [0, 44] or [52, 4000], and the
// boundary is pinned separately on our side with exact sample counts. Where
// the two really differ is 96, 192 and 384 kHz: SEQ3's float runs one sample
// longer there (97 / 193 / 385 against our 96 / 192 / 384), and boundaryTests
// pins the 96 kHz case as the one sample the two disagree on.
//
// SEQ3 has no implicit reset at load (Impromptu does, and so do we), so the
// stream opens with more than 1 ms of silence before the first edge.

#include "modules/Euclid/EuclidEngine.hpp"
#include "check.hpp"

#include <cstdint>
#include <cstdio>
#include <vector>

static const float SR = 48000.f;
static const float DT = 1.f / SR;

// ── Rack's dsp primitives, as written in include/dsp/digital.hpp (v2) ────────

// dsp::SchmittTrigger. The state starts HIGH: a line that is already high at
// power-up is not an edge until it has been low once.
struct RackSchmitt
{
    bool state = true;

    bool process(float in, float low, float high)
    {
        if (state)
        {
            if (in <= low) state = false;
        }
        else if (in >= high)
        {
            state = true;
            return true;
        }
        return false;
    }
};

// dsp::PulseGenerator. trigger() only ever EXTENDS the pulse.
struct RackPulse
{
    float remaining = 0.f;

    bool process(float dt)
    {
        if (remaining > 0.f)
        {
            remaining -= dt;
            return true;
        }
        return false;
    }

    void trigger(float duration)
    {
        if (duration > remaining) remaining = duration;
    }
};

// ── The SEQ3 model ───────────────────────────────────────────────────────────
struct Seq3
{
    RackSchmitt resetTrigger;
    RackSchmitt clockTrigger;
    RackPulse resetPulse;
    int index = 0;
    int numSteps = 8;
    bool resetGate = false;          // this sample's, for the boundary probe
    bool clocked = false;            // this sample's accepted clock

    void process(float resetVolts, float clockVolts, float sampleTime)
    {
        const bool resetTriggered = resetTrigger.process(resetVolts, 0.1f, 2.f);
        if (resetTriggered)
        {
            resetPulse.trigger(1e-3f);
            index = 0;
        }
        resetGate = resetPulse.process(sampleTime);
        const bool clockTriggered = clockTrigger.process(clockVolts, 0.1f, 2.f);
        clocked = clockTriggered && !resetGate;
        if (clocked)
        {
            index++;
            if (index >= numSteps) index = 0;
        }
    }
};

// ── Our side: the same Schmitts in front of euclid::Sequencer ────────────────
//
// The module's adapter in miniature: two Schmitts at the house thresholds and
// the engine, hits 8 / steps 8 / rot 0 so every step is a hit and the index
// is the whole story.
//
struct EuclidSide
{
    RackSchmitt resetTrigger;
    RackSchmitt clockTrigger;
    euclid::Sequencer seq;
    int window;

    explicit EuclidSide(int window_samples) : window(window_samples)
    {
        seq.load(window_samples);
    }

    euclid::Sequencer::Out process(float resetVolts, float clockVolts)
    {
        const bool r = resetTrigger.process(resetVolts, 0.1f, 2.f);
        const bool c = clockTrigger.process(clockVolts, 0.1f, 2.f);
        return seq.process(r, c, window, 8, 8, 0);
    }
};

// ── A deterministic edge stream ──────────────────────────────────────────────

struct XorShift32
{
    uint32_t state;

    explicit XorShift32(uint32_t seed) : state(seed ? seed : 0x9E3779B9u) {}

    uint32_t next()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return state = x;
    }

    int range(int lo, int hi) { return lo + (int)(next() % (uint32_t)(hi - lo + 1)); }
};

// Reset and clock lines as volts per sample: 10 V on an edge sample, 0 V
// otherwise. Edges on one line are at least two samples apart so every 10 V
// sample follows a 0 V one and is a rising edge to the Schmitt. Events are
// placed in time order; a clock whose gap from the most recent reset would
// land in the float-ambiguous band is moved to +52.
struct Stream
{
    std::vector<float> reset;
    std::vector<float> clock;
    int n;
    int last_reset = -1000;
    int last_clock = -1000;
    int resets = 0;
    int clocks = 0;
    int moved = 0;                   // clocks moved off the ambiguous band
    int scenario_count[10];

    explicit Stream(int samples) : reset(samples, 0.f), clock(samples, 0.f), n(samples)
    {
        for (int i = 0; i < 10; i++) scenario_count[i] = 0;
    }

    // Place a reset edge at t or the first legal sample after it. Never
    // before an already placed clock (events are placed in time order).
    // Returns the sample used, or -1 past the end.
    int placeReset(int t)
    {
        if (t < last_reset + 2) t = last_reset + 2;
        if (t < last_clock) t = last_clock;
        if (t >= n) return -1;
        reset[t] = 10.f;
        last_reset = t;
        resets++;
        return t;
    }

    int placeClock(int t)
    {
        if (t < last_clock + 2) t = last_clock + 2;
        const int gap = t - last_reset;
        if (gap >= 45 && gap <= 51)
        {
            t = last_reset + 52;
            moved++;
        }
        if (t >= n) return -1;
        clock[t] = 10.f;
        last_clock = t;
        clocks++;
        return t;
    }

    int cursor() const { return last_reset > last_clock ? last_reset : last_clock; }

    void generate(uint32_t seed)
    {
        XorShift32 rng(seed);
        int t = 60;                  // > 1 ms of silence: SEQ3 has no load window
        while (t < n)
        {
            const int scenario = rng.range(0, 9);
            scenario_count[scenario]++;
            int r, c;
            switch (scenario)
            {
                case 0:              // same-sample reset + clock
                    r = placeReset(t);
                    if (r < 0) return;
                    placeClock(r);
                    break;
                case 1:              // clock 1 sample after reset
                    r = placeReset(t);
                    if (r < 0) return;
                    placeClock(r + 1);
                    break;
                case 2:              // reset 1 sample after clock
                    c = placeClock(t);
                    if (c < 0) return;
                    placeReset(c + 1);
                    break;
                case 3:              // reset, clock inside the window
                    r = placeReset(t);
                    if (r < 0) return;
                    placeClock(r + rng.range(2, 44));
                    break;
                case 4:              // reset, clock just outside the window
                    r = placeReset(t);
                    if (r < 0) return;
                    placeClock(r + rng.range(52, 200));
                    break;
                case 5:              // a burst of clocks
                {
                    const int k = rng.range(2, 12);
                    const int spacing = rng.range(2, 6);
                    for (int i = 0; i < k; i++)
                    {
                        if (placeClock(t + i * spacing) < 0) return;
                    }
                    break;
                }
                case 6:              // a long idle gap
                    t += rng.range(500, 4000);
                    break;
                case 7:              // a reset alone
                    if (placeReset(t) < 0) return;
                    break;
                case 8:              // a clock alone
                    if (placeClock(t) < 0) return;
                    break;
                case 9:              // a reset inside a reset's window, then a clock
                    r = placeReset(t);
                    if (r < 0) return;
                    r = placeReset(r + rng.range(2, 40));
                    if (r < 0) return;
                    placeClock(r + (rng.range(0, 1) ? rng.range(2, 44) : rng.range(52, 120)));
                    break;
            }
            const int at = cursor() > t ? cursor() : t;
            t = at + rng.range(2, 80);
        }
    }
};

// ── The parity run ───────────────────────────────────────────────────────────

static long g_samples_compared = 0;

// Run both models over the stream and compare after every sample. Returns
// the first mismatching sample, or -1.
static long runParity(const Stream& s, int window_samples, int* seq3_clocks_accepted)
{
    Seq3 seq3;
    EuclidSide ours(window_samples);
    long first_mismatch = -1;
    int accepted = 0;
    for (int t = 0; t < s.n; t++)
    {
        seq3.process(s.reset[t], s.clock[t], DT);
        const euclid::Sequencer::Out o = ours.process(s.reset[t], s.clock[t]);
        g_samples_compared++;
        if (seq3.clocked) accepted++;
        if (o.position != seq3.index && first_mismatch < 0)
        {
            first_mismatch = t;
            std::printf("  mismatch at sample %d: SEQ3 index %d, euclid position %d (reset %.0f V, clock %.0f V)\n",
                        t, seq3.index, o.position, s.reset[t], s.clock[t]);
        }
    }
    if (seq3_clocks_accepted) *seq3_clocks_accepted = accepted;
    return first_mismatch;
}

static void randomStreamTests(int window_samples)
{
    std::printf("random edge streams (window %d samples)\n", window_samples);
    static const uint32_t seeds[3] = { 0x12345678u, 0xC0FFEE42u, 0x9E3779B9u };
    for (int k = 0; k < 3; k++)
    {
        Stream s(250000);
        s.generate(seeds[k]);
        int accepted = 0;
        const long mismatch = runParity(s, window_samples, &accepted);
        char label[200];
        std::snprintf(label, sizeof(label),
                      "seed %08x: %d samples, %d resets, %d clocks (%d accepted by SEQ3, %d moved off the float band), "
                      "scenarios %d/%d/%d/%d/%d/%d/%d/%d/%d/%d, position == index on every sample",
                      seeds[k], s.n, s.resets, s.clocks, accepted, s.moved,
                      s.scenario_count[0], s.scenario_count[1], s.scenario_count[2], s.scenario_count[3],
                      s.scenario_count[4], s.scenario_count[5], s.scenario_count[6], s.scenario_count[7],
                      s.scenario_count[8], s.scenario_count[9]);
        std::printf("  %s\n", label);
        check(label, mismatch < 0);
        check("the stream exercised every scenario",
              s.scenario_count[0] && s.scenario_count[1] && s.scenario_count[2] && s.scenario_count[3] &&
              s.scenario_count[4] && s.scenario_count[5] && s.scenario_count[6] && s.scenario_count[7] &&
              s.scenario_count[8] && s.scenario_count[9]);
        check("SEQ3 accepted some clocks and dropped some", accepted > 0 && accepted < s.clocks);
    }
}

// ── Impromptu Clocked at its defaults ────────────────────────────────────────
//
// Stopped, reset pressed: Clocked's clock output goes high on the reset and
// STAYS high; run start produces no edge; the output continues its cycle
// from the start of the period, so the first rising edge after run is beat
// 2. Both models must sit on step 1 through the held-high stretch and step
// through 2, 3, 4 ... on the edges that follow.
//
static void clockedDefaultsTests(int window_samples)
{
    std::printf("Clocked defaults: stopped reset with the clock line held high\n");
    const int n = 6000;
    const int reset_at = 100;        // the reset trigger, held 10 samples
    const int run_at = 2000;         // run start: no edge, the high half continues
    const int period = 400;          // 200 high, 200 low
    Stream s(n);
    for (int t = reset_at; t < reset_at + 10; t++) s.reset[t] = 10.f;
    for (int t = reset_at; t < run_at + period / 2; t++) s.clock[t] = 10.f;
    for (int t = run_at + period / 2; t < n; t++)
    {
        const int phase = (t - run_at) % period;
        s.clock[t] = phase < period / 2 ? 10.f : 0.f;
    }

    Seq3 seq3;
    EuclidSide ours(window_samples);
    bool parity = true;
    bool schedule_seq3 = true;
    bool schedule_ours = true;
    for (int t = 0; t < n; t++)
    {
        seq3.process(s.reset[t], s.clock[t], DT);
        const euclid::Sequencer::Out o = ours.process(s.reset[t], s.clock[t]);
        g_samples_compared++;
        int want = 0;
        if (t >= run_at + period)
        {
            want = ((t - run_at) / period) % 8;
        }
        if (o.position != seq3.index) parity = false;
        if (seq3.index != want) schedule_seq3 = false;
        if (o.position != want) schedule_ours = false;
    }
    check("SEQ3 sits on step 1 through the held-high stretch, then 2, 3, 4 ...", schedule_seq3);
    check("euclid sits on step 1 through the held-high stretch, then 2, 3, 4 ...", schedule_ours);
    check("position == index on every sample", parity);

    // A running reset that lands in the clock's high half (no edge from the
    // master): step 1 at the reset, the next rising edge is step 2.
    std::printf("running reset inside the clock's high half\n");
    Stream s2(n);
    for (int t = 0; t < n; t++) s2.clock[t] = (t % period) < period / 2 ? 10.f : 0.f;
    const int mid_reset = 5 * period + 50;   // 50 samples into a high half
    for (int t = mid_reset; t < mid_reset + 10; t++) s2.reset[t] = 10.f;

    Seq3 seq3b;
    EuclidSide oursb(window_samples);
    parity = true;
    bool step1_at_reset = true;
    bool step2_at_next_edge = true;
    for (int t = 0; t < n; t++)
    {
        seq3b.process(s2.reset[t], s2.clock[t], DT);
        const euclid::Sequencer::Out o = oursb.process(s2.reset[t], s2.clock[t]);
        g_samples_compared++;
        if (o.position != seq3b.index) parity = false;
        if (t >= mid_reset && t < 6 * period && (o.position != 0 || seq3b.index != 0)) step1_at_reset = false;
        if (t >= 6 * period && t < 7 * period && (o.position != 1 || seq3b.index != 1)) step2_at_next_edge = false;
    }
    check("both sit on step 1 from the reset to the next edge", step1_at_reset);
    check("both play step 2 on the next rising edge", step2_at_next_edge);
    check("position == index on every sample", parity);
}

// ── Our integer contract at the exact boundary ───────────────────────────────

// A reset at sample 0, then a clock `gap` samples later, straight into the
// engine (no Schmitts). Returns the position after the clock.
static int positionAfter(int window_samples, int gap)
{
    euclid::Sequencer seq;
    seq.load(window_samples);
    for (int i = 0; i < window_samples; i++) seq.process(false, false, window_samples, 8, 8, 0);
    seq.process(true, false, window_samples, 8, 8, 0);
    for (int i = 1; i < gap; i++) seq.process(false, false, window_samples, 8, 8, 0);
    return seq.process(false, true, window_samples, 8, 8, 0).position;
}

// SEQ3's own float model at `rate`: a reset, then a clock `gap` samples
// later (both lines low for one sample first, so the Schmitts have seen low).
// Returns SEQ3's index after the clock.
static int seq3IndexAfter(float rate, int gap)
{
    Seq3 seq3;
    const float dt = 1.f / rate;
    seq3.process(0.f, 0.f, dt);
    seq3.process(10.f, 0.f, dt);                        // the reset
    for (int i = 1; i < gap; i++) seq3.process(0.f, 0.f, dt);
    seq3.process(0.f, 10.f, dt);                        // the clock, `gap` samples after it
    return seq3.index;
}

// Samples SEQ3's resetGate stays true at `rate`, counting the reset sample.
static int seq3SpanAt(float rate)
{
    RackPulse probe;
    probe.trigger(1e-3f);
    int span = 0;
    while (probe.process(1.f / rate)) span++;
    return span;
}

static void boundaryTests()
{
    std::printf("the exact boundary, integer contract\n");

    // Where SEQ3's float pulse actually ends in this build, next to our
    // integer window (the ResetClockWindow header's rate list).
    std::printf("  SEQ3 float resetGate spans 44.1k %d / 48k %d / 96k %d / 192k %d / 384k %d samples; "
                "samplesFor gives %d / %d / %d / %d / %d\n",
                seq3SpanAt(44100.f), seq3SpanAt(48000.f), seq3SpanAt(96000.f), seq3SpanAt(192000.f), seq3SpanAt(384000.f),
                vgLib_v2::ResetClockWindow::samplesFor(44100.f), vgLib_v2::ResetClockWindow::samplesFor(48000.f),
                vgLib_v2::ResetClockWindow::samplesFor(96000.f), vgLib_v2::ResetClockWindow::samplesFor(192000.f),
                vgLib_v2::ResetClockWindow::samplesFor(384000.f));

    checkEq("ResetClockWindow::samplesFor(48000) is 48",
            vgLib_v2::ResetClockWindow::samplesFor(48000.f), 48);
    checkEq("ResetClockWindow::samplesFor(44100) is 45",
            vgLib_v2::ResetClockWindow::samplesFor(44100.f), 45);
    checkEq("ResetClockWindow::samplesFor(96000) is 96",
            vgLib_v2::ResetClockWindow::samplesFor(96000.f), 96);
    checkEq("ResetClockWindow::samplesFor never returns fewer than 1",
            vgLib_v2::ResetClockWindow::samplesFor(0.f), 1);

    checkEq("48 kHz: a clock 47 samples after a reset is swallowed", positionAfter(48, 47), 0);
    checkEq("48 kHz: a clock 48 samples after a reset is accepted", positionAfter(48, 48), 1);
    checkEq("48 kHz: a same-sample clock is swallowed", positionAfter(48, 0), 0);
    checkEq("48 kHz: a clock 1 sample after a reset is swallowed", positionAfter(48, 1), 0);
    checkEq("44.1 kHz: a clock 44 samples after a reset is swallowed", positionAfter(45, 44), 0);
    checkEq("44.1 kHz: a clock 45 samples after a reset is accepted", positionAfter(45, 45), 1);

    // SEQ3 at the same boundaries. At 44.1 and 48 kHz its float pulse ends on
    // our sample (1e-3f - n * (1.f / rate) lands below zero), so the two
    // agree even on the boundary sample the random streams keep clear of.
    checkEq("48 kHz: SEQ3 accepts the clock 48 samples after a reset too", seq3IndexAfter(48000.f, 48), 1);
    checkEq("48 kHz: and gates the one 47 samples after", seq3IndexAfter(48000.f, 47), 0);
    checkEq("44.1 kHz: SEQ3 accepts the clock 45 samples after a reset too", seq3IndexAfter(44100.f, 45), 1);
    checkEq("44.1 kHz: and gates the one 44 samples after", seq3IndexAfter(44100.f, 44), 0);

    // 96 kHz is one of the three rates (with 192 and 384) where SEQ3's float
    // runs one sample past the standard's ceil(1 ms): its resetGate spans 97
    // samples counting the reset sample, our window 96. A clock exactly 96
    // samples after a reset is accepted here and gated there: the one sample
    // the two disagree on. 95 and 97 agree.
    checkEq("SEQ3's float resetGate at 96 kHz spans 97 samples in this build", seq3SpanAt(96000.f), 97);
    checkEq("96 kHz: a clock 95 samples after a reset is swallowed here", positionAfter(96, 95), 0);
    checkEq("96 kHz: and gated in SEQ3", seq3IndexAfter(96000.f, 95), 0);
    checkEq("96 kHz: a clock 96 samples after a reset is accepted here", positionAfter(96, 96), 1);
    checkEq("96 kHz: but still gated in SEQ3: the one sample the two differ on", seq3IndexAfter(96000.f, 96), 0);
    checkEq("96 kHz: a clock 97 samples after a reset is accepted here", positionAfter(96, 97), 1);
    checkEq("96 kHz: and in SEQ3", seq3IndexAfter(96000.f, 97), 1);
}

int main()
{
    // The contract: the standard's 1 ms at 48 kHz is 48 samples, which is what
    // ResetClockWindow::samplesFor(48000) returns (boundaryTests pins it). The
    // streams run against the helper's value, so parity is shown for what the
    // module actually passes.
    const int window = vgLib_v2::ResetClockWindow::samplesFor(SR);
    std::printf("window at 48 kHz: ResetClockWindow::samplesFor(48000) = %d samples\n", window);

    randomStreamTests(window);
    clockedDefaultsTests(window);
    boundaryTests();

    std::printf("samples compared: %ld\n", g_samples_compared);
    return finish("seq3_parity_test");
}
