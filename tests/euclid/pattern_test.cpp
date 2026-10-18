// Euclid engine: the Bresenham pattern (golden table, rotation, the clamps,
// the dirty cache, exhaustive hit counts) and the CV law.
//
//   g++ -std=c++11 -O2 -I ../../src -o pattern_test pattern_test.cpp && ./pattern_test
//
// The golden rows are the ones in docs/implementation_plans/euclid-migration.md
// §2, computed from euclid_recompute_pattern (euclid.c). E(5,8) is the row
// that tells Bresenham from Bjorklund.

#include "modules/Euclid/EuclidEngine.hpp"
#include "check.hpp"

#include <cstdio>
#include <string>

using namespace euclid;

// The pattern as text, 'x' for a hit and '.' for a rest, `steps` wide.
static std::string render(const Pattern& p)
{
    std::string s;
    for (int i = 0; i < p.steps; i++) s += p.hit[i] ? 'x' : '.';
    return s;
}

static void golden(int h, int s, int r, const char* want)
{
    Pattern p;
    p.compute(h, s, r);
    const std::string got = render(p);
    char label[160];
    std::snprintf(label, sizeof(label), "E(%d,%d) rot %d is %s (got %s)", h, s, r, want, got.c_str());
    check(label, got == want);
}

static void goldenTests()
{
    std::printf("golden table\n");
    golden(3, 8, 0, "x..x..x.");
    golden(5, 8, 0, "x.x.xx.x");             // Bresenham; Bjorklund would give x.xx.xx.
    golden(4, 16, 0, "x...x...x...x...");
    golden(5, 16, 0, "x...x..x..x..x..");
    golden(3, 8, 1, "..x..x.x");             // increasing ROT pulls hits to earlier indices

    Pattern p;
    p.compute(5, 8, 0);
    check("E(5,8) is not the Bjorklund rotation", render(p) != "x.xx.xx.");

    // The remaining rotations of E(3,8), by hand from base = x..x..x.
    golden(3, 8, 2, ".x..x.x.");
    golden(3, 8, 3, "x..x.x..");
    golden(3, 8, 7, ".x..x..x");
}

static void rotationTests()
{
    std::printf("rotation\n");
    Pattern a, b;
    a.compute(3, 8, 31);
    b.compute(3, 8, 7);
    check("rot 31 on 8 steps is rot 7", render(a) == render(b));
    checkEq("the cached rot is the normalised one", a.rot, 7);

    a.compute(3, 8, 8);
    b.compute(3, 8, 0);
    check("rot == steps is rot 0", render(a) == render(b));

    a.compute(3, 8, -1);
    b.compute(3, 8, 7);
    check("a negative rot is brought into range", render(a) == render(b));
    checkEq("rot -1 on 8 steps caches as 7", a.rot, 7);

    // Rotation is the cyclic shift pattern[i] = base[(i + rot) % s], for every (h, s, r).
    bool shift_ok = true;
    for (int s = 1; s <= MAX_STEPS && shift_ok; s++)
    {
        for (int h = 0; h <= s && shift_ok; h++)
        {
            Pattern base;
            base.compute(h, s, 0);
            for (int r = 0; r < s && shift_ok; r++)
            {
                Pattern rotated;
                rotated.compute(h, s, r);
                for (int i = 0; i < s; i++)
                {
                    if (rotated.hit[i] != base.hit[(i + r) % s]) { shift_ok = false; break; }
                }
            }
        }
    }
    check("every rotation is the cyclic shift base[(i + rot) % s]", shift_ok);
}

static void clampTests()
{
    std::printf("clamps\n");
    Pattern p;

    p.compute(40, 8, 0);
    check("hits > steps gives all hits", render(p) == "xxxxxxxx");
    checkEq("hits > steps counts steps hits", p.count(), 8);
    checkEq("hits > steps caches hits = steps", p.hits, 8);

    p.compute(8, 8, 3);
    check("hits == steps gives all hits whatever the rotation", render(p) == "xxxxxxxx");

    p.compute(0, 8, 0);
    check("hits 0 gives all rests", render(p) == "........");
    checkEq("hits 0 counts none", p.count(), 0);

    p.compute(-3, 8, 0);
    checkEq("a negative hits is clamped to 0", p.hits, 0);
    checkEq("and gives all rests", p.count(), 0);

    p.compute(1, 1, 0);
    check("steps 1, hits 1 is x", render(p) == "x");
    p.compute(0, 1, 0);
    check("steps 1, hits 0 is .", render(p) == ".");
    p.compute(5, 1, 0);
    check("steps 1, hits 5 is x", render(p) == "x");
    p.compute(1, 1, 31);
    check("steps 1 ignores the rotation", render(p) == "x");

    p.compute(4, 0, 0);
    checkEq("steps 0 is clamped to 1", p.steps, 1);
    p.compute(4, -5, 0);
    checkEq("a negative steps is clamped to 1", p.steps, 1);
    p.compute(4, 40, 0);
    checkEq("steps 40 is clamped to MAX_STEPS", p.steps, MAX_STEPS);
    checkEq("and keeps the 4 hits", p.count(), 4);

    // hit[] is false beyond steps, whatever was there before.
    p.compute(32, 32, 0);
    p.compute(3, 8, 0);
    bool beyond_clear = true;
    for (int i = 8; i < MAX_STEPS; i++) if (p.hit[i]) beyond_clear = false;
    check("hit[] is false beyond steps after shrinking from 32", beyond_clear);
}

