#pragma once
//
// VX Drum Sequencer — the sequencing engine, Rack-free.
//
// Everything the sequencer DOES between "a clock edge landed" and "these
// are this sample's output voltages" lives here, in four objects with one
// job each, composed by Sequencer:
//
//   ClockFollower  measures the clock period (the anchor rule, the rescale)
//   Playhead       the position and the first-step convention
//   Ratchets       the re-strikes in flight per voice lane
//   TriggerShaper  strike bits -> pulses, with the gap rule and the ACC cut
//
// plus resolveStep(), a function of (memory, mute, position, a random source)
// that says which lanes strike and which ratchets arm — random because a pad
// can carry a CHANCE below 100 % (2026-09-07), rolled once per step reached. The module (VXDrumSequencer.hpp)
// is the Rack adapter: it reads the jacks, resolves WHICH memory plays this
// sample (a PlaySource), calls Sequencer::process, and writes the jacks.
//
// The engine never learns which module a PlaySource came from. That is what
// lets one Sequencer drive a chain of adjacent modules (chain-foundations.md
// §2-§3): the head passes whichever member's memory is active, and a hand-off
// is Playhead::rewind() on the same engine. The ratchets armed on the last
// step of one member finish while the next member's step 1 fires, because a
// hand-off does not call Sequencer::rewind() (which cancels them).
//
// Timing rules carried over from the module unchanged (brief-clock-reset §5-§7
// and rack-port-design.md): advance-then-play from -1, no clock-ignore window
// after a reset, the period is measured only from a valid anchor inside the
// (1 sample, 4 s) window and survives a reset, ratchets need a known period.
//
// Compiled standalone by tests/vx_drum_sequencer/.
//

#include <algorithm>
#include <cstdint>

#include "VXDrumSequencerTypes.hpp"

namespace vx_drum_sequencer
{

// ── ClockFollower ────────────────────────────────────────────────────────────
//
// Samples since the last detected edge, and the period that measurement
// yields. The counter is int64_t, not the house `long`: `long` is 32-bit under
// MinGW-w64 and, with no clock patched, nothing zeroes this counter — it would
// overflow (UB) after 2^31 samples.
//
struct ClockFollower
{
    int64_t counter = 0;             // samples since the last DETECTED edge (accepted or not)
    bool anchor_valid = false;       // false after load/reset/rewind: the next edge anchors, does not measure
    float period_samples = 0.f;      // measured clock period, 0 = unknown (only at load). Survives reset.

    void tick() { counter++; }

    // An edge landed this sample. Measure only from a valid anchor, inside
    // the sync-effects debounce window (> 1 sample, < 4 s). The partial
    // interval after a reset is never taken as the period; period_samples
    // keeps its last good value so ratchets on step 0 still subdivide
    // sensibly (§6.d). ALWAYS zero the counter, measured or not
    // (PianoRoll.hpp:646-652: an edge that does not zero it makes the next
    // measured edge report two intervals).
    void edge(float sample_rate)
    {
        if (anchor_valid && counter > 1 && counter < (int64_t)(sample_rate * 4.f))
        {
            period_samples = (float)counter;
        }
        counter = 0;
        anchor_valid = true;
    }

    // Reset / rewind: the next edge anchors without measuring (ArpSeq.hpp:879-881,
    // §6.d). The counter, NOT the period (PianoRoll.hpp:629-631).
    void rearm()
    {
        anchor_valid = false;
        counter = 0;
    }

    // The period is the one thing in the clock path stored as samples that
    // outlives its measurement. Rescale it so ratchets stay right until the
    // next edge re-measures, and re-anchor: the counter now straddles two
    // rates and must not be read as a period.
    void rescale(float old_rate, float new_rate)
    {
        if (old_rate > 0.f && new_rate > 0.f && new_rate != old_rate)
        {
            period_samples *= new_rate / old_rate;
            rearm();
        }
    }
};

// ── Playhead ─────────────────────────────────────────────────────────────────
//
// FIRST-STEP CONVENTION (brief-clock-reset §3, §5): advance-then-play from
// position -1. "Has anything played yet?" is position >= 0.
//
struct Playhead
{
    int position = -1;               // -1 = nothing played since load/reset/rewind/hand-off

