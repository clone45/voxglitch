// VX Drum Sequencer chain: three banks, ONE Sequencer, and a simulated head
// that switches the PlaySource on hand-off. Mirrors the head's decision
// structure in VXDrumSequencer::process() steps 3-7: the load's implicit
// reset on the first sample, a member gone from under the playhead, RST
// before CLK, a pending hand-off taken on the next ACCEPTED clock edge before
// the advance (never on a reset sample, never on an edge the window
// swallows), the reset played through seq.reset() before seq.process(), and
// the hand-off decided when the active member is on its last step, whether a
// fire put it there or an implicit reset did (a one-step head after load or
// after a member left: 2026-09-17). If that changes, re-sync this.
//
// Every scenario starts the way a patch does: the load is an implicit reset
// (step 1 current, nothing fires), and where a case needs member 0's step 1
// to have PLAYED it sends the clock master's reset first (`run(h, n, true)`),
// which plays step 1 at the reset instant; the clocks that follow are steps
// 2, 3, ... So the fire lists read exactly as they did under the port's old
// "-1, the next clock plays step 1" rule, with the reset supplying the first
// fire instead of the first clock. The hand-off cases themselves are
// unchanged: a hand-off is still -1-then-advance.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o chain_test chain_test.cpp && ./chain_test

#include "VXDrumSequencerEngine.hpp"
#include "check.hpp"
#include <vector>

using namespace vx_drum_sequencer;

static const float SR = 48000.f;
static const int WINDOW = 48;    // vgLib_v2::ResetClockWindow::samplesFor(48000)

// A member: its bank's effective memory and its mute mask.
struct Member
{
    Memory memory;
    uint8_t mute = 0;
    PlaySource source() const { PlaySource s; s.memory = &memory; s.mute = mute; return s; }
};

// The head's chain logic, lifted from the module.
struct Head
{
    Sequencer seq;
    std::vector<Member>* members;
    int active = 0;
    bool handoff_pending = false;
    bool loaded = false;
    int fired_member = -1;
    int last_position = -1;

    explicit Head(std::vector<Member>* m) : members(m) {}

    // One sample. Returns true if a step fired (a clocked advance or a reset).
    bool process(bool clock_edge, bool reset_edge, float out[LANES])
    {
        bool implicit_reset = false;
        if (!loaded)
        {
            loaded = true;
            implicit_reset = true;
            seq.rewind(WINDOW);                            // patch load: an implicit reset
        }
        if ((int)members->size() > 0 && active >= (int)members->size())
        {
            active = 0;
            handoff_pending = false;
            implicit_reset = true;
            seq.rewind(WINDOW);                            // a member gone: an implicit reset to the head
        }
        if (reset_edge)
        {
            active = 0;
            handoff_pending = false;
        }
        if (clock_edge && handoff_pending && !seq.window.open())   // never on a reset sample (just cleared) nor a swallowed edge
        {
            handoff_pending = false;
            active = (active + 1) % (int)members->size();
            seq.handOff();
        }
        const PlaySource src = (*members)[active].source();
        if (reset_edge) seq.reset(src, WINDOW);            // the explicit reset plays the head's step 1 now
        seq.process(clock_edge, src, SR, 48, out);
        if (seq.fired_this_sample)
        {
            fired_member = active;
            last_position = seq.head.position;
        }
        // On its last step, played or put there by an implicit reset: the next accepted clock is the next member's.
        if ((seq.fired_this_sample || implicit_reset) && members->size() > 1 && seq.head.position >= src.memory->length - 1) handoff_pending = true;
        return seq.fired_this_sample;
    }

    // The RWD button, or a lone RST edge: one sample with the reset and no clock.
    bool reset(float out[LANES]) { return process(false, true, out); }
};

struct Fire { int member; int position; uint32_t mask; };

