#pragma once
//
// Euclid — the sequencing engine, Rack-free.
//
// Three things live here and nothing else: the CV law that turns a knob, a
// jack and an attenuverter into one effective integer; the Bresenham pattern
// with its dirty cache; and the playhead under the VCV reset rule. The module
// (Euclid.hpp) is the Rack adapter: it reads the jacks, runs the two Schmitt
// triggers, calls Sequencer::process once per sample and shapes the TRIG pulse
// and the GATE gap from what comes back. Nothing here knows the sample rate.
// The one rate-dependent quantity the engine needs, the 1 ms clock-ignore
// window, arrives as a sample count the module computes when it arms it
// (vgLib_v2::ResetClockWindow::samplesFor), so the engine is pure integers and
// compiles standalone for tests/euclid/.
//
// Behaviour contract: euclid.c (~/vxsynth/app/studio/lib/audio/wasm/modules/
// euclid.c), itself a port of GAEuclid.hpp (voxglitch_devices,
// src/GrooveboxAdvanced/modules). The CV law and the pattern are
// euclid_process_inline and euclid_recompute_pattern, carried over unchanged
// (euclid-migration.md §2). The reset rule is NOT the source's. euclid.c arms
// a 10 ms window and holds the first clock after a reset on step 0
// (firstStep), which puts it a beat late behind a master that sends reset and
// clock together and a step behind one that sends reset alone. This engine
// follows the VCV standard as Fundamental SEQ3 writes it (euclid-migration.md
// §4.1; the rule and the per-sample order are in ResetClockWindow.hpp): a
// reset makes step 1 current and plays it, clock edges inside the 1 ms window
// that follows are swallowed, the next clock after the window is step 2's.
//
// BRESENHAM, NOT BJORKLUND. base[0] is a hit, and base[i] is a hit when the
// integer quotient i*h/s is larger than (i-1)*h/s. Rotation is the cyclic
// shift pattern[i] = base[(i + rot) % s], so a larger ROT pulls the hits to
// EARLIER indices. Bjorklund produces the same rhythms up to rotation but not
// the same rotation: E(5,8) is x.x.xx.x here where Bjorklund gives x.xx.xx.
// vxsynth patches and the Groovebox Advanced module depend on this one;
// tests/euclid/pattern_test.cpp pins it. Do not "correct" it.
//

#include "vgLib-2.0/sequencer/ResetClockWindow.hpp"

namespace euclid
{

static const int MAX_STEPS = 32;

static const int DEFAULT_HITS = 4;
static const int DEFAULT_STEPS = 16;
static const int DEFAULT_ROT = 0;

// ── The CV law ───────────────────────────────────────────────────────────────
//
// Source: euclid_process_inline, "CV modulation (±5V ≈ ±16 steps via ×3.2;
// (int) truncates toward zero)". The attenuverter scales the CV BEFORE the
// truncation, so 0 silences the jack, +1 passes it and -1 inverts it. The
// truncation is toward zero, not floor: 1 V is +3 steps (3.2), not 4, and
// -0.5 V is -1 (-1.6), not -2. The module applies it three times, with
// (0, 32) for HITS, (1, 32) for STEPS and (0, steps - 1) for ROT, then clamps
// HITS to the effective STEPS (§2).
//
inline int effectiveValue(int knob, float cv, float atten, int lo, int hi)
{
    int v = knob + (int)(cv * 3.2f * atten);
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return v;
}

// ── Pattern ──────────────────────────────────────────────────────────────────
//
// The hit table for one (hits, steps, rot), dirty-cached on those three
// integers the way euclid_recompute_pattern caches on (cachedHits,
// cachedSteps, cachedRotation): compute() is called every sample and does
// nothing when nothing changed. The three ints stored are the ones the table
// was computed FROM, after the defensive clamps below, and -1 -1 -1 until the
// first compute so that the first call always computes.
//
struct Pattern
{
    bool hit[MAX_STEPS];             // false beyond `steps`
    int hits;                        // 0..steps
    int steps;                       // 1..MAX_STEPS
    int rot;                         // 0..steps-1

    Pattern() : hits(-1), steps(-1), rot(-1)
    {
        for (int i = 0; i < MAX_STEPS; i++) hit[i] = false;
    }