static void exhaustiveTests()
{
    std::printf("exhaustive (h, s) in 0..32 x 1..32\n");
    bool counts_ok = true;
    bool beyond_ok = true;
    bool first_ok = true;
    for (int h = 0; h <= MAX_STEPS; h++)
    {
        for (int s = 1; s <= MAX_STEPS; s++)
        {
            Pattern p;
            p.compute(h, s, 0);
            const int want = h < s ? h : s;
            if (p.count() != want)
            {
                if (counts_ok) std::printf("  E(%d,%d) has %d hits, want %d\n", h, s, p.count(), want);
                counts_ok = false;
            }
            for (int i = s; i < MAX_STEPS; i++) if (p.hit[i]) beyond_ok = false;
            if (h > 0 && !p.hit[0]) first_ok = false;
        }
    }
    check("every (h, s) has exactly min(h, s) hits", counts_ok);
    check("hit[] is false beyond steps for every (h, s)", beyond_ok);
    check("step 1 is a hit whenever hits > 0 at rot 0 (base[0] = 1)", first_ok);
}

static void cacheTests()
{
    std::printf("dirty cache\n");
    Pattern p;
    checkEq("a fresh Pattern has hits -1", p.hits, -1);
    checkEq("a fresh Pattern has steps -1", p.steps, -1);
    checkEq("a fresh Pattern has rot -1", p.rot, -1);
    checkEq("a fresh Pattern has no hits", p.count(), 0);

    check("the first compute recomputes", p.compute(4, 16, 0));
    check("a repeat does not", !p.compute(4, 16, 0));
    check("a hits change does", p.compute(5, 16, 0));
    check("a steps change does", p.compute(5, 12, 0));
    check("a rot change does", p.compute(5, 12, 1));
    check("a repeat after the changes does not", !p.compute(5, 12, 1));
    check("rot 13 on 12 steps is a repeat of rot 1", !p.compute(5, 12, 13));
    check("hits 40 on 12 steps recomputes (clamped to 12)", p.compute(40, 12, 1));
    check("hits 12 on 12 steps is then a repeat", !p.compute(12, 12, 1));
    checkEq("the cached triple is the clamped one: hits", p.hits, 12);
    checkEq("the cached triple is the clamped one: steps", p.steps, 12);
    checkEq("the cached triple is the clamped one: rot", p.rot, 1);
}

static void cvLawTests()
{
    std::printf("CV law\n");
    checkEq("1 V at atten 1 is +3 (3.2 truncated)", effectiveValue(4, 1.f, 1.f, 0, 32), 7);
    checkEq("-1 V at atten 1 is -3", effectiveValue(4, -1.f, 1.f, 0, 32), 1);
    checkEq("5 V at atten 1 is +16", effectiveValue(4, 5.f, 1.f, 0, 32), 20);
    checkEq("atten 0 silences the jack", effectiveValue(4, 5.f, 0.f, 0, 32), 4);
    checkEq("atten -1 inverts and the low clamp holds", effectiveValue(4, 5.f, -1.f, 0, 32), 0);
    checkEq("the high clamp holds", effectiveValue(16, 10.f, 1.f, 1, 32), 32);

    checkEq("0.3 V truncates to 0 (toward zero)", effectiveValue(4, 0.3f, 1.f, 0, 32), 4);
    checkEq("-0.5 V is -1, not floor's -2", effectiveValue(4, -0.5f, 1.f, 0, 32), 3);
    checkEq("atten 0.5 halves the CV before truncation", effectiveValue(4, 5.f, 0.5f, 0, 32), 12);
    checkEq("the STEPS range floor is 1", effectiveValue(1, -5.f, 1.f, 1, 32), 1);
    checkEq("the ROT range is 0..steps-1", effectiveValue(7, 5.f, 1.f, 0, 7), 7);
    checkEq("no CV, no atten: the knob", effectiveValue(9, 0.f, 0.f, 0, 32), 9);
}

int main()
{
    goldenTests();
    rotationTests();
    clampTests();
    exhaustiveTests();
    cacheTests();
    cvLawTests();
    return finish("pattern_test");
}
