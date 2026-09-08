// VX Drum Sequencer trigger shaper: pulse length, the gap rule, and the ACC
// cut, as voltage tables. Each case is one paragraph of the comment above
// TriggerShaper in VXDrumSequencerEngine.hpp.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o shaper_test shaper_test.cpp && ./shaper_test

#include "VXDrumSequencerEngine.hpp"
#include "check.hpp"

using namespace vx_drum_sequencer;

static const uint32_t BD = 1u << 0;
static const uint32_t SD = 1u << 1;
static const uint32_t AC = 1u << ACCENT_LANE;

static void pulseLength()
{
    std::printf("pulse length\n");
    TriggerShaper s;
    float out[LANES];
    s.strike(BD);
    int high = 0;
    for (int i = 0; i < 20; i++)
    {
        s.tick(5, out);
        if (out[0] == 10.f) high++;
        else break;
    }
    checkEq("a strike is high for exactly pulse_len samples", high, 5);
    check("other lanes stay low", out[1] == 0.f && out[ACCENT_LANE] == 0.f);

    s.strike(SD);
    s.tick(0, out);
    check("pulse_len below 1 is clamped to 1", out[1] == 10.f);
    s.tick(0, out);
    check("and ends after one sample", out[1] == 0.f);
}

static void gapRule()
{
    std::printf("gap rule\n");
    TriggerShaper s;
    float out[LANES];

    // A strike on a lane that is still high: one sample at 0 V, then a fresh pulse.
    s.strike(BD);
    for (int i = 0; i < 3; i++) s.tick(10, out);
    check("BD is high mid-pulse", out[0] == 10.f);
    s.strike(BD);
    s.tick(10, out);
    check("the re-strike forces one sample at 0 V", out[0] == 0.f);
    s.tick(10, out);
    check("then rises again", out[0] == 10.f);
    int high = 1;
    for (int i = 0; i < 20; i++) { s.tick(10, out); if (out[0] == 10.f) high++; else break; }
    checkEq("the fresh pulse is a full pulse_len", high, 10);

    // The exact-boundary case: a strike landing pulse_len after the previous
    // one keys on was_high, not the countdown, so it still takes the gap.
    TriggerShaper t;
    t.strike(BD);
    for (int i = 0; i < 10; i++) t.tick(10, out);    // 10 samples high; pulse_remaining is now 0 but was_high is true
    check("last high sample of the pulse", out[0] == 10.f);
    t.strike(BD);
    t.tick(10, out);
    check("a strike exactly pulse_len later takes the gap", out[0] == 0.f);
    t.tick(10, out);
    check("and then rises", out[0] == 10.f);

    // The gap applies to the whole strike set: an accented strike on a lane
    // that is still high delays ACC too, so both rise on the same sample.
    TriggerShaper u;
    u.strike(BD);
    for (int i = 0; i < 3; i++) u.tick(10, out);
    u.strike(BD | AC);
    u.tick(10, out);
    check("gap: BD low", out[0] == 0.f);
    check("gap: ACC low too, though ACC itself was not high", out[ACCENT_LANE] == 0.f);
    u.tick(10, out);
    check("both rise together", out[0] == 10.f && out[ACCENT_LANE] == 10.f);

    // A strike on a lane that is LOW takes no gap even if another lane is high.
    TriggerShaper v;
    v.strike(BD);
    for (int i = 0; i < 3; i++) v.tick(10, out);
    v.strike(SD);
    v.tick(10, out);
    check("a strike on an idle lane rises immediately", out[1] == 10.f);
    check("and does not disturb the lane that was high", out[0] == 10.f);
}

static void accCut()
{
    std::printf("ACC cut\n");
    float out[LANES];

    // A long ACC gate from an accented strike is cut on the sample an
    // un-accented strike rises.
    TriggerShaper s;
    s.strike(BD | AC);
    for (int i = 0; i < 5; i++) s.tick(100, out);
    check("ACC gate is high from the accented strike", out[ACCENT_LANE] == 10.f);
    s.strike(SD);
    s.tick(100, out);
    check("the un-accented strike rises now", out[1] == 10.f);
    check("and ACC is cut on the same sample", out[ACCENT_LANE] == 0.f);

    // With a gap the cut waits one sample so the un-accented lanes and the
    // cut line up on the rise.
    TriggerShaper t;
    t.strike(BD | AC);
    for (int i = 0; i < 5; i++) t.tick(100, out);
    t.strike(BD);                       // BD is high, so the set takes the gap
    t.tick(100, out);
    check("gap sample: BD low", out[0] == 0.f);
    check("gap sample: ACC still high for this one sample", out[ACCENT_LANE] == 10.f);
    t.tick(100, out);
    check("rise: BD high", out[0] == 10.f);
    check("rise: ACC cut", out[ACCENT_LANE] == 0.f);

    // An accented set that took the gap on the previous sample still rises
    // with ACC high: the cut rule does not eat its own gate.
    TriggerShaper u;
    u.strike(BD | AC);
    for (int i = 0; i < 5; i++) u.tick(100, out);
    u.strike(BD | AC);                  // accented re-strike on a high lane: gap
    u.tick(100, out);
    check("gap sample", out[0] == 0.f && out[ACCENT_LANE] == 0.f);
    u.tick(100, out);
    check("accented set rises with ACC high", out[0] == 10.f && out[ACCENT_LANE] == 10.f);

    // The documented unresolvable case: an accented and an un-accented lane
    // rising on the same sample. The strike set carries the ACC bit, so the
    // un-accented lane does not cut it here; but a set WITHOUT the ACC bit
    // wins over a still-high gate. Pin the rule as implemented.
    TriggerShaper v;
    v.strike(BD | SD | AC);
    v.tick(100, out);
    check("a set with the ACC bit rises with ACC high", out[ACCENT_LANE] == 10.f);
    v.strike(SD);                       // SD is still high: the set takes the gap
    v.tick(100, out);
    check("gap sample: ACC keeps one sample", out[ACCENT_LANE] == 10.f && out[1] == 0.f);
    v.tick(100, out);
    check("a set without the ACC bit cuts ACC on its rise", out[ACCENT_LANE] == 0.f && out[1] == 10.f);

    // reset() drops everything.
    v.strike(BD);
    v.reset();
    v.tick(100, out);
    check("reset: nothing pending, nothing high", out[0] == 0.f && out[ACCENT_LANE] == 0.f);
}

int main()
{
    pulseLength();
    gapRule();
    accCut();
    return finish("shaper_test");
}
