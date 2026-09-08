#pragma once
//
// VX Drum Sequencer — TWEAK: deterministic, per-lane pattern evolution.
// Rack-free; compiled standalone by tests/vx_drum_sequencer/.
//
// EXPERIMENT, 2026-09-08 (Bret's idea). The TWEAK CV picks a LEVEL 0..8;
// level 0 is the stored pattern untouched, and each level up applies ONE
// more edit from the lane's ordered list. Same base, same level, same
// result, forever: no randomness, no seed. The stored memory is never
// changed; the module hands the engine the tweaked copy as its PlaySource.
//
// Each lane ROLE (BD SD CP PERC CH OH AC) has its own list, written for
// that instrument's idiom. An entry is an edit on one step of THAT lane
// only — a lane never looks at another lane — with an optional condition on
// another step of the same lane, so a snare list behaves differently on a
// bare backbeat than on a busy pattern. An entry that would change nothing
// (adding a hit that is already there, removing one that is not, a step
// beyond the memory's length, a failed condition) is SKIPPED and does not
// count as a level, so every level differs from the one below by exactly
// one edit. Lists carry more entries than there are levels for that reason.
//
// Step numbers below are 0-based: beats on 0 4 8 12, "e" on 1 5 9 13,
// "&" on 2 6 10 14, "a" on 3 7 11 15.
//

#include <cstdint>

#include "VXDrumSequencerTypes.hpp"

