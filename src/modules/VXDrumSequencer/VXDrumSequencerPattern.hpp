#pragma once
//
// VX Drum Sequencer — the Rack-bound pattern helpers: the Random beat
// generator (rack::random) and the JSON shape of a memory, shared by the
// patch, the clipboard and the pattern files (jansson). The data types
// themselves (Step, Memory, Bank, the grid constants, SEED) are in
// VXDrumSequencerTypes.hpp, which is Rack-free.
//
// Source of truth: vxdrums-random.js (Random), memory-slots.js (clipboard).
//
// JSON, format 1.1.0 (2026-09-07, the per-pad model):
//
//   memory: { "length": 16,
//             "steps": [ { "on": [0|1 x16], "ratchet": [0..3 x16], "chance": [0..100 x16] }, x7 ] }
//
// one object per lane, three arrays of sixteen, so a memory reads the way the
// grid does. The 1.0.0 shape — "lanes": [7 bit-mask words], "ratchets": [6
// two-bit-per-step words] — is still READ (legacyMasksFromJson) so every
// patch and pattern file saved before the change loads; it is never written.
// Every loader bounds its loops (arrays capped at 7 / 16, values clamped), so
// a hand-edited file can neither allocate without limit nor index past the
// grid, and probes every key so a missing one keeps the default (a memory
// with no "chance" arrays plays every pad at 100).
//
// This header is included from the module TU after plugin.hpp (jansson and
// rack::random come from there), in the house one-TU-per-module pattern
// (src/modules/PianoRoll.cpp:21-29).
//

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "VXDrumSequencerTypes.hpp"

namespace vx_drum_sequencer
{
    // The "format" tag that stamps a memory on the OS clipboard, so a
    // DigitalSequencer clip (format "voxglitch-sequence") is rejected on paste
    // (brief-rack-sdk-api §12).
    static const char* const MEMORY_FORMAT = "voxglitch_vx_drum_memory";

    // vxdrums-random.js:19 — `const chance = (p) => R() < p;`
    inline bool roll(float p) { return rack::random::uniform() < p; }

    // Random beat generator — vxdrums-random.js EXACTLY, in the source's draw
    // order: every per-lane probability, the forced BD on step 1, the ratchet
    // seasoning. Replaces each chosen lane's pads (on, ratchet) and resets
    // their chance to 100 — a new beat starts certain; KEEPS the length (the
    // source patch carries no `len` key, :59-62). The caller wraps it in one
    // undo step.
    //
    // `lanes` is a bitmask of the lanes to touch, bit l = lane l, bit 6 = the
    // accent lane (Bret, 2026-09-07: a user asked to re-roll one drum at a
    // time). A lane whose bit is clear keeps every pad it has. The draw order
    // is unchanged whatever the mask, so re-rolling one lane draws exactly
    // what it would have drawn in a full randomize with the same seed.
    inline void randomizeMemory(Memory& m, uint8_t lanes = 0x7F)
    {
        const bool touch[LANES] = {
            (lanes & (1u << 0)) != 0u, (lanes & (1u << 1)) != 0u, (lanes & (1u << 2)) != 0u,
            (lanes & (1u << 3)) != 0u, (lanes & (1u << 4)) != 0u, (lanes & (1u << 5)) != 0u,
            (lanes & (1u << ACCENT_LANE)) != 0u,
        };

        Memory before = m;
        const int length = m.length;
        m.clear();
        m.length = length;
        for (int l = 0; l < LANES; l++)
            if (!touch[l])
                for (int s = 0; s < STEPS; s++) m.at(l, s) = before.at(l, s);

        for (int s = 0; s < STEPS; s++)                                  // :25-46
        {
            const bool onBeat = (s % 4) == 0;                             // 0 4 8 12
            const bool backbeat = (s == 4 || s == 12);
            const bool offEighth = (s % 4) == 2;                          // 2 6 10 14

            // Every lane draws whether or not it is kept, so the random
            // stream does not shift with the mask; the result is discarded
            // for a lane the user left unticked.
            // BD — owns the downbeats, almost nothing between, an occasional 15 pickup
            const bool bd = onBeat ? roll(0.86f) : roll(s == 15 ? 0.18f : 0.05f);            // :31
            // SD — the backbeat, sparse ghosts elsewhere
            const bool sd = backbeat ? roll(0.72f) : roll(0.06f);                            // :33
            // CP — mostly with the backbeat, occasionally its own answer
            const bool cp = backbeat ? roll(0.45f) : roll(0.03f);                            // :35
            // PERC (percussion; rimshot in the House kit) — syncopation only
            const bool perc = !onBeat && roll(0.11f);                                        // :37
            // CH — eighth floor + sixteenth fills
            const bool ch = ((s % 2) == 0) ? roll(0.82f) : roll(0.24f);                      // :39
            // OH — the offbeats
            const bool oh = offEighth ? roll(0.55f) : roll(0.04f);                           // :41
            // AC — the 1 and the 3 hard, the backbeats sometimes, a push rarely
            bool ac = false;
            if (s == 0 || s == 8)  ac = roll(0.62f);                                         // :43
            else if (backbeat)     ac = roll(0.3f);                                          // :44
            else if ((s % 4) == 3) ac = roll(0.1f);                                          // :45

            if (touch[0]) m.at(0, s).on = bd;
            if (touch[1]) m.at(1, s).on = sd;
            if (touch[2]) m.at(2, s).on = cp;
            if (touch[3]) m.at(3, s).on = perc;
            if (touch[4]) m.at(4, s).on = ch;
            if (touch[5]) m.at(5, s).on = oh;
            if (touch[ACCENT_LANE]) m.at(ACCENT_LANE, s).on = ac;
        }

        // "A beat with no kick on the 1 isn't a starting point — it's a mistake." (:48-49)
        if (touch[0]) m.at(0, 0).on = true;

        // A pinch of ratchet: an occasional x2 on a hat hit, rarely a x3 on a
        // snare hit late in the bar (:51-57).
        for (int s = 0; s < STEPS; s++)
        {
            const bool ch_ratchet = roll(0.08f);
            const bool sd_ratchet = (s >= 12) && roll(0.12f);
            if (touch[4] && m.at(4, s).on && ch_ratchet) m.at(4, s).ratchet = 1;             // :55  x2 on CH
            if (touch[1] && m.at(1, s).on && sd_ratchet) m.at(1, s).ratchet = 2;             // :56  x3 on SD
        }
    }

