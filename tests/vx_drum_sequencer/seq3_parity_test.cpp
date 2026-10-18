// VX Drum Sequencer vs Fundamental SEQ3: the reset/clock rule, sample for
// sample.
//
// SEQ3 is VCV's own sequencer and the reference implementation of the VCV
// Voltage Standards' reset rule (https://vcvrack.com/manual/VoltageStandards;
// https://github.com/VCVRack/Fundamental/blob/v2/src/SEQ3.cpp). Its process(),
// running on an external clock, does per sample:
//
//     resetTriggered = resetTrigger.process(resetV, 0.1f, 2.f);   // dsp::SchmittTrigger
//     if (resetTriggered) { resetPulse.trigger(1e-3f); index = 0; }
//     resetGate = resetPulse.process(args.sampleTime);           // dsp::PulseGenerator
//     clockTriggered = clockTrigger.process(clockV, 0.1f, 2.f);
//     if (clockTriggered && !resetGate) clock = true;
//     if (clock) { index++; if (index >= numSteps) index = 0; }
//     if (index != oldIndex) clockPulse.trigger(1e-3f);          // the TRIG output
//
// Seq3 below is exactly that, with Rack's SchmittTrigger and PulseGenerator
// copied from Rack-SDK/include/dsp/digital.hpp so the FLOAT arithmetic is
// SEQ3's. At 48 kHz resetGate is true on the reset sample and on the 47 after
// it; the 48th sample after is the float boundary (1e-3f minus 48 x 1/48000
// lands a hair below zero here, -1.06e-10, so SEQ3 closes on the same sample
// as our window; the margin is too thin to promise for every compiler), and
// the edge stream below never puts a clock there. Our window is integer:
// ResetClockWindow::samplesFor(48000) == 48, open on the reset sample and the
// 47 after it, closed from the 48th. engine_test.cpp pins that integer
// contract at its exact boundary; this test pins that the two implementations
// agree everywhere SEQ3's own arithmetic is unambiguous. The rates where the
// two really differ by one sample are 96, 192 and 384 kHz (SEQ3's float runs
// 97 / 193 / 385); tests/euclid/seq3_parity_test.cpp pins the 96 kHz case
// against the same shared helper.
//
// What is compared: the playhead position (SEQ3's `index`) after EVERY
// sample of a long deterministic edge stream (xorshift32) that mixes clock
// pulses of many widths and periods, resets on the clock's sample, one
// sample before it and one after, resets inside the clock's high half,
// resets inside an open window, clocks faster than the window; then the
// Impromptu-Clocked-at-defaults scenarios the sync report was about. What
// is not compared: the TRIG output. SEQ3 fires it only when the index MOVED
// (a reset while already on step 1 is silent there); this engine plays step 1
// on every explicit reset, as Impromptu's sequencers do (gates forced low and
// re-raised), because a drum pattern restarted from its downbeat should be
// heard restarting. The stream check accounts for exactly that difference
// and no other.
//
// Both sides use the module's Schmitt thresholds, constants::gate_low_trigger
// 0.1 V / gate_high_trigger 2.0 V, which are also the numbers SEQ3 passes.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o seq3_parity_test seq3_parity_test.cpp && ./seq3_parity_test

#include "VXDrumSequencerEngine.hpp"
#include "check.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

using namespace vx_drum_sequencer;

static const float SR = 48000.f;
static const float DT = 1.f / SR;          // ProcessArgs::sampleTime
static const int NUM_STEPS = 8;
static const int WINDOW = vgLib_v2::ResetClockWindow::samplesFor(SR);
static const float LOW_V = 0.1f;           // constants::gate_low_trigger
static const float HIGH_V = 2.f;           // constants::gate_high_trigger

// dsp::TSchmittTrigger<float>, Rack-SDK dsp/digital.hpp:83-126, the logic
// verbatim (UNINITIALIZED -> HIGH is not an edge).
struct RackSchmitt
{
    enum State { LOW, HIGH, UNINITIALIZED };
    State s = UNINITIALIZED;