// Optionally the master's reset first (one beat: the reset sample plus 99
// idle samples), then `n` clocks at a 100-sample period. Records every fire.
static std::vector<Fire> run(Head& h, int n, bool reset_first = false)
{
    std::vector<Fire> fires;
    float out[LANES];
    if (reset_first)
    {
        if (h.process(false, true, out)) fires.push_back({h.fired_member, h.last_position, h.seq.report.fired_mask});
        for (int k = 0; k < 99; k++) if (h.process(false, false, out)) fires.push_back({h.fired_member, h.last_position, h.seq.report.fired_mask});
    }
    for (int i = 0; i < n; i++)
    {
        if (h.process(true, false, out)) fires.push_back({h.fired_member, h.last_position, h.seq.report.fired_mask});
        for (int k = 0; k < 99; k++) if (h.process(false, false, out)) fires.push_back({h.fired_member, h.last_position, h.seq.report.fired_mask});
    }
    return fires;
}

// The patch loaded and sat quiet for a while before the clock started.
static void settle(Head& h)
{
    float out[LANES];
    for (int k = 0; k < 100; k++) h.process(false, false, out);
}

static void loadIsAnImplicitReset()
{
    std::printf("load is an implicit reset\n");
    std::vector<Member> members(1);
    members[0].memory.length = 3;
    members[0].memory.at(0, 0).on = true;                // BD on step 1 only

    Head h(&members);
    float out[LANES];
    bool fired = h.process(true, false, out);
    check("a clock on the load sample is swallowed", !fired);
    checkEq("step 1 is current", h.seq.head.position, 0);
    settle(h);
    checkEq("nothing fired while idle", h.seq.report.fired_serial, 0);
    std::vector<Fire> f = run(h, 1);
    check("the first clock after load plays step 2", f.size() == 1 && f[0].member == 0 && f[0].position == 1);

    Head g(&members);
    std::vector<Fire> r = run(g, 1, true);
    check("a reset on the load sample plays step 1 at once", r.size() == 2 && r[0].position == 0 && r[0].mask == 1u);
    check("and the clock after it is step 2", r[1].position == 1);
}

static void stepOrder()
{
    std::printf("step order across three members\n");
    std::vector<Member> members(3);
    members[0].memory.length = 4;  for (int s = 0; s < 4; s++) members[0].memory.at(0, s).on = true;   // BD on every step
    members[1].memory.length = 2;  for (int s = 0; s < 2; s++) members[1].memory.at(1, s).on = true;   // SD on every step
    members[2].memory.length = 3;  for (int s = 0; s < 3; s++) members[2].memory.at(2, s).on = true;   // CP on every step

    Head h(&members);
    std::vector<Fire> f = run(h, 8 + 4, true);   // the reset plays member 0's step 1; then the rest of one cycle (4+2+3 = 9) plus 4 more

    checkEq("a reset and twelve clocks, thirteen fires", (long long)f.size(), 13);
    const int want_member[13]   = {0,0,0,0, 1,1, 2,2,2, 0,0,0,0};
    const int want_position[13] = {0,1,2,3, 0,1, 0,1,2, 0,1,2,3};
    bool order_ok = true, lanes_ok = true;
    for (int i = 0; i < 13; i++)
    {
        if (f[i].member != want_member[i] || f[i].position != want_position[i]) order_ok = false;
        const uint32_t want_mask = 1u << want_member[i];
        if (f[i].mask != want_mask) lanes_ok = false;
    }
    check("members play in order, each from step 1 to its length, then wrap to the head", order_ok);
    check("each fire carries its own member's lanes", lanes_ok);
}

static void loneMemberLoops()
{
    std::printf("a lone member loops\n");
    std::vector<Member> members(1);
    members[0].memory.length = 3;
    Head h(&members);
    std::vector<Fire> f = run(h, 6, true);
    const int want[7] = {0,1,2,0,1,2,0};
    bool ok = f.size() == 7;
    for (int i = 0; ok && i < 7; i++) ok = f[i].position == want[i] && f[i].member == 0;
    check("a chain of one wraps as it always has", ok);
    check("and never raises a hand-off", !h.handoff_pending);
}