namespace vx_drum_sequencer
{

static const int TWEAK_LEVELS = 8;
static const int TWEAK_MAX_EDITS = 16;
static const uint8_t TWEAK_SOFT_CHANCE = 60;   // an ADD_SOFT hit plays this often

enum TweakOp : uint8_t
{
    TW_END = 0,    // end of list (the zero value, so a short initializer list ends itself)
    TW_ADD,        // light the step at 100 %
    TW_ADD_SOFT,   // light the step at TWEAK_SOFT_CHANCE: a ghost
    TW_REMOVE,     // clear the step
    TW_FLAM,       // ratchet x2 on a lit step (skipped if unlit or already ratcheted)
};

enum TweakCond : uint8_t
{
    TW_ALWAYS,
    TW_IF_LIT,     // only if cond_step is lit
    TW_IF_UNLIT,   // only if cond_step is unlit
};

struct TweakEdit
{
    TweakOp op;
    int8_t step;
    TweakCond cond;
    int8_t cond_step;
    const char* why;   // the musical reason, for the harness and the design record
};

// ── The vocabulary ───────────────────────────────────────────────────────────
static const TweakEdit TWEAK_LISTS[LANES][TWEAK_MAX_EDITS] = {
    // BD — the downbeats are sacred; the ladder syncopates around them, then
    // drops the 3 for a breakbeat feel near the top.
    {
        { TW_ADD,      10, TW_ALWAYS,   0, "the & of 3: the classic house syncopation" },
        { TW_ADD_SOFT, 15, TW_ALWAYS,   0, "a ghost pickup on the a of 4, into the 1" },
        { TW_ADD,       6, TW_ALWAYS,   0, "the & of 2" },
        { TW_ADD,       3, TW_IF_LIT,   4, "the a of 1, only as a pickup into a kick on the 2" },
        { TW_FLAM,      0, TW_ALWAYS,   0, "double kick on the 1" },
        { TW_ADD,      13, TW_ALWAYS,   0, "the e of 4: a stumble before the pickup" },
        { TW_REMOVE,    8, TW_ALWAYS,   0, "drop the 3: the breakbeat hole" },
        { TW_ADD,       7, TW_ALWAYS,   0, "the a of 2" },
        { TW_ADD,      11, TW_ALWAYS,   0, "the a of 3" },
        { TW_ADD,       2, TW_ALWAYS,   0, "the & of 1" },
    },
    // SD — ghosts around the backbeat first, then flams, then a fill.
    {
        { TW_ADD,      14, TW_ALWAYS,   0, "the & of 4: the pickup into the 1" },
        { TW_ADD_SOFT, 10, TW_ALWAYS,   0, "a ghost on the & of 3" },
        { TW_ADD_SOFT,  7, TW_IF_LIT,   4, "a ghost on the a of 2, answering a backbeat on the 2" },
        { TW_ADD_SOFT, 15, TW_IF_LIT,  14, "the a of 4: a two-note pickup once the & is there" },
        { TW_ADD_SOFT,  2, TW_ALWAYS,   0, "a ghost on the & of 1" },
        { TW_FLAM,     12, TW_IF_LIT,  12, "flam the 4" },
        { TW_ADD,      11, TW_ALWAYS,   0, "the a of 3: the fill starts" },
        { TW_ADD,      13, TW_ALWAYS,   0, "the e of 4: the fill grows" },
        { TW_FLAM,      4, TW_IF_LIT,   4, "flam the 2" },
        { TW_ADD,       9, TW_ALWAYS,   0, "the e of 3" },
    },
    // CP — sits with the snare; its ladder pushes and answers, and drops the
    // 4 near the top so the bar ends open.
    {
        { TW_ADD,      15, TW_ALWAYS,   0, "the a of 4: a clap pickup" },
        { TW_ADD,       6, TW_ALWAYS,   0, "the & of 2: an answer after the backbeat" },
        { TW_ADD,      10, TW_ALWAYS,   0, "the & of 3" },
        { TW_ADD_SOFT,  2, TW_ALWAYS,   0, "a ghost on the & of 1" },
        { TW_FLAM,      4, TW_IF_LIT,   4, "flam the 2" },
        { TW_REMOVE,   12, TW_ALWAYS,   0, "drop the 4: leave the bar open" },
        { TW_ADD,      14, TW_ALWAYS,   0, "the & of 4" },
        { TW_ADD,      11, TW_ALWAYS,   0, "the a of 3" },
        { TW_ADD,       7, TW_ALWAYS,   0, "the a of 2" },
    },
    // PERC — syncopation only, never on a beat: a rim pattern that builds
    // toward a clave.
    {
        { TW_ADD,       2, TW_ALWAYS,   0, "the & of 1" },
        { TW_ADD,      10, TW_ALWAYS,   0, "the & of 3" },
        { TW_ADD_SOFT,  6, TW_ALWAYS,   0, "a ghost on the & of 2" },
        { TW_ADD,      14, TW_ALWAYS,   0, "the & of 4" },
        { TW_ADD,       7, TW_ALWAYS,   0, "the a of 2" },
        { TW_ADD,      11, TW_ALWAYS,   0, "the a of 3" },
        { TW_ADD,       3, TW_ALWAYS,   0, "the a of 1" },
        { TW_ADD,      15, TW_ALWAYS,   0, "the a of 4" },
        { TW_REMOVE,   10, TW_ALWAYS,   0, "take the & of 3 back out: the pattern breathes" },
        { TW_ADD,       5, TW_ALWAYS,   0, "the e of 2" },
    },
    // CH — fills toward sixteenths from the END of the bar backwards, so the
    // busiest playing lands before the 1; on a lane already at sixteenths it
    // thins the offbeats instead.
    {
        { TW_ADD,      13, TW_ALWAYS,   0, "the e of 4: the fill starts" },
        { TW_ADD,      15, TW_ALWAYS,   0, "the a of 4" },
        { TW_ADD,      11, TW_ALWAYS,   0, "the a of 3" },
        { TW_ADD,       9, TW_ALWAYS,   0, "the e of 3" },
        { TW_ADD,       7, TW_ALWAYS,   0, "the a of 2" },
        { TW_ADD,       5, TW_ALWAYS,   0, "the e of 2" },
        { TW_ADD,       3, TW_ALWAYS,   0, "the a of 1" },
        { TW_ADD,       1, TW_ALWAYS,   0, "the e of 1: full sixteenths" },
        { TW_REMOVE,   14, TW_ALWAYS,   0, "open a gap on the & of 4" },
        { TW_REMOVE,    6, TW_ALWAYS,   0, "open a gap on the & of 2" },
        { TW_REMOVE,   10, TW_ALWAYS,   0, "and on the & of 3" },
        { TW_REMOVE,    2, TW_ALWAYS,   0, "and on the & of 1" },
        { TW_REMOVE,    5, TW_ALWAYS,   0, "thin the e of 2" },
        { TW_REMOVE,   13, TW_ALWAYS,   0, "thin the e of 4" },
        { TW_REMOVE,    9, TW_ALWAYS,   0, "thin the e of 3" },
        { TW_REMOVE,    1, TW_ALWAYS,   0, "thin the e of 1: back to a skipping eighth" },
    },
    // OH — lifts on the offbeats, then a long lift on the 4.
    {
        { TW_ADD,      10, TW_ALWAYS,   0, "the & of 3" },
        { TW_ADD,       2, TW_ALWAYS,   0, "the & of 1" },
        { TW_ADD,      15, TW_ALWAYS,   0, "the a of 4: a lift into the 1" },
        { TW_ADD,       6, TW_ALWAYS,   0, "the & of 2" },
        { TW_ADD,      14, TW_ALWAYS,   0, "the & of 4" },
        { TW_ADD,      12, TW_ALWAYS,   0, "open on the 4 itself: the long lift" },
        { TW_ADD,      11, TW_ALWAYS,   0, "the a of 3" },
        { TW_ADD,       7, TW_ALWAYS,   0, "the a of 2" },
        { TW_REMOVE,   10, TW_ALWAYS,   0, "close the & of 3 again" },
        { TW_ADD,       3, TW_ALWAYS,   0, "the a of 1" },
    },
    // AC — accents the backbeats, then the pickups the other lanes are
    // gaining, then the offbeats.
    {
        { TW_ADD,       4, TW_ALWAYS,   0, "accent the 2" },
        { TW_ADD,      12, TW_ALWAYS,   0, "accent the 4" },
        { TW_ADD,      14, TW_ALWAYS,   0, "accent the & of 4 pickup" },
        { TW_ADD,      10, TW_ALWAYS,   0, "accent the & of 3" },
        { TW_ADD,       6, TW_ALWAYS,   0, "accent the & of 2" },
        { TW_ADD_SOFT, 15, TW_ALWAYS,   0, "sometimes accent the a of 4" },
        { TW_ADD,       2, TW_ALWAYS,   0, "accent the & of 1" },
        { TW_REMOVE,    8, TW_ALWAYS,   0, "un-accent the 3: lean the bar" },
        { TW_ADD,      11, TW_ALWAYS,   0, "accent the a of 3" },
        { TW_ADD,       7, TW_ALWAYS,   0, "accent the a of 2" },
    },
};

// ── Applying it ──────────────────────────────────────────────────────────────

inline bool tweakCondition(const Memory& m, int lane, const TweakEdit& e)
{
    switch (e.cond)
    {
        case TW_IF_LIT:   return e.cond_step >= 0 && e.cond_step < m.length && m.at(lane, e.cond_step).on;
        case TW_IF_UNLIT: return e.cond_step >= 0 && e.cond_step < m.length && !m.at(lane, e.cond_step).on;
        default:          return true;
    }
}

// Try one edit on `m`. Returns true if it changed something.
inline bool tweakApply(Memory& m, int lane, const TweakEdit& e)
{
    if (e.op == TW_END) return false;
    if (e.step < 0 || e.step >= m.length) return false;
    if (!tweakCondition(m, lane, e)) return false;

    Step& st = m.at(lane, e.step);
    switch (e.op)
    {
        case TW_ADD:
            if (st.on) return false;
            st.on = true;
            st.chance = CHANCE_MAX;
            return true;
        case TW_ADD_SOFT:
            if (st.on) return false;
            st.on = true;
            st.chance = TWEAK_SOFT_CHANCE;
            return true;
        case TW_REMOVE:
            if (!st.on) return false;
            st.on = false;
            return true;
        case TW_FLAM:
            if (lane >= VOICES || !st.on || st.ratchet != 0) return false;
            st.ratchet = 1;
            return true;
        default:
            return false;
    }
}

// Tweak one lane of `m` in place to `level`. Returns the level reached (the
// list can run out on a very busy pattern).
inline int tweakLane(Memory& m, int lane, int level)
{
    if (lane < 0 || lane >= LANES) return 0;
    if (level > TWEAK_LEVELS) level = TWEAK_LEVELS;
    int applied = 0;
    for (int i = 0; i < TWEAK_MAX_EDITS && applied < level; i++)
    {
        const TweakEdit& e = TWEAK_LISTS[lane][i];
        if (e.op == TW_END) break;
        if (tweakApply(m, lane, e)) applied++;
    }
    return applied;
}

// The tweaked copy of `base`: every lane in `lanes` (bit l = lane l) taken to
// `level`; every other lane byte-identical. Level 0 returns `base`.
inline Memory tweakMemory(const Memory& base, uint8_t lanes, int level)
{
    Memory m = base;
    if (level <= 0) return m;
    for (int l = 0; l < LANES; l++)
        if (lanes & (1u << l)) tweakLane(m, l, level);
    return m;
}

// ── The CV ───────────────────────────────────────────────────────────────────
//
// 0..10 V over TWEAK_LEVELS + 1 levels (0..8), 1.25 V per level, with a
// little hysteresis either side of every boundary so a CV resting on one
// does not flicker between two ladders. `current` is the level in force;
// the result is the level the CV now asks for.
//
static const float TWEAK_VOLTS_PER_LEVEL = 10.f / (float)TWEAK_LEVELS;
static const float TWEAK_HYSTERESIS = 0.12f;

inline int tweakLevelFromVolts(float volts, int current)
{
    if (current < 0) current = 0;
    if (current > TWEAK_LEVELS) current = TWEAK_LEVELS;
    for (int guard = 0; guard < TWEAK_LEVELS + 1; guard++)
    {
        const float up   = ((float)current + 0.5f) * TWEAK_VOLTS_PER_LEVEL + TWEAK_HYSTERESIS;
        const float down = ((float)current - 0.5f) * TWEAK_VOLTS_PER_LEVEL - TWEAK_HYSTERESIS;
        if (current < TWEAK_LEVELS && volts >= up) { current++; continue; }
        if (current > 0 && volts <= down) { current--; continue; }
        break;
    }
    return current;
}

} // namespace vx_drum_sequencer