    bool process(float in, float lowThreshold, float highThreshold)
    {
        if (s == LOW && in >= highThreshold) { s = HIGH; return true; }
        else if (s == HIGH && in <= lowThreshold) { s = LOW; }
        else if (s == UNINITIALIZED && in >= highThreshold) { s = HIGH; }
        else if (s == UNINITIALIZED && in <= lowThreshold) { s = LOW; }
        return false;
    }
};

// dsp::PulseGenerator, Rack-SDK dsp/digital.hpp:167-195, verbatim.
struct RackPulse
{
    float remaining = 0.f;

    bool process(float deltaTime)
    {
        if (remaining > 0.f)
        {
            remaining -= deltaTime;
            return true;
        }
        return false;
    }

    void trigger(float duration)
    {
        if (duration > remaining) remaining = duration;
    }
};

// SEQ3's reset/clock/index logic, external clock, running.
struct Seq3
{
    RackSchmitt clockTrigger;
    RackSchmitt resetTrigger;
    RackPulse resetPulse;
    int index = 0;
    bool reset_triggered = false;          // this sample's resetTriggered, for the fire accounting

    // One sample. Returns whether TRIG would fire: the index moved.
    bool process(float clockV, float resetV)
    {
        const int oldIndex = index;
        reset_triggered = resetTrigger.process(resetV, LOW_V, HIGH_V);
        if (reset_triggered)
        {
            resetPulse.trigger(1e-3f);
            index = 0;
        }
        const bool resetGate = resetPulse.process(DT);
        const bool clockTriggered = clockTrigger.process(clockV, LOW_V, HIGH_V);
        const bool clock = clockTriggered && !resetGate;
        if (clock)
        {
            index++;
            if (index >= NUM_STEPS) index = 0;
        }
        return index != oldIndex;
    }
};

// The module's adapter around the engine, VXDrumSequencer::process() steps
// 5-7 for a lone sequencer: the load's implicit reset on the first call, RST
// before CLK, seq.reset() before seq.process(). A Memory of length 8 with
// lane 0 lit on every step, so each step fires and every fire is visible.
struct Ours
{
    RackSchmitt clock_trigger;
    RackSchmitt reset_trigger;
    Sequencer seq;
    Memory memory;
    PlaySource src;
    bool loaded = false;

    Ours()
    {
        for (int st = 0; st < NUM_STEPS; st++) memory.at(0, st).on = true;
        memory.length = NUM_STEPS;
        src.memory = &memory;
        src.mute = 0;
    }

    // One sample. Returns whether a step fired.
    bool process(float clockV, float resetV)
    {
        float out[LANES];
        if (!loaded)
        {
            loaded = true;
            seq.rewind(WINDOW);
        }
        const bool reset_edge = reset_trigger.process(resetV, LOW_V, HIGH_V);
        const bool clock_edge = clock_trigger.process(clockV, LOW_V, HIGH_V);
        if (reset_edge) seq.reset(src, WINDOW);
        seq.process(clock_edge, src, SR, 48, out);
        return seq.fired_this_sample;
    }

    int position() const { return seq.head.position; }
};

// ── The edge stream ─────────────────────────────────────────────────────────
//
// Two voltage tracks. Clock: rising edges at random periods (a quarter of
// them 2..10 samples, faster than the window; a quarter 11..100; the rest
// 100..600, a 16th at 5 to 30 BPM... at 48 kHz, wide enough to hold a whole
// window plus slack either side) with a random high width that always leaves
// the line low for at least one sample. Reset: about one clock in three carries a reset edge,
// placed on the clock's sample, one sample before it, one after, 2..41
// samples after it (inside the clock's high half or just past it), or
// anywhere in the period; a quarter of those get a second reset 1..30
// samples later, inside the first's window. A reset is dropped if any clock
// edge would fall 45..51 samples after it (SEQ3's float boundary, see the
// header). The stream opens with 100 quiet samples: SEQ3 arms no window at
// power-up and we do (Impromptu's implicit reset), so the first edge must
// come after ours has closed.
//
struct Stream
{
    std::vector<float> clock;
    std::vector<float> reset;
    int clock_edges = 0;
    int reset_edges = 0;
    int same_sample = 0;                   // reset on a clock edge's sample
    int reset_before = 0;                  // reset one sample before a clock edge
    int reset_after = 0;                   // reset one sample after a clock edge
    int in_high_half = 0;                  // reset while the clock line was high
    int rearmed = 0;                       // a second reset inside the first's window
    int fast_clocks = 0;                   // clock edges inside a window (swallowed)
};