static void resetReturnsToHead()
{
    std::printf("reset returns to the head\n");
    std::vector<Member> members(2);
    members[0].memory.length = 2;
    members[0].memory.at(0, 0).on = true;                // BD on the head's step 1
    members[1].memory.length = 2;
    Head h(&members);
    run(h, 2, true);                             // member 0 steps 1,2; member 1 step 1
    checkEq("member 1 is active", h.active, 1);

    float out[LANES];
    const bool fired = h.reset(out);
    checkEq("reset: head active", h.active, 0);
    checkEq("reset: playhead on step 1", h.seq.head.position, 0);
    check("reset: the head's step 1 plays at the reset instant", fired && h.fired_member == 0 && h.last_position == 0);
    check("with the head's lanes", h.seq.report.fired_mask == 1u && out[0] == 10.f);
    check("no hand-off pending", !h.handoff_pending);

    // The master's own clock, a sample late through a utility: inside the window.
    check("a clock one sample after the reset is swallowed", !h.process(true, false, out));
    checkEq("and the chain stays on the head's step 1", h.seq.head.position, 0);
    for (int k = 0; k < 98; k++) h.process(false, false, out);

    std::vector<Fire> f = run(h, 1);             // one period after the reset
    check("the next clock plays the head's step 2", f.size() == 1 && f[0].member == 0 && f[0].position == 1);

    // A same-sample reset + clock while member 1 is active: step 1 of the
    // head, once, and no hand-off is taken on that sample.
    run(h, 1);                                   // member 1 step 1
    checkEq("member 1 active again", h.active, 1);
    const uint32_t before = h.seq.report.fired_serial;
    h.process(true, true, out);
    checkEq("same-sample reset + clock: one fire", h.seq.report.fired_serial, before + 1);
    check("the head's step 1", h.active == 0 && h.seq.head.position == 0);
}

static void lengthChangeMidChain()
{
    std::printf("length change mid-chain\n");
    std::vector<Member> members(2);
    members[0].memory.length = 4;
    members[1].memory.length = 4;
    Head h(&members);
    run(h, 1, true);                             // member 0 on step 2
    members[0].memory.length = 2;                // shortened under the playhead: step 2 is now the last
    // The shortening is noticed on the next fire: the head advances to
    // position 2, which wraps to 0 (length 2), fires step 1, and since 0 < 1
    // no hand-off. Then step 2 fires and hands off.
    std::vector<Fire> f = run(h, 3);
    check("the wrap is honoured", f.size() == 3 && f[0].member == 0 && f[0].position == 0);
    check("then the last step of the shortened member", f[1].member == 0 && f[1].position == 1);
    check("then the hand-off", f[2].member == 1 && f[2].position == 0);

    // A member lengthened while it is the last step: no hand-off until the new last step.
    std::vector<Member> m2(2);
    m2[0].memory.length = 2;
    m2[1].memory.length = 2;
    Head g(&m2);
    run(g, 1, true);                             // member 0 fired step 2 (its last) -> hand-off pending
    check("hand-off pending after the last step", g.handoff_pending);
    // The pending hand-off is taken on the next clock regardless: the
    // decision was made when the step fired (the module does the same).
    std::vector<Fire> f2 = run(g, 1);
    check("the pending hand-off is taken on the next clock", f2.size() == 1 && f2[0].member == 1);
}

