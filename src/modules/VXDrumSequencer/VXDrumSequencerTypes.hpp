#pragma once
//
// VX Drum Sequencer — the pattern data TYPES, Rack-free.
//
// The grid constants, one pad (Step), one pattern Memory, the Bank of
// sixteen, and the SEED pattern a fresh module grooves on. Nothing here
// touches Rack or jansson, so the sequencing engine (VXDrumSequencerEngine.hpp)
// and the tests in tests/vx_drum_sequencer/ can include it standalone. The
// Rack-bound helpers (the Random beat generator, the JSON shapes) live in
// VXDrumSequencerPattern.hpp, which includes this.
//
// THE MODEL (2026-09-07, owner decision): one Step struct per pad, in a
// LANES x STEPS array. The vxsynth source packs the same information into
// bit masks (vxdrums.c:106-112: 7 lane words, 6 two-bit-per-step ratchet
// words); the port shipped that way and now unpacks it, so a pad's fields sit
// together and a new per-pad attribute (chance) is a field, not a fourth
// word to shift. The packed form survives only as the 1.0.0 JSON, which the
// loaders still read (VXDrumSequencerPattern.hpp, legacyMasksFromJson).
//
// Equality is elementwise (operator== below). A Bank compare walks 16 x 7 x 16
// pads, three bytes each: trivially cheap, and it needs no padding argument.
//
// Identity rule: lanes, steps and slots are fixed-size indices of a fixed grid
// (structure, not identity). LANE_NAMES is display only and is never a key.
//

#include <cstdint>

namespace vx_drum_sequencer
{
    static const int SLOTS = 16;         // pattern memories
    static const int STEPS = 16;         // steps per memory
    static const int LANES = 7;          // 6 voices + the accent lane
    static const int VOICES = 6;         // BD SD CP PERC CH OH
    static const int ACCENT_LANE = 6;    // the accent lane's index / mute bit

    static const int RATCHET_MAX = 3;    // extra hits per pad: x1 .. x4
    static const int CHANCE_MAX = 100;   // percent

    // Display only — never matched on, never persisted as a key.
    static const char* const LANE_NAMES[LANES] = {"BD", "SD", "CP", "PERC", "CH", "OH", "AC"};

    // One pad of the grid: the (lane, step) cell.
    struct Step
    {
        bool on = false;                 // lit
        uint8_t ratchet = 0;             // EXTRA hits when it fires, 0..RATCHET_MAX (x1 .. x4); the accent lane's is unused
        uint8_t chance = CHANCE_MAX;     // percent probability it fires when reached; 100 = always

        bool plays() const { return on; }
        int hits() const { return (int)ratchet + 1; }
    };

    inline bool operator==(const Step& a, const Step& b)
    {
        return a.on == b.on && a.ratchet == b.ratchet && a.chance == b.chance;
    }
    inline bool operator!=(const Step& a, const Step& b) { return !(a == b); }

    // One pattern memory = the grid of pads + its length. The kit knobs and
    // the mute mask are NOT part of a memory (memory-slots.js).
    struct Memory
    {
        Step steps[LANES][STEPS];
        int length = STEPS;              // 1..16

        Step& at(int lane, int step) { return steps[lane][step]; }
        const Step& at(int lane, int step) const { return steps[lane][step]; }

        // Every pad off, every ratchet single, every chance 100, length 16
        // (memory-slots.js:74-78). The same as a fresh Memory().
        void clear() { *this = Memory(); }
    };

    inline bool operator==(const Memory& a, const Memory& b)
    {
        if (a.length != b.length) return false;
        for (int l = 0; l < LANES; l++)
            for (int s = 0; s < STEPS; s++)
                if (a.steps[l][s] != b.steps[l][s]) return false;
        return true;
    }
    inline bool operator!=(const Memory& a, const Memory& b) { return !(a == b); }

    struct Bank
    {
        Memory memories[SLOTS];
    };

    inline bool operator==(const Bank& a, const Bank& b)
    {
        for (int s = 0; s < SLOTS; s++)
            if (a.memories[s] != b.memories[s]) return false;
        return true;
    }
    inline bool operator!=(const Bank& a, const Bank& b) { return !(a == b); }

    // SEED (memory 0 on a fresh module / Initialize): vxdrums.js:80-87.
    // Four-on-the-floor kick, clap backbeat, closed-hat eighths, open hat on the
    // offbeats of 2 and 4, accents on the downbeats. Every other memory is empty.
    inline void seedBank(Bank& b)
    {
        for (int s = 0; s < SLOTS; s++) b.memories[s] = Memory();

        Memory& m = b.memories[0];
        for (int s = 0; s < STEPS; s++)
        {
            m.at(0, s).on = (s % 4) == 0;                       // BD  steps 1,5,9,13
            m.at(2, s).on = (s == 4 || s == 12);                // CP  steps 5,13
            m.at(4, s).on = (s % 2) == 0;                       // CH  eighths
            m.at(5, s).on = (s == 6 || s == 14);                // OH  steps 7,15
            m.at(ACCENT_LANE, s).on = (s == 0 || s == 8);       // AC  steps 1,9
        }
        m.length = STEPS;
    }
}