static bool boundaryClear(const std::vector<int>& edges, size_t from, int r)
{
    for (size_t j = from; j < edges.size() && edges[j] <= r + 51; j++)
    {
        const int gap = edges[j] - r;
        if (gap >= 45 && gap <= 51) return false;
    }
    return true;
}

static Stream buildStream(uint32_t seed, int samples)
{
    Rng rng;
    rng.seed(seed);
    Stream st;
    st.clock.assign(samples, 0.f);
    st.reset.assign(samples, 0.f);

    std::vector<int> edges;
    int t = 100;
    while (t < samples)
    {
        int period;
        switch (rng.next() % 4)
        {
            case 0:  period = 2 + (int)(rng.next() % 9u); break;        // 2..10
            case 1:  period = 11 + (int)(rng.next() % 90u); break;      // 11..100
            default: period = 100 + (int)(rng.next() % 501u); break;    // 100..600
        }
        const int width = 1 + (int)(rng.next() % (uint32_t)(period - 1));
        for (int k = 0; k < width && t + k < samples; k++) st.clock[t + k] = 10.f;
        edges.push_back(t);
        t += period;
    }
    st.clock_edges = (int)edges.size();

    int reset_free_from = 0;               // the reset line must be low for a sample before the next edge
    for (size_t i = 0; i < edges.size(); i++)
    {
        if (rng.next() % 3u == 0) continue;
        const int c = edges[i];
        const int next_c = (i + 1 < edges.size()) ? edges[i + 1] : samples;
        int r;
        int kind = (int)(rng.next() % 6u);
        switch (kind)
        {
            case 0:  r = c; break;
            case 1:  r = c - 1; break;
            case 2:  r = c + 1; break;
            case 3:  r = c + 2 + (int)(rng.next() % 40u); break;
            default: r = c + (int)(rng.next() % (uint32_t)std::max(1, next_c - c)); break;
        }
        if (r < reset_free_from || r >= samples) continue;
        if (!boundaryClear(edges, i, r)) continue;

        const int width = 1 + (int)(rng.next() % 10u);
        for (int k = 0; k < width && r + k < samples; k++) st.reset[r + k] = 10.f;
        reset_free_from = r + width + 1;
        st.reset_edges++;
        if (r == c) st.same_sample++;
        else if (r == c - 1) st.reset_before++;
        else if (r == c + 1) st.reset_after++;
        if (st.clock[r] > HIGH_V && r != c) st.in_high_half++;

        // A second reset inside the first's window.
        if (rng.next() % 4u == 0)
        {
            const int r2 = reset_free_from + (int)(rng.next() % 30u);
            if (r2 < r + WINDOW && r2 < samples && boundaryClear(edges, i, r2))
            {
                const int w2 = 1 + (int)(rng.next() % 10u);
                for (int k = 0; k < w2 && r2 + k < samples; k++) st.reset[r2 + k] = 10.f;
                reset_free_from = r2 + w2 + 1;
                st.reset_edges++;
                st.rearmed++;
            }
        }
    }

    // Count the clock edges that land inside a window (for the summary only).
    int last_reset = -1000000;
    size_t e = 0;
    for (int s = 0; s < samples; s++)
    {
        if (st.reset[s] > HIGH_V && (s == 0 || st.reset[s - 1] <= LOW_V)) last_reset = s;
        if (e < edges.size() && edges[e] == s)
        {
            if (s - last_reset < WINDOW) st.fast_clocks++;
            e++;
        }
    }
    return st;
}