    // ── JSON ─────────────────────────────────────────────────────────────────
    // json_object_set_new / json_array_append_new EXCLUSIVELY (PianoRoll.hpp:711-717).

    // The grid of one memory: seven lane objects.
    inline json_t* stepsToJson(const Memory& m)
    {
        json_t* lanes = json_array();
        for (int l = 0; l < LANES; l++)
        {
            json_t* on = json_array();
            json_t* ratchet = json_array();
            json_t* chance = json_array();
            for (int s = 0; s < STEPS; s++)
            {
                const Step& st = m.at(l, s);
                json_array_append_new(on, json_integer(st.on ? 1 : 0));
                json_array_append_new(ratchet, json_integer(st.ratchet));
                json_array_append_new(chance, json_integer(st.chance));
            }
            json_t* lane = json_object();
            json_object_set_new(lane, "on", on);
            json_object_set_new(lane, "ratchet", ratchet);
            json_object_set_new(lane, "chance", chance);
            json_array_append_new(lanes, lane);
        }
        return lanes;
    }

    // One lane object's three arrays into lane `l` of `m`. Missing or short
    // arrays leave the defaults (off, single, 100).
    inline void laneFromJson(json_t* lane, int l, Memory& m)
    {
        if (!json_is_object(lane)) return;

        json_t* on = json_object_get(lane, "on");
        if (json_is_array(on))
        {
            const size_t n = std::min(json_array_size(on), (size_t)STEPS);
            for (size_t s = 0; s < n; s++)
            {
                json_t* v = json_array_get(on, s);
                if (json_is_integer(v)) m.at(l, (int)s).on = json_integer_value(v) != 0;
                else if (json_is_boolean(v)) m.at(l, (int)s).on = json_boolean_value(v);
            }
        }

        json_t* ratchet = json_object_get(lane, "ratchet");
        if (json_is_array(ratchet))
        {
            const size_t n = std::min(json_array_size(ratchet), (size_t)STEPS);
            for (size_t s = 0; s < n; s++)
            {
                json_t* v = json_array_get(ratchet, s);
                if (json_is_integer(v))
                    m.at(l, (int)s).ratchet = (uint8_t)rack::math::clamp((int)json_integer_value(v), 0, RATCHET_MAX);
            }
        }

        json_t* chance = json_object_get(lane, "chance");
        if (json_is_array(chance))
        {
            const size_t n = std::min(json_array_size(chance), (size_t)STEPS);
            for (size_t s = 0; s < n; s++)
            {
                json_t* v = json_array_get(chance, s);
                if (json_is_integer(v))
                    m.at(l, (int)s).chance = (uint8_t)rack::math::clamp((int)json_integer_value(v), 0, CHANCE_MAX);
            }
        }
    }