    // Advance one step (source: vxd_step + vxd_step_at, vxdrums.c:399-421,
    // minus swing). From -1 this lands on step 0: the first clock after a
    // reset plays step 1. A length that shrank below the position wraps.
    int advance(int length)
    {
        length = std::max(1, std::min(length, STEPS));
        int next = position + 1;
        if (next < 0 || next >= length) next = 0;
        position = next;
        return position;
    }

    void rewind() { position = -1; }
};

// ── Rng ──────────────────────────────────────────────────────────────────────
//
// A xorshift32 for the chance rolls: Rack-free, allocation-free, seedable so
// the tests are deterministic. The module seeds it from rack::random at
// construction. Not persisted.
//
struct Rng
{
    uint32_t state = 0x9E3779B9u;

    void seed(uint32_t s) { state = s ? s : 0x9E3779B9u; }

    uint32_t next()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return state = x;
    }

    // 0..99, uniform.
    int percent() { return (int)(next() % 100u); }
};

// A pad with `chance` percent plays when a 0..99 roll lands below it: 100
// always (no roll is taken), 0 never, 50 half the time.
inline bool passes(uint8_t chance, Rng& rng)
{
    if (chance >= CHANCE_MAX) return true;
    if (chance == 0) return false;
    return rng.percent() < (int)chance;
}

// ── One step's decisions ─────────────────────────────────────────────────────
//
// What playing step `pos` of `m` under `mute` means: which lanes strike (bit 6
// = the accent lane, set when the step is accented and the accent lane is not
// muted) and how many EXTRA ratchet hits each voice lane arms. Each lit pad
// is rolled once against its chance; a pad that fails the roll neither
// strikes nor arms its ratchets, and does not light its lane's flash.
//
struct StepFire
{
    uint32_t struck = 0;             // lane bits; bit ACCENT_LANE = the step is accented
    uint8_t extra[VOICES] = {};      // 0..3 extra hits per voice lane (0 for lanes not struck)
    bool accented = false;
};

// Source: vxd_fire, vxdrums.c:369-396. A muted accent lane removes the boost
// AND the accent lamp bit (:373-375); a muted voice lane neither strikes nor
// pulses (:379).
inline StepFire resolveStep(const Memory& m, uint8_t mute, int pos, Rng& rng)
{
    StepFire f;
    if (pos < 0 || pos >= STEPS) return f;

    const Step& acc = m.at(ACCENT_LANE, pos);
    f.accented = !(mute & (1u << ACCENT_LANE)) && acc.on && passes(acc.chance, rng);
    if (f.accented) f.struck |= (1u << ACCENT_LANE);

    for (int l = 0; l < VOICES; l++)
    {
        if (mute & (1u << l)) continue;
        const Step& st = m.at(l, pos);
        if (!st.on) continue;
        if (!passes(st.chance, rng)) continue;
        f.struck |= (1u << l);
        f.extra[l] = (uint8_t)std::min((int)st.ratchet, RATCHET_MAX);
    }
    return f;
}

// ── Ratchets ─────────────────────────────────────────────────────────────────
//
// Re-strikes in flight, per voice lane (source: ratLeft/ratInterval/ratCount).
// Evenly subdivide the MEASURED clock period; without a known period (before
// the second clock edge, §6.e) the extra hits are dropped rather than guessed
// (:383-393). An accented step's re-strikes re-raise ACC (DESIGN §2).
//
struct Ratchets
{
    int left[VOICES] = {};
    int64_t interval[VOICES] = {};
    int64_t count[VOICES] = {};
    bool accented[VOICES] = {};

    void arm(const StepFire& f, float period_samples)
    {
        if (!(period_samples > 4.f)) return;
        for (int l = 0; l < VOICES; l++)
        {
            if (!((f.struck >> l) & 1u) || f.extra[l] == 0) continue;
            left[l] = f.extra[l];
            interval[l] = std::max((int64_t)2, (int64_t)(period_samples / (float)(f.extra[l] + 1)));
            count[l] = interval[l];
            accented[l] = f.accented;
        }
    }

    // Every sample (source :674-682). Returns the strike bits due NOW: the
    // re-striking lanes, plus the ACC bit when any of them was accented. A
    // lane muted mid-step still finishes its re-strikes (source parity).
    uint32_t tick()
    {
        uint32_t strikes = 0;
        for (int l = 0; l < VOICES; l++)
        {
            if (left[l] > 0 && --count[l] <= 0)
            {
                strikes |= (1u << l);
                if (accented[l]) strikes |= (1u << ACCENT_LANE);
                left[l]--;
                count[l] = interval[l];
            }
        }
        return strikes;
    }