// Run both over the stream. Every sample: positions equal. Every fire of
// SEQ3's is ours; every extra fire of ours is an explicit reset that landed
// while already on step 1 (the one documented difference).
static void streamParity(uint32_t seed, int samples)
{
    const Stream st = buildStream(seed, samples);
    Seq3 ref;
    Ours ours;
    int mismatches = 0, first_mismatch = -1;
    int ref_fires = 0, our_fires = 0, extra_ok = 0, extra_bad = 0, missing = 0;
    for (int s = 0; s < samples; s++)
    {
        const bool rf = ref.process(st.clock[s], st.reset[s]);
        const bool of = ours.process(st.clock[s], st.reset[s]);
        if (ours.position() != ref.index)
        {
            mismatches++;
            if (first_mismatch < 0) first_mismatch = s;
        }
        if (rf) ref_fires++;
        if (of) our_fires++;
        if (of && !rf)
        {
            if (ref.reset_triggered && ref.index == 0) extra_ok++; else extra_bad++;
        }
        if (rf && !of) missing++;
    }
    std::printf("  seed %u: %d samples, %d clock edges (%d inside a window), %d reset edges "
                "(%d same-sample, %d one before, %d one after, %d in the high half, %d re-arms); "
                "SEQ3 fired %d, we fired %d\n",
                seed, samples, st.clock_edges, st.fast_clocks, st.reset_edges, st.same_sample,
                st.reset_before, st.reset_after, st.in_high_half, st.rearmed, ref_fires, our_fires);
    checkEq("positions agree on every sample", mismatches, 0);
    if (first_mismatch >= 0) std::printf("  first mismatch at sample %d\n", first_mismatch);
    checkEq("every SEQ3 fire is ours", missing, 0);
    checkEq("every extra fire of ours is a reset while already on step 1", extra_bad, 0);
    check("the stream exercised same-sample resets", st.same_sample >= 20);
    check("the stream exercised reset-before-clock", st.reset_before >= 20);
    check("the stream exercised clock-before-reset", st.reset_after >= 20);
    check("the stream exercised resets in the clock's high half", st.in_high_half >= 20);
    check("the stream exercised re-armed windows", st.rearmed >= 20);
    check("the stream exercised clocks inside a window", st.fast_clocks >= 20);
}

// ── Impromptu Clocked at its defaults ───────────────────────────────────────
//
// Clocked's README: "the clock outputs are held high when a stopped clock is
// reset [...] such that when starting again, it will not miss the first step
// in sequential switches or sequencers that don't have run inputs." So: a
// reset while stopped raises the clock output on the same sample as the
// reset pulse; run start produces NO edge; the first rising edge after the
// start is one period later and is beat 2. A sequencer that goes to "no
// step" on reset and plays step 1 on the next clock plays its step 1 on beat
// 2 there: the sync report.
//
struct Pair
{
    Seq3 ref;
    Ours ours;
    int fires = 0;                         // ours
    int disagreements = 0;

    void sample(float clockV, float resetV)
    {
        ref.process(clockV, resetV);
        if (ours.process(clockV, resetV)) fires++;
        if (ours.position() != ref.index) disagreements++;
    }

    void quiet(int n, float clockV = 0.f) { for (int i = 0; i < n; i++) sample(clockV, 0.f); }

    // One beat of a 50 % duty clock: high for P/2 from the rising edge, then low.
    void beat(int P) { quiet(P / 2, 10.f); quiet(P - P / 2, 0.f); }
};

