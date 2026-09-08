// VX Drum Sequencer TWEAK: the ladder's invariants and the CV quantiser.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o tweak_test tweak_test.cpp && ./tweak_test

#include "VXDrumSequencerTweak.hpp"
#include "check.hpp"

using namespace vx_drum_sequencer;

static int diffCount(const Memory& a, const Memory& b)
{
    int n = 0;
    for (int l = 0; l < LANES; l++)
        for (int s = 0; s < STEPS; s++)
            if (a.at(l, s) != b.at(l, s)) n++;
    return n;
}

static void ladder()
{
    std::printf("ladder\n");
    Bank b;
    seedBank(b);
    const Memory base = b.memories[0];

    check("level 0 is the base", tweakMemory(base, 0x7F, 0) == base);
    check("a negative level is the base", tweakMemory(base, 0x7F, -3) == base);
    check("an empty lane mask is the base at any level", tweakMemory(base, 0, 8) == base);
    check("determinism", tweakMemory(base, 0x7F, 5) == tweakMemory(base, 0x7F, 5));
    check("levels above the top clamp", tweakMemory(base, 0x7F, 99) == tweakMemory(base, 0x7F, TWEAK_LEVELS));

    // Each level differs from the one below by at most one pad per tweaked lane.
    bool one_per_level = true;
    for (int l = 0; l < LANES; l++)
    {
        Memory prev = base;
        for (int level = 1; level <= TWEAK_LEVELS; level++)
        {
            Memory m = tweakMemory(base, (uint8_t)(1u << l), level);
            if (diffCount(prev, m) > 1) one_per_level = false;
            prev = m;
        }
    }
    check("each level changes at most one pad of the lane", one_per_level);

    // An untouched lane is byte-identical.
    Memory only_sd = tweakMemory(base, (uint8_t)(1u << 1), 8);
    bool others_same = true;
    for (int l = 0; l < LANES; l++)
    {
        if (l == 1) continue;
        for (int s = 0; s < STEPS; s++) if (only_sd.at(l, s) != base.at(l, s)) others_same = false;
    }
    check("lanes outside the mask are untouched", others_same);
    check("length is kept", only_sd.length == base.length);

    // Steps past the length are never touched.
    Memory shortm = base;
    shortm.length = 8;
    Memory t = tweakMemory(shortm, 0x7F, 8);
    bool tail_untouched = true;
    for (int l = 0; l < LANES; l++)
        for (int s = 8; s < STEPS; s++) if (t.at(l, s) != shortm.at(l, s)) tail_untouched = false;
    check("steps past the length are untouched", tail_untouched);

    // The kick on the 1 survives every level of the kick ladder.
    bool kick_kept = true;
    for (int level = 0; level <= TWEAK_LEVELS; level++)
        if (!tweakMemory(base, 1u, level).at(0, 0).on) kick_kept = false;
    check("the kick on the 1 survives the whole ladder", kick_kept);

    // A soft add carries the soft chance; a plain add is 100.
    Memory sd = tweakMemory(base, (uint8_t)(1u << 1), 2);
    check("SD level 1 is a plain add at 100", sd.at(1, 14).on && sd.at(1, 14).chance == CHANCE_MAX);
    check("SD level 2 is a ghost at the soft chance", sd.at(1, 10).on && sd.at(1, 10).chance == TWEAK_SOFT_CHANCE);

    // Every list has at least TWEAK_LEVELS live entries, so an empty lane climbs the whole ladder.
    bool full = true;
    for (int l = 0; l < LANES; l++)
    {
        Memory empty;
        Memory top = tweakMemory(empty, (uint8_t)(1u << l), TWEAK_LEVELS);
        int n = 0;
        for (int s = 0; s < STEPS; s++) if (top.at(l, s).on || top.at(l, s).ratchet) n++;
        if (n < TWEAK_LEVELS - 2) full = false;   // flams and conditionals may not all apply on an empty lane
    }
    check("every lane's list reaches most of the ladder from empty", full);
}

static void quantiser()
{
    std::printf("quantiser\n");
    checkEq("0 V is level 0", tweakLevelFromVolts(0.f, 0), 0);
    checkEq("10 V is level 8", tweakLevelFromVolts(10.f, 0), 8);
    checkEq("a big jump lands in one call", tweakLevelFromVolts(5.f, 0), 4);
    checkEq("negative volts pin at 0", tweakLevelFromVolts(-3.f, 5), 0);
    checkEq("over-range pins at 8", tweakLevelFromVolts(14.f, 2), 8);

    // Hysteresis: sitting exactly on the 1|2 boundary (1.875 V) does not flicker.
    int level = tweakLevelFromVolts(1.5f, 0);
    checkEq("1.5 V is level 1", level, 1);
    level = tweakLevelFromVolts(1.875f, level);
    checkEq("at the boundary it holds 1", level, 1);
    level = tweakLevelFromVolts(1.875f + 0.2f, level);
    checkEq("clearly past it goes to 2", level, 2);
    level = tweakLevelFromVolts(1.875f, level);
    checkEq("back at the boundary it holds 2", level, 2);
    level = tweakLevelFromVolts(1.875f - 0.2f, level);
    checkEq("clearly below it returns to 1", level, 1);
}

int main()
{
    ladder();
    quantiser();
    return finish("tweak_test");
}