    // Format 1.1.0: the "steps" array.
    inline void stepsFromJson(json_t* lanes, Memory& m)
    {
        if (!json_is_array(lanes)) return;
        const size_t n = std::min(json_array_size(lanes), (size_t)LANES);
        for (size_t l = 0; l < n; l++) laneFromJson(json_array_get(lanes, l), (int)l, m);
    }

    // Format 1.0.0: "lanes" = 7 bit-mask words (bit s = step s), "ratchets" =
    // 6 words of two bits per step (0..3 extra hits). Read only.
    inline void legacyMasksFromJson(json_t* lanes, json_t* ratchets, Memory& m)
    {
        if (json_is_array(lanes))
        {
            const size_t n = std::min(json_array_size(lanes), (size_t)LANES);
            for (size_t l = 0; l < n; l++)
            {
                json_t* v = json_array_get(lanes, l);
                if (!json_is_integer(v)) continue;
                const uint32_t mask = (uint32_t)json_integer_value(v);
                for (int s = 0; s < STEPS; s++) m.at((int)l, s).on = ((mask >> s) & 1u) != 0u;
            }
        }
        if (json_is_array(ratchets))
        {
            const size_t n = std::min(json_array_size(ratchets), (size_t)VOICES);
            for (size_t l = 0; l < n; l++)
            {
                json_t* v = json_array_get(ratchets, l);
                if (!json_is_integer(v)) continue;
                const uint32_t word = (uint32_t)json_integer_value(v);
                for (int s = 0; s < STEPS; s++) m.at((int)l, s).ratchet = (uint8_t)((word >> (s * 2)) & 3u);
            }
        }
    }

    // A whole memory's body ("length" plus the grid in either format) into
    // `m`, which the caller has already defaulted.
    inline void memoryBodyFromJson(json_t* j, Memory& m)
    {
        json_t* steps = json_object_get(j, "steps");
        if (json_is_array(steps))
        {
            stepsFromJson(steps, m);
        }
        else
        {
            legacyMasksFromJson(json_object_get(j, "lanes"), json_object_get(j, "ratchets"), m);
        }

        json_t* length = json_object_get(j, "length");
        if (json_is_integer(length)) m.length = rack::math::clamp((int)json_integer_value(length), 1, STEPS);
    }

    inline void memoryBodyToJson(json_t* j, const Memory& m)
    {
        json_object_set_new(j, "steps", stepsToJson(m));
        json_object_set_new(j, "length", json_integer(m.length));
    }

    // Clipboard / pattern file shape (DESIGN §4.1):
    //   {"format":"voxglitch_vx_drum_memory","steps":[7 lane objects],"length":n}
    inline json_t* memoryToJson(const Memory& m)
    {
        json_t* root = json_object();
        json_object_set_new(root, "format", json_string(MEMORY_FORMAT));
        memoryBodyToJson(root, m);
        return root;
    }

    // Validates the format tag and that a grid is present in either format.
    // Returns false (and leaves `out` untouched) on anything that is not a VX
    // drum memory, so the memory button can grey out Paste.
    inline bool memoryFromJson(json_t* j, Memory& out)
    {
        if (!j || !json_is_object(j)) return false;

        json_t* format = json_object_get(j, "format");
        if (!json_is_string(format)) return false;
        if (std::strcmp(json_string_value(format), MEMORY_FORMAT) != 0) return false;

        const bool has_steps = json_is_array(json_object_get(j, "steps"));
        const bool has_masks = json_is_array(json_object_get(j, "lanes")) && json_is_array(json_object_get(j, "ratchets"));
        if (!has_steps && !has_masks) return false;

        Memory m;   // every pad off, single, 100; length 16: a short or sparse array leaves the rest default
        memoryBodyFromJson(j, m);
        out = m;
        return true;
    }
}