static void clockedStopResetRun()
{
    std::printf("Clocked at defaults: stop, reset, run\n");
    const int P = 1000;
    Pair p;
    p.quiet(100);                          // the patch loaded; nothing playing

    // Running: four beats. After load both are on step 1; the first edge
    // plays step 2 (SEQ3 does exactly this too), so four beats end on step 5.
    for (int b = 0; b < 4; b++) p.beat(P);
    checkEq("four beats after load: step 5", p.ours.position(), 4);
    checkEq("SEQ3 agrees", p.ref.index, 4);
    checkEq("four fires, one per edge", p.fires, 4);

    // Stop: the clock line rests low for a while.
    p.quiet(700);

    // Reset while stopped: the reset pulse, and the clock output goes HIGH on
    // the same sample and stays there (no edge is produced by run start).
    const int before = p.fires;
    p.sample(10.f, 10.f);
    checkEq("the stopped reset plays step 1", p.ours.position(), 0);
    checkEq("at the reset instant", p.fires, before + 1);
    checkEq("SEQ3 is on step 1 too", p.ref.index, 0);
    for (int i = 0; i < 9; i++) p.sample(10.f, 10.f);
    p.quiet(500, 10.f);                    // stopped, reset released, clock still high

    // Run: the cycle restarts with the output high: high for the rest of the
    // half period, low, then the first rising edge = beat 2.
    p.quiet(P / 2, 10.f);
    p.quiet(P - P / 2, 0.f);
    checkEq("no edge at run start: still step 1 after the first half-period", p.ours.position(), 0);
    checkEq("still one fire", p.fires, before + 1);
    p.beat(P);                             // the rising edge at the start of this beat is beat 2
    checkEq("beat 2 plays step 2 (the old rule played step 1 here)", p.ours.position(), 1);
    checkEq("SEQ3 is on step 2", p.ref.index, 1);
    p.beat(P);
    checkEq("beat 3 plays step 3", p.ours.position(), 2);
    checkEq("no disagreement anywhere in the scenario", p.disagreements, 0);
}

static void clockedRunningReset()
{
    std::printf("Clocked running: a reset in the clock's high half\n");
    const int P = 1000;
    Pair p;
    p.quiet(100);
    for (int b = 0; b < 3; b++) p.beat(P);
    checkEq("three beats: step 4", p.ours.position(), 3);

    // Beat 4 begins; 200 samples into its high half the user presses reset.
    // Clocked restarts its cycle from the reset: the output is already high
    // and stays high for a new half period (Count Modula's remark in the
    // community thread: no positive edge is presented to the sequencers).
    p.quiet(1, 10.f);                      // the rising edge of beat 4
    p.quiet(199, 10.f);
    const int before = p.fires;
    p.sample(10.f, 10.f);                  // the reset, clock line high
    checkEq("the reset in the high half plays step 1", p.ours.position(), 0);
    checkEq("once", p.fires, before + 1);
    for (int i = 0; i < 9; i++) p.sample(10.f, 10.f);
    p.quiet(P / 2 - 10, 10.f);             // the restarted cycle's high half
    p.quiet(P - P / 2, 0.f);
    checkEq("no edge until the restarted cycle rises: still step 1", p.ours.position(), 0);
    p.beat(P);
    checkEq("the next rising edge is step 2", p.ours.position(), 1);
    checkEq("SEQ3 agrees", p.ref.index, 1);
    checkEq("no disagreement anywhere in the scenario", p.disagreements, 0);
}

static void clockedRunningResetSameSample()
{
    std::printf("Clocked running: reset and clock edge on the same sample\n");
    const int P = 1000;
    Pair p;
    p.quiet(100);
    for (int b = 0; b < 3; b++) p.beat(P);
    checkEq("three beats: step 4", p.ours.position(), 3);

    // The reset lands in the low half; Clocked restarts its cycle at once, so
    // the clock output rises on the reset's sample.
    p.quiet(1, 10.f);
    p.quiet(P / 2 - 1, 10.f);
    p.quiet(300, 0.f);
    const int before = p.fires;
    p.sample(10.f, 10.f);                  // reset + clock rise, same sample
    checkEq("same-sample reset + clock: step 1", p.ours.position(), 0);
    checkEq("once", p.fires, before + 1);
    for (int i = 0; i < 9; i++) p.sample(10.f, 10.f);
    p.quiet(P / 2 - 10, 10.f);
    p.quiet(P - P / 2, 0.f);
    p.beat(P);
    checkEq("the next edge is step 2", p.ours.position(), 1);
    checkEq("SEQ3 agrees", p.ref.index, 1);
    checkEq("no disagreement anywhere in the scenario", p.disagreements, 0);
}

int main()
{
    std::printf("xorshift32 edge streams\n");
    streamParity(0x20260916u, 250000);
    streamParity(1u, 250000);
    streamParity(0xDEADBEEFu, 250000);
    streamParity(48000u, 250000);
    clockedStopResetRun();
    clockedRunningReset();
    clockedRunningResetSameSample();
    return finish("seq3_parity_test");
}
