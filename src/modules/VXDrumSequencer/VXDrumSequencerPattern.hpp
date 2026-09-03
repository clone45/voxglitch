#pragma once
//
// VX Drum Sequencer — pattern data.
//
// The sequencer half of the vxsynth "VX Drum Machine" port (the kit is VXDrums).
// This header is pure data: the grid constants, one pattern Memory, the Bank of
// sixteen, the SEED pattern a fresh module grooves on, the Random beat
// generator, and the clipboard JSON shape the memory buttons Copy/Paste.
//
// Source of truth for the shapes: vxdrums.c:106-112 (pattern/length/ratchet
// storage), vxdrums.js:80-87 (SEED), vxdrums-random.js (Random), memory-slots.js
// (what a memory IS: 7 lane masks + 6 ratchet words + length; the kit knobs and
// the mute mask are NOT part of a memory).
//
// Identity rule: lanes, steps and slots are fixed-size indices of a fixed grid
// (structure, not identity). LANE_NAMES is display only and is never a key.
//
// This header is included from the module TU after plugin.hpp (jansson and
// rack::random come from there), in the house one-TU-per-module pattern
// (src/modules/PianoRoll.cpp:21-29).
//

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace vx_drum_sequencer
{
    static const int SLOTS = 16;         // pattern memories
    static const int STEPS = 16;         // steps per memory (bit s = step s)
    static const int LANES = 7;          // 6 voices + the accent lane
    static const int VOICES = 6;         // BD SD CP PERC CH OH
    static const int ACCENT_LANE = 6;    // the accent lane's index / mute bit

    // Display only — never matched on, never persisted as a key.
    static const char* const LANE_NAMES[LANES] = {"BD", "SD", "CP", "PERC", "CH", "OH", "AC"};

    // The "format" tag that stamps a memory on the OS clipboard, so a
    // DigitalSequencer clip (format "voxglitch-sequence") is rejected on paste
    // (brief-rack-sdk-api §12).
    static const char* const MEMORY_FORMAT = "voxglitch_vx_drum_memory";

    // One pattern memory = 7 lane masks + 6 ratchet words + its length
    // (vxdrums.c:107-108, :112).
    struct Memory
    {
        uint32_t lanes[LANES] = {};      // bit s = step s (s 0..15); lane 6 = accent
        uint32_t ratchets[VOICES] = {};  // 2 bits per step: 0..3 EXTRA hits (1x .. 4x); the accent lane has none
        int length = STEPS;              // 1..16
    };

    struct Bank
    {
        Memory memories[SLOTS];
    };

    // Memory and Bank are compared with memcmp (DESIGN §4.7: "Skip the push when
    // before == after (memcmp)"). That is only sound without padding, which the
    // all-4-byte layout guarantees; pin it so a future field cannot silently
    // break the comparison.
    static_assert(sizeof(Memory) == (LANES + VOICES + 1) * 4, "Memory must be padding-free for memcmp");
    static_assert(sizeof(Bank) == SLOTS * sizeof(Memory), "Bank must be padding-free for memcmp");

    inline bool operator==(const Memory& a, const Memory& b) { return std::memcmp(&a, &b, sizeof(Memory)) == 0; }
    inline bool operator!=(const Memory& a, const Memory& b) { return !(a == b); }
    inline bool operator==(const Bank& a, const Bank& b) { return std::memcmp(&a, &b, sizeof(Bank)) == 0; }
    inline bool operator!=(const Bank& a, const Bank& b) { return !(a == b); }

    // SEED (memory 0 on a fresh module / Initialize): vxdrums.js:80-87.
    // Four-on-the-floor kick, clap backbeat, closed-hat eighths, open hat on the
    // offbeats of 2 and 4, accents on the downbeats. Every other memory is empty.
    inline void seedBank(Bank& b)
    {
        for (int s = 0; s < SLOTS; s++) b.memories[s] = Memory();

        Memory& m = b.memories[0];
        m.lanes[0] = 0x1111u;    // BD  steps 1,5,9,13   (1<<0 | 1<<4 | 1<<8 | 1<<12)
        m.lanes[2] = 0x1010u;    // CP  steps 5,13       (1<<4 | 1<<12)
        m.lanes[4] = 0x5555u;    // CH  eighths (even steps)
        m.lanes[5] = 0x4040u;    // OH  steps 7,15       (1<<6 | 1<<14)
        m.lanes[6] = 0x0101u;    // AC  steps 1,9        (1<<0 | 1<<8)
        m.length = STEPS;
    }

    // vxdrums-random.js:19 — `const chance = (p) => R() < p;`
    inline bool chance(float p) { return rack::random::uniform() < p; }

    // Random beat generator — vxdrums-random.js EXACTLY: every per-lane
    // probability, the forced BD on step 1, the ratchet seasoning. Replaces all
    // seven lane masks and all six ratchet words; KEEPS the length (the source
    // patch carries no `len` key, :59-62). The caller wraps it in one undo step.
    inline void randomizeMemory(Memory& m)
    {
        uint32_t masks[LANES] = {};

        for (int s = 0; s < STEPS; s++)                                  // :25-46
        {
            const bool onBeat = (s % 4) == 0;                             // 0 4 8 12
            const bool backbeat = (s == 4 || s == 12);
            const bool offEighth = (s % 4) == 2;                          // 2 6 10 14

            // BD — owns the downbeats, almost nothing between, an occasional 15 pickup
            if (onBeat ? chance(0.86f) : chance(s == 15 ? 0.18f : 0.05f)) masks[0] |= (1u << s);   // :31
            // SD — the backbeat, sparse ghosts elsewhere
            if (backbeat ? chance(0.72f) : chance(0.06f)) masks[1] |= (1u << s);                   // :33
            // CP — mostly with the backbeat, occasionally its own answer
            if (backbeat ? chance(0.45f) : chance(0.03f)) masks[2] |= (1u << s);                   // :35
            // PERC (percussion; rimshot in the House kit) — syncopation only
            if (!onBeat && chance(0.11f)) masks[3] |= (1u << s);                                    // :37
            // CH — eighth floor + sixteenth fills
            if ((s % 2) == 0 ? chance(0.82f) : chance(0.24f)) masks[4] |= (1u << s);               // :39
            // OH — the offbeats
            if (offEighth ? chance(0.55f) : chance(0.04f)) masks[5] |= (1u << s);                  // :41
            // AC — the 1 and the 3 hard, the backbeats sometimes, a push rarely
            if (s == 0 || s == 8) { if (chance(0.62f)) masks[6] |= (1u << s); }                     // :43
            else if (backbeat) { if (chance(0.3f)) masks[6] |= (1u << s); }                         // :44
            else if ((s % 4) == 3) { if (chance(0.1f)) masks[6] |= (1u << s); }                     // :45
        }

        // "A beat with no kick on the 1 isn't a starting point — it's a mistake." (:48-49)
        masks[0] |= 1u;

        // A pinch of ratchet: an occasional x2 on a hat hit, rarely a x3 on a
        // snare hit late in the bar (:51-57). 2 bits per step, packed.
        uint32_t rats[VOICES] = {};
        for (int s = 0; s < STEPS; s++)
        {
            if (((masks[4] >> s) & 1u) && chance(0.08f)) rats[4] |= (1u << (s * 2));               // :55  x2 on CH
            if (((masks[1] >> s) & 1u) && s >= 12 && chance(0.12f)) rats[1] |= (2u << (s * 2));    // :56  x3 on SD
        }

        for (int l = 0; l < LANES; l++) m.lanes[l] = masks[l];
        for (int l = 0; l < VOICES; l++) m.ratchets[l] = rats[l];
    }

    // Clipboard shape (DESIGN §4.1):
    //   {"format":"voxglitch_vx_drum_memory","lanes":[7],"ratchets":[6],"length":n}
    // json_object_set_new / json_array_append_new only (PianoRoll.hpp:711-717).
    inline json_t* memoryToJson(const Memory& m)
    {
        json_t* root = json_object();
        json_object_set_new(root, "format", json_string(MEMORY_FORMAT));

        json_t* lanes = json_array();
        for (int l = 0; l < LANES; l++) json_array_append_new(lanes, json_integer((json_int_t)m.lanes[l]));
        json_object_set_new(root, "lanes", lanes);

        json_t* ratchets = json_array();
        for (int l = 0; l < VOICES; l++) json_array_append_new(ratchets, json_integer((json_int_t)m.ratchets[l]));
        json_object_set_new(root, "ratchets", ratchets);

        json_object_set_new(root, "length", json_integer(m.length));
        return root;
    }

    // Validates the format tag, caps the arrays at 7/6, clamps length 1..16.
    // Returns false (and leaves `out` untouched) on anything that is not a VX
    // drum memory, so the memory button can grey out Paste.
    inline bool memoryFromJson(json_t* j, Memory& out)
    {
        if (!j || !json_is_object(j)) return false;

        json_t* format = json_object_get(j, "format");
        if (!json_is_string(format)) return false;
        if (std::strcmp(json_string_value(format), MEMORY_FORMAT) != 0) return false;

        json_t* lanes = json_object_get(j, "lanes");
        json_t* ratchets = json_object_get(j, "ratchets");
        if (!json_is_array(lanes) || !json_is_array(ratchets)) return false;

        Memory m;   // zero masks, length 16: a short or sparse array leaves the rest empty

        size_t lane_count = std::min(json_array_size(lanes), (size_t)LANES);
        for (size_t l = 0; l < lane_count; l++)
        {
            json_t* v = json_array_get(lanes, l);
            if (json_is_integer(v)) m.lanes[l] = (uint32_t)json_integer_value(v);
        }

        size_t ratchet_count = std::min(json_array_size(ratchets), (size_t)VOICES);
        for (size_t l = 0; l < ratchet_count; l++)
        {
            json_t* v = json_array_get(ratchets, l);
            if (json_is_integer(v)) m.ratchets[l] = (uint32_t)json_integer_value(v);
        }

        json_t* length = json_object_get(j, "length");
        if (json_is_integer(length)) m.length = rack::math::clamp((int)json_integer_value(length), 1, STEPS);

        out = m;
        return true;
    }
}