    // Cut everything in flight (source: vxd_silence, vxdrums.c:424-430).
    void cancel()
    {
        for (int l = 0; l < VOICES; l++) left[l] = 0;
    }

    bool any() const
    {
        for (int l = 0; l < VOICES; l++) if (left[l] > 0) return true;
        return false;
    }
};

// ── TriggerShaper ────────────────────────────────────────────────────────────
//
// 10 V pulses of `pulse_len` samples, as sample countdowns (Timeline.hpp:101-105
// form, not dsp::PulseGenerator — whose trigger() only EXTENDS a pulse and can
// never emit a second edge inside one, which is why GrooveBox ratchets merge
// at the expander). Lane index ACCENT_LANE is the ACC gate.
//
// GAP RULE: a strike on a lane whose output was HIGH on the previous sample
// forces exactly ONE sample at 0 V, then a fresh pulse (one sample is enough
// for every Rack Schmitt and survives the >= 2-sample ratchet floor). It keys
// on the previous sample's OUTPUT (was_high), not on the countdown: a pulse of
// pulse_len samples has pulse_remaining == 0 on its last high sample, so a
// strike landing exactly pulse_len after the previous one (100 ms triggers
// with a clock whose period equals the pulse) would otherwise go 10 V -> 10 V
// and merge every hit into one long pulse. The rule is applied to the
// sample's whole strike SET at once: if any struck lane (ACC included) needs
// the gap, every struck lane takes it, so the ACC gate and the trigger it
// accents always rise on the SAME sample — the kit samples ACC at the trigger
// instant (DESIGN §2, VXDrums.hpp:203). Per-lane gaps would let a still-high
// ACC lag its trigger by a sample with a long pulse length and lose the accent.
//
// ACC CUT: an un-accented strike set ends any ACC gate still high from an
// earlier accented strike on the sample its voice lanes RISE (this sample, or
// the next one when the set takes the gap), so the kit reads 0 V at the
// trigger edge. The source accents per step (vxdrums.c:373-374); with a pulse
// longer than the clock period (200 ms above 75 BPM, 100 ms above 150 BPM at
// one pulse per 16th) the gate would otherwise carry one step's accent onto
// the next hit. Cutting at the rise rather than at the strike keeps an
// accented set that took the gap on the previous sample rising with ACC high.
// One case stays unresolvable: an accented lane and an un-accented lane rising
// on the SAME sample (an accented re-strike landing on an un-accented step
// fire, or one gap-delayed onto the other's fire). The mono ACC cable of
// DESIGN §2 cannot serve both; the un-accented strike wins and ACC is low.
//
// Every claim above is a case in tests/vx_drum_sequencer/shaper_test.cpp.
//
struct TriggerShaper
{
    uint32_t strike_mask = 0;        // lanes struck THIS sample; resolved into pulses by tick()
    int64_t pulse_remaining[LANES] = {};
    bool gap_sample[LANES] = {};     // force 0 V this sample (the re-strike gap)
    bool was_high[LANES] = {};       // output was > 0 V on the previous sample

    void strike(uint32_t lanes) { strike_mask |= lanes; }

    // One sample: resolve this sample's strike set, then write every lane's
    // voltage into out[LANES].
    void tick(int64_t pulse_len, float out[LANES])
    {
        if (pulse_len < 1) pulse_len = 1;

        if (strike_mask)
        {
            bool need_gap = false;
            for (int lane = 0; lane < LANES; lane++)
            {
                if ((strike_mask & (1u << lane)) && was_high[lane]) need_gap = true;
            }

            // ACC CUT (see above): with the ACC bit clear, at least one voice
            // lane is striking un-accented, so the gate must be low on the
            // sample those lanes RISE: now, or after the gap. With a gap the
            // gate keeps this one sample, so an accented set that took the gap
            // on the previous sample still rises with ACC high.
            if (!(strike_mask & (1u << ACCENT_LANE)) && pulse_remaining[ACCENT_LANE] > 0)
            {
                pulse_remaining[ACCENT_LANE] = need_gap ? 1 : 0;
            }

            for (int lane = 0; lane < LANES; lane++)
            {
                if (!(strike_mask & (1u << lane))) continue;
                gap_sample[lane] = need_gap;
                pulse_remaining[lane] = pulse_len + (need_gap ? 1 : 0);
            }

            strike_mask = 0;
        }

        for (int lane = 0; lane < LANES; lane++)
        {
            float v;
            if (gap_sample[lane])
            {
                v = 0.f;
                gap_sample[lane] = false;
            }
            else
            {
                v = (pulse_remaining[lane] > 0) ? 10.f : 0.f;
            }
            was_high[lane] = v > 0.f;
            if (pulse_remaining[lane] > 0) pulse_remaining[lane]--;
            out[lane] = v;
        }
    }

