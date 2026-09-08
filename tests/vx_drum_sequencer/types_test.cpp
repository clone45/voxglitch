// VX Drum Sequencer data model: the Step / Memory / Bank types, elementwise
// equality, the defaults a fresh or cleared memory carries, and SEED.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o types_test types_test.cpp && ./types_test

#include "VXDrumSequencerTypes.hpp"
#include "check.hpp"

using namespace vx_drum_sequencer;

static void defaults()
{
    std::printf("defaults\n");
    Memory m;
    bool all_default = true;
    for (int l = 0; l < LANES; l++)
        for (int s = 0; s < STEPS; s++)
        {
            const Step& st = m.at(l, s);
            if (st.on || st.ratchet != 0 || st.chance != CHANCE_MAX) all_default = false;
        }
    check("a fresh memory: every pad off, single, chance 100", all_default);
    checkEq("length 16", m.length, STEPS);
    checkEq("a single pad plays once", Step().hits(), 1);
    Step r; r.ratchet = 3;
    checkEq("ratchet 3 is four hits", r.hits(), 4);

    m.at(2, 5).on = true;
    m.at(2, 5).ratchet = 2;
    m.at(2, 5).chance = 40;
    m.length = 9;
    m.clear();
    check("clear() is a fresh memory", m == Memory());
}

static void equality()
{
    std::printf("equality\n");
    Memory a, b;
    check("two fresh memories are equal", a == b);
    b.at(3, 7).on = true;
    check("a pad differs", a != b);
    a.at(3, 7).on = true;
    check("and matches again", a == b);
    b.at(3, 7).chance = 99;
    check("chance participates", a != b);
    a.at(3, 7).chance = 99;
    b.at(3, 7).ratchet = 1;
    check("ratchet participates", a != b);
    a.at(3, 7).ratchet = 1;
    b.length = 15;
    check("length participates", a != b);

    Bank x, y;
    check("two fresh banks are equal", x == y);
    y.memories[15].at(ACCENT_LANE, 15).on = true;
    check("the last pad of the last memory differs", x != y);
    x.memories[15].at(ACCENT_LANE, 15).on = true;
    check("and matches again", x == y);
}

static void seed()
{
    std::printf("seed\n");
    Bank b;
    b.memories[1].at(0, 0).on = true;   // dirt that seedBank must clear
    seedBank(b);
    const Memory& m = b.memories[0];

    bool ok = true;
    for (int s = 0; s < STEPS; s++)
    {
        if (m.at(0, s).on != ((s % 4) == 0)) ok = false;                 // BD 1 5 9 13
        if (m.at(1, s).on) ok = false;                                   // SD empty
        if (m.at(2, s).on != (s == 4 || s == 12)) ok = false;            // CP 5 13
        if (m.at(3, s).on) ok = false;                                   // PERC empty
        if (m.at(4, s).on != ((s % 2) == 0)) ok = false;                 // CH eighths
        if (m.at(5, s).on != (s == 6 || s == 14)) ok = false;            // OH 7 15
        if (m.at(ACCENT_LANE, s).on != (s == 0 || s == 8)) ok = false;   // AC 1 9
    }
    check("SEED matches vxdrums.js:80-87", ok);
    checkEq("SEED length 16", m.length, STEPS);

    bool rest_empty = true;
    for (int i = 1; i < SLOTS; i++) if (b.memories[i] != Memory()) rest_empty = false;
    check("every other memory is empty", rest_empty);
}

int main()
{
    defaults();
    equality();
    seed();
    return finish("types_test");
}