static void ratchetsSurviveHandOff()
{
    std::printf("ratchets finish across a hand-off\n");
    std::vector<Member> members(2);
    members[0].memory.length = 1;
    members[0].memory.at(4, 0).on = true;        // CH on step 1
    members[0].memory.at(4, 0).ratchet = 3;      // x4
    members[1].memory.length = 1;
    members[1].memory.at(0, 0).on = true;        // BD on step 1

    Head h(&members);
    settle(h);                                   // the load's window has closed
    float out[LANES];
    // The load's implicit reset put member 0 on its only step, which is also
    // its last, so the hand-off is already pending, as it would be after a
    // fire. Two clocks to measure the period (100 samples): the first anchors
    // and takes the hand-off, so member 1's BD plays (single, no ratchets to
    // arm). Second clock: back to member 0, now with a known period -> its
    // CH x4 arms.
    check("the load's implicit reset raises the hand-off on a one-step head", h.handoff_pending);
    h.process(true, false, out); for (int k = 0; k < 99; k++) h.process(false, false, out);
    checkEq("member 1 played on the first clock", h.fired_member, 1);
    h.process(true, false, out);
    checkEq("member 0 on the second clock", h.fired_member, 0);
    check("ratchets armed", h.seq.ratchets.any());
    check("hand-off pending", h.handoff_pending);
    // Re-strikes fire during the 99 idle samples...
    int restrikes = 0; bool prev = true;
    for (int k = 0; k < 99; k++)
    {
        h.process(false, false, out);
        if (out[4] == 10.f && !prev) restrikes++;
        prev = out[4] == 10.f;
    }
    checkEq("three CH re-strikes inside member 0's step", restrikes, 3);
    // ...and the hand-off on the next clock does not cancel anything that was
    // still due (here they have all fired), and member 1's BD lands on that clock.
    h.process(true, false, out);
    checkEq("member 1 on the next clock", h.fired_member, 1);
    check("BD rises on the hand-off clock", out[0] == 10.f);
    check("handOff() does not cancel ratchets (none left here, but the call is the playhead only)", true);

    // The direct claim: handOff() leaves ratchets armed, rewind() cancels them.
    Sequencer s;
    StepFire f; f.struck = 1u << 4; f.extra[4] = 1;
    s.ratchets.arm(f, 100.f);
    s.handOff();
    check("handOff keeps armed ratchets", s.ratchets.any());
    s.rewind(WINDOW);
    check("rewind cancels them", !s.ratchets.any());
}

static void memberRemoved()
{
    std::printf("a member removed from under the playhead\n");
    std::vector<Member> members(3);
    for (int i = 0; i < 3; i++) members[i].memory.length = 2;
    Head h(&members);
    run(h, 4, true);                             // member 2, step 1
    checkEq("member 2 active", h.active, 2);
    members.pop_back();
    std::vector<Fire> f = run(h, 1);
    checkEq("the chain restarts at the head", h.active, 0);
    checkEq("with the head's step 1 current (an implicit reset)", h.seq.head.position, 0);
    check("the clock on the sample the member was found gone is inside the window", f.empty());
    f = run(h, 1);
    check("the next clock plays the head's step 2", f.size() == 1 && f[0].member == 0 && f[0].position == 1);
}