    // Recompute if (h, s, r) differ from the cached triple. Returns true only
    // when it recomputed. The module already applies the CV law and clamps
    // hits to steps; the clamps here are for the engine's own safety and for
    // callers that do not (steps 1..32, hits 0..steps, rot brought into
    // 0..steps-1 by modulo, negative values included).
    bool compute(int h, int s, int r)
    {
        if (s < 1) s = 1;
        if (s > MAX_STEPS) s = MAX_STEPS;
        if (h < 0) h = 0;
        if (h > s) h = s;
        r %= s;
        if (r < 0) r += s;

        if (h == hits && s == steps && r == rot) return false;
        hits = h;
        steps = s;
        rot = r;

        for (int i = 0; i < MAX_STEPS; i++) hit[i] = false;

        // Source: "if (sCount <= 0 || h <= 0) return;" — all rests.
        if (h == 0) return true;

        // Source: "if (effectiveHits >= sCount) ... all hits".
        if (h >= s)
        {
            for (int i = 0; i < s; i++) hit[i] = true;
            return true;
        }

        // Source: "base[0] = 1; base[i] = ((i * effectiveHits) / sCount) >
        // (((i - 1) * effectiveHits) / sCount)". Integer division throughout.
        bool base[MAX_STEPS];
        for (int i = 0; i < MAX_STEPS; i++) base[i] = false;
        base[0] = true;
        for (int i = 1; i < s; i++)
        {
            base[i] = ((i * h) / s) > (((i - 1) * h) / s);
        }

        // Source: "pattern[i] = base[(i + rot) % sCount]".
        for (int i = 0; i < s; i++)
        {
            hit[i] = base[(i + r) % s];
        }
        return true;
    }

    // Hits in the table: min(hits, steps) by construction.
    int count() const
    {
        int n = 0;
        for (int i = 0; i < MAX_STEPS; i++) if (hit[i]) n++;
        return n;
    }
};

// ── Sequencer ────────────────────────────────────────────────────────────────
//
// The playhead and the reset rule. `position` is the step index: 0 means
// step 1 is current, which it is after load() and after every reset (the VCV
// standard, euclid-migration.md §4.1), so GATE is high at rest whenever step
// 1 is a hit, as in the source.
//
// What process() reports for one sample:
//
//   gate      step `position` is a hit: the GATE level for this sample
//   struck    the playhead LANDED on a hit this sample (a clock edge onto a
//             hit, or a reset with step 1 a hit): the module starts a TRIG
//             pulse. False on every other sample, so consecutive hit steps
//             make one continuous GATE and one TRIG per step
//   gate_gap  GATE was high on the previous sample and a reset just moved the
//             playhead: the module forces ONE sample of 0 V before showing
//             step 1's gate so a downstream envelope sees a fresh edge
//             (Impromptu's gate retrigger, at the VX Drum Sequencer's
//             one-sample width, §4.1 rule 1). `gate` is still the logical
//             level; the gap is the module's to apply
//   position  the step index after this sample
//
struct Sequencer
{
    Pattern pattern;
    int position;                    // step index; 0 = step 1 current after load and reset
    vgLib_v2::ResetClockWindow window;
    bool gate_high;                  // `gate` as reported by the last process() call

    struct Out
    {
        bool gate;
        bool struck;
        bool gate_gap;
        int position;
    };

    Sequencer() : position(0), gate_high(false) {}

    // The implicit reset at patch load (Impromptu: "the clock is also ignored
    // during 1ms when the module is initialized or powered up"): step 1
    // current, the window armed, nothing fires. gate_high stays false: there
    // was no output before this, so the first sample needs no gap.
    void load(int window_samples)
    {
        position = 0;
        window.arm(window_samples);
        gate_high = false;
    }

    // One sample, in SEQ3's order (ResetClockWindow.hpp): the pattern, then
    // the reset, then the clock, then the window tick, then the outputs.
    //
    // A reset never counts as a clock and the window never blocks a reset. The
    // clock check is an independent `if`, not an `else`: a same-sample reset
    // and clock is swallowed by the window the reset just armed, exactly as
    // SEQ3's `clock && !resetGate` swallows it, so step 1 plays once, at the
    // reset. A clock one sample BEFORE a reset is the standard's known
    // limitation and plays step N+1 then step 1 (§4.1 table, row 3).
    //
    // `window_samples` is whatever the caller passed: the module computes it
    // from the sample rate when it arms, never caches it, so a rate change
    // is the caller's business (§4.3).
    Out process(bool reset_edge, bool clock_edge, int window_samples, int hits, int steps, int rot)
    {
        Out out;
        out.struck = false;
        out.gate_gap = false;

        pattern.compute(hits, steps, rot);
        const int length = pattern.steps;

        if (reset_edge)
        {
            position = 0;
            window.arm(window_samples);
            out.struck = pattern.hit[0];
            out.gate_gap = gate_high;
        }

        if (clock_edge && !window.open())
        {
            // Source: "currentStep = (currentStep + 1) % effectiveSteps". A
            // STEPS that shrank below the playhead wraps here (§2).
            position = position + 1;
            if (position >= length) position = 0;
            out.struck = pattern.hit[position];
        }

        window.tick();

        // A playhead left beyond a shrunken STEPS reads a cleared slot until
        // the next clock wraps it (§4.1 table, last row).
        out.gate = position < length && pattern.hit[position];
        gate_high = out.gate;
        out.position = position;
        return out;
    }
};

} // namespace euclid
