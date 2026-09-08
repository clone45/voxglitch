// Prints the TWEAK ladder (levels 0..8) for a few base patterns, lane by
// lane, as text grids — the tool for tuning the vocabulary by eye.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o tweak_ladder tweak_ladder.cpp && ./tweak_ladder

#include <cstdio>
#include <cstring>
#include "VXDrumSequencerTweak.hpp"

using namespace vx_drum_sequencer;

// One lane as 16 characters: '.' off, 'x' on, 'o' on at reduced chance,
// 'X'/'O' with a ratchet. A '|' every four steps. Steps past the length are ' '.
static void row(const Memory& m, int lane, char* out)
{
    int k = 0;
    for (int s = 0; s < STEPS; s++)
    {
        if (s > 0 && (s % 4) == 0) out[k++] = '|';
        if (s >= m.length) { out[k++] = ' '; continue; }
        const Step& st = m.at(lane, s);
        char c = '.';
        if (st.on) c = (st.chance < CHANCE_MAX) ? 'o' : 'x';
        if (st.on && st.ratchet > 0) c = (c == 'o') ? 'O' : 'X';
        out[k++] = c;
    }
    out[k] = 0;
}

static void ladder(const char* title, const Memory& base, int lane)
{
    std::printf("\n%s  [%s]\n", title, LANE_NAMES[lane]);
    Memory prev = base;
    for (int level = 0; level <= TWEAK_LEVELS; level++)
    {
        Memory m = tweakMemory(base, (uint8_t)(1u << lane), level);
        char r[24];
        row(m, lane, r);

        // Find the reason: the edit that made this level differ from the last.
        const char* why = (level == 0) ? "the base pattern" : "(no further edit applies)";
        if (level > 0 && m != prev)
        {
            for (int i = 0; i < TWEAK_MAX_EDITS; i++)
            {
                const TweakEdit& e = TWEAK_LISTS[lane][i];
                if (e.op == TW_END) break;
                Memory t = prev;
                if (tweakApply(t, lane, e) && t == m) { why = e.why; break; }
            }
        }
        std::printf("  %d  %s   %s\n", level, r, why);
        prev = m;
    }
}

static Memory seed()
{
    Bank b;
    seedBank(b);
    return b.memories[0];
}

int main()
{
    std::printf("Legend: x hit  o ghost (%d%%)  X/O with flam  . rest  | beat\n", TWEAK_SOFT_CHANCE);

    // 1. The seed beat, every lane.
    {
        Memory m = seed();
        for (int l = 0; l < LANES; l++) ladder("SEED beat", m, l);
    }

    // 2. Bret's example: a bare 2-and-4 snare.
    {
        Memory m;
        m.at(1, 4).on = true;
        m.at(1, 12).on = true;
        ladder("Bare backbeat", m, 1);
    }

    // 3. A sparse kick: 1 and 3 only.
    {
        Memory m;
        m.at(0, 0).on = true;
        m.at(0, 8).on = true;
        ladder("Kick on 1 and 3", m, 0);
    }

    // 4. Hats already at sixteenths: the ladder must thin, not stall.
    {
        Memory m;
        for (int s = 0; s < STEPS; s++) m.at(4, s).on = true;
        ladder("Sixteenth hats", m, 4);
    }

    // 5. An empty lane.
    {
        Memory m;
        ladder("Empty snare lane", m, 1);
    }

    // 6. A 12-step pattern: steps 12..15 must be skipped.
    {
        Memory m = seed();
        m.length = 12;
        ladder("Seed at length 12", m, 0);
        ladder("Seed at length 12", m, 1);
    }
    return 0;
}