    void reset()
    {
        strike_mask = 0;
        for (int lane = 0; lane < LANES; lane++)
        {
            pulse_remaining[lane] = 0;
            gap_sample[lane] = false;
            was_high[lane] = false;
        }
    }
};

// ── What the engine plays, and what it reports ───────────────────────────────

// The memory and mute mask that apply THIS sample. Resolved by the owner (its
// MEM CV, its buttons, its mute) — or, in a chain, by the head from whichever
// member is active. The pointer must outlive the process() call it is passed
// to and nothing more.
struct PlaySource
{
    const Memory* memory = nullptr;
    uint8_t mute = 0;
};

// What the UI needs, one struct (chain-foundations.md §4). The engine fills
// position / fired_mask / fired_serial; `slot` is the owner's (the effective
// memory shown and edited) and the engine never touches it.
struct Report
{
    int position = -1;               // current step, -1 = nothing played yet
    uint32_t fired_mask = 0;         // lanes struck at the last step fire, bit ACCENT_LANE = accent lamp
    uint32_t fired_serial = 0;       // incremented on every step fire; widgets flash lanes when it changes
    int slot = 0;                    // owner's: the effective memory
};

// ── Sequencer ────────────────────────────────────────────────────────────────
struct Sequencer
{
    ClockFollower clock;
    Playhead head;
    Ratchets ratchets;
    TriggerShaper shaper;
    Report report;
    Rng rng;                         // the chance rolls

    // True after process() when a step fired on that call.
    bool fired_this_sample = false;

    // Reset = rewind (source: vxd_reset_pos, :542-549; §6.h). Cancels the
    // ratchets (silence, :424-430) and the lamp bits; trigger pulses already
    // high are left to finish (§6.k). No clock-ignore window.
    void rewind()
    {
        ratchets.cancel();
        report.fired_mask = 0;       // the serial is untouched: nothing fired
        head.rewind();
        report.position = head.position;
        clock.rearm();
    }

    // Hand-off to the next member of a chain: the playhead only. Ratchets
    // armed on the step that just fired keep running, the period keeps its
    // anchor.
    void handOff()
    {
        head.rewind();
        report.position = head.position;
    }

    // One sample, in the module's order (brief-clock-reset §5.3):
    //   clock counter -> the edge (measure, advance, fire) -> ratchet re-strikes
    //   -> the pulses. `reset` is handled by the caller (rewind()) BEFORE this,
    //   so a same-sample reset + clock fires step 0 on this sample (§6.b).
    void process(bool clock_edge, const PlaySource& src, float sample_rate,
                 int64_t pulse_len, float out[LANES])
    {
        fired_this_sample = false;
        clock.tick();

        if (clock_edge)
        {
            clock.edge(sample_rate);
            if (src.memory)
            {
                const int pos = head.advance(src.memory->length);
                fire(*src.memory, src.mute, pos);
            }
        }

        shaper.strike(ratchets.tick());
        shaper.tick(pulse_len, out);
    }

    // Fire step `pos` NOW (source: vxd_fire). Strikes, arms the ratchets, and
    // writes the report — step fires only, never re-strikes (source
    // report.fired).
    void fire(const Memory& m, uint8_t mute, int pos)
    {
        const StepFire f = resolveStep(m, mute, pos, rng);
        shaper.strike(f.struck);
        ratchets.arm(f, clock.period_samples);
        report.position = head.position;
        report.fired_mask = f.struck;
        report.fired_serial++;
        fired_this_sample = true;
    }
};

} // namespace vx_drum_sequencer