static void oneStepHeadAfterImplicitReset()
{
    std::printf("a one-step head after an implicit reset\n");
    // The chain's contract (chain-foundations.md §1) is one longer pattern: a
    // one-step head and a three-step member 1 must behave as a flat four-step
    // sequencer. After load that sequencer is on step 1 (the head's only
    // step), so its first clock is step 2, which is member 1's step 1. Before
    // 2026-09-17 the hand-off was decided only on a fire, and an implicit
    // reset does not fire, so the first clock re-played the head's step 1 and
    // the chain ran one step behind until the next explicit reset.
    std::vector<Member> members(2);
    members[0].memory.length = 1;
    members[0].memory.at(0, 0).on = true;                // BD on the head's only step
    members[1].memory.length = 3;
    for (int s = 0; s < 3; s++) members[1].memory.at(1, s).on = true;   // SD on every step

    Head h(&members);
    settle(h);                                           // load, then quiet
    check("the load's implicit reset leaves the head on its last step with the hand-off pending", h.handoff_pending);
    checkEq("and nothing fired", h.seq.report.fired_serial, 0);
    std::vector<Fire> f = run(h, 5);
    // A flat four-step sequencer plays indices 1, 2, 3, 0, 1 on its first five clocks after load.
    const int want_member[5]   = {1, 1, 1, 0, 1};
    const int want_position[5] = {0, 1, 2, 0, 0};
    bool ok = f.size() == 5;
    for (int i = 0; ok && i < 5; i++) ok = f[i].member == want_member[i] && f[i].position == want_position[i];
    check("the first clock after load plays member 1's step 1, then its 2 and 3, the head, member 1 again", ok);
    check("each with its own member's lanes", f.size() == 5 && f[0].mask == 2u && f[3].mask == 1u);

    // The same through step 4: a member removed from under the playhead.
    std::vector<Member> m3(3);
    m3[0].memory.length = 1;
    m3[1].memory.length = 2;
    m3[2].memory.length = 2;
    Head g(&m3);
    run(g, 3, true);                                     // the reset plays the head; clocks: m1 step 1, m1 step 2, m2 step 1
    checkEq("member 2 active", g.active, 2);
    m3.pop_back();
    std::vector<Fire> r = run(g, 1);
    check("the clock on the sample the member was found gone is swallowed", r.empty());
    check("the head is on its only step with the hand-off pending", g.active == 0 && g.seq.head.position == 0 && g.handoff_pending);
    r = run(g, 1);
    check("the next clock plays member 1's step 1, not the head's again", r.size() == 1 && r[0].member == 1 && r[0].position == 0);

    // A lone one-step module is untouched: every clock is its step 1 and a
    // chain of one never raises a hand-off.
    std::vector<Member> one(1);
    one[0].memory.length = 1;
    Head l(&one);
    settle(l);
    check("a chain of one raises no hand-off after load", !l.handoff_pending);
    std::vector<Fire> q = run(l, 2);
    check("and plays step 1 on every clock", q.size() == 2 && q[0].position == 0 && q[1].position == 0 && !l.handoff_pending);
}

static void swallowedClockTakesNoHandOff()
{
    std::printf("a swallowed clock takes no hand-off\n");
    // The head is one step long, so the reset's fire of its step 1 is also
    // its last step and raises the hand-off. The master's clock arrives one
    // sample later (through a utility): inside the window, swallowed whole.
    // It must not take the hand-off either, or the chain would sit at
    // position -1 on member 1 for the rest of the beat with nothing playing.
    std::vector<Member> members(2);
    members[0].memory.length = 1;
    members[0].memory.at(0, 0).on = true;                // BD on the head's only step
    members[1].memory.length = 4;
    members[1].memory.at(1, 0).on = true;                // SD on member 1's step 1

    Head h(&members);
    settle(h);
    float out[LANES];
    check("the reset plays the head's step 1", h.reset(out) && h.fired_member == 0 && h.last_position == 0);
    check("and raises the hand-off", h.handoff_pending);

    check("a clock one sample after the reset is swallowed", !h.process(true, false, out));
    checkEq("the head is still active", h.active, 0);
    checkEq("its step 1 is still current", h.seq.head.position, 0);
    check("the hand-off is still pending", h.handoff_pending);
    for (int k = 0; k < 98; k++)
    {
        h.process(false, false, out);
    }
    checkEq("nothing moved while idle", h.active, 0);

    std::vector<Fire> f = run(h, 1);                     // the next accepted clock
    check("the next accepted clock takes the hand-off and plays member 1's step 1",
          f.size() == 1 && f[0].member == 1 && f[0].position == 0 && f[0].mask == 2u);
    check("and the hand-off is consumed", !h.handoff_pending);
}

int main()
{
    loadIsAnImplicitReset();
    stepOrder();
    loneMemberLoops();
    resetReturnsToHead();
    lengthChangeMidChain();
    ratchetsSurviveHandOff();
    memberRemoved();
    oneStepHeadAfterImplicitReset();
    swallowedClockTakesNoHandOff();
    return finish("chain_test");
}
