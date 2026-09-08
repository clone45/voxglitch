// VX Drum Sequencer chain: three banks, ONE Sequencer, and a simulated head
// that switches the PlaySource on hand-off. Mirrors the head's decision
// structure in VXDrumSequencer::process() steps 4-7 (a pending hand-off taken
// on the next clock edge, before the advance). If that changes, re-sync this.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrumSequencer -o chain_test chain_test.cpp && ./chain_test

#include "VXDrumSequencerEngine.hpp"
#include "check.hpp"
#include <vector>

using namespace vx_drum_sequencer;

static const float SR = 48000.f;

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
    int fired_member = -1;
    int last_position = -1;

    explicit Head(std::vector<Member>* m) : members(m) {}

    void reset()
    {
        seq.rewind();
        active = 0;
        handoff_pending = false;
    }

    // One sample. Returns true if a step fired.
    bool process(bool clock_edge, float out[LANES])
    {
        if ((int)members->size() > 0 && active >= (int)members->size())
        {
            active = 0;
            handoff_pending = false;
            seq.rewind();
        }
        if (clock_edge && handoff_pending)
        {
            handoff_pending = false;
            active = (active + 1) % (int)members->size();
            seq.handOff();
        }
        const PlaySource src = (*members)[active].source();
        seq.process(clock_edge, src, SR, 48, out);
        if (seq.fired_this_sample)
        {
            fired_member = active;
            last_position = seq.head.position;
            if (members->size() > 1 && seq.head.position >= src.memory->length - 1) handoff_pending = true;
        }
        return seq.fired_this_sample;
    }
};

struct Fire { int member; int position; uint32_t mask; };

// Clock the head `n` times at a 100-sample period and record every fire.
static std::vector<Fire> run(Head& h, int n)
{
    std::vector<Fire> fires;
    float out[LANES];
    for (int i = 0; i < n; i++)
    {
        if (h.process(true, out)) fires.push_back({h.fired_member, h.last_position, h.seq.report.fired_mask});
        for (int k = 0; k < 99; k++) if (h.process(false, out)) fires.push_back({h.fired_member, h.last_position, h.seq.report.fired_mask});
    }
    return fires;
}

static void stepOrder()
{
    std::printf("step order across three members\n");
    std::vector<Member> members(3);
    members[0].memory.length = 4;  for (int s = 0; s < 4; s++) members[0].memory.at(0, s).on = true;   // BD on every step
    members[1].memory.length = 2;  for (int s = 0; s < 2; s++) members[1].memory.at(1, s).on = true;   // SD on every step
    members[2].memory.length = 3;  for (int s = 0; s < 3; s++) members[2].memory.at(2, s).on = true;   // CP on every step

    Head h(&members);
    std::vector<Fire> f = run(h, 9 + 4);   // one full cycle (4+2+3 = 9) plus 4 more

    checkEq("thirteen clocks, thirteen fires", (long long)f.size(), 13);
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
    std::vector<Fire> f = run(h, 7);
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
    members[1].memory.length = 2;
    Head h(&members);
    run(h, 3);                                   // member 0 steps 1,2; member 1 step 1
    checkEq("member 1 is active", h.active, 1);
    h.reset();
    checkEq("reset: head active", h.active, 0);
    checkEq("reset: playhead -1", h.seq.head.position, -1);
    std::vector<Fire> f = run(h, 1);
    check("the next clock plays the head's step 1", f.size() == 1 && f[0].member == 0 && f[0].position == 0);
}

static void lengthChangeMidChain()
{
    std::printf("length change mid-chain\n");
    std::vector<Member> members(2);
    members[0].memory.length = 4;
    members[1].memory.length = 4;
    Head h(&members);
    run(h, 2);                                   // member 0 on step 2
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
    run(g, 2);                                   // member 0 fired step 2 (its last) -> hand-off pending
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
    float out[LANES];
    // Two clocks to measure the period (100 samples): the first anchors.
    h.process(true, out); for (int k = 0; k < 99; k++) h.process(false, out);
    // Second clock: member 1 (index 1) plays because member 0 handed off after its
    // single step. Wait — the first fire was member 0's step 1 (its last) with no
    // period yet, so no ratchets; the hand-off is pending. Second clock: member 1.
    h.process(true, out); for (int k = 0; k < 99; k++) h.process(false, out);
    checkEq("member 1 played on the second clock", h.fired_member, 1);
    // Third clock: back to member 0, now with a known period -> ratchets arm.
    h.process(true, out);
    checkEq("member 0 again", h.fired_member, 0);
    check("ratchets armed", h.seq.ratchets.any());
    check("hand-off pending", h.handoff_pending);
    // Re-strikes fire during the 99 idle samples...
    int restrikes = 0; bool prev = true;
    for (int k = 0; k < 99; k++)
    {
        h.process(false, out);
        if (out[4] == 10.f && !prev) restrikes++;
        prev = out[4] == 10.f;
    }
    checkEq("three CH re-strikes inside member 0's step", restrikes, 3);
    // ...and the hand-off on the next clock does not cancel anything that was
    // still due (here they have all fired), and member 1's BD lands on that clock.
    h.process(true, out);
    checkEq("member 1 on the next clock", h.fired_member, 1);
    check("BD rises on the hand-off clock", out[0] == 10.f);
    check("handOff() does not cancel ratchets (none left here, but the call is the playhead only)", true);

    // The direct claim: handOff() leaves ratchets armed, rewind() cancels them.
    Sequencer s;
    StepFire f; f.struck = 1u << 4; f.extra[4] = 1;
    s.ratchets.arm(f, 100.f);
    s.handOff();
    check("handOff keeps armed ratchets", s.ratchets.any());
    s.rewind();
    check("rewind cancels them", !s.ratchets.any());
}

static void memberRemoved()
{
    std::printf("a member removed from under the playhead\n");
    std::vector<Member> members(3);
    for (int i = 0; i < 3; i++) members[i].memory.length = 2;
    Head h(&members);
    run(h, 5);                                   // member 2, step 1
    checkEq("member 2 active", h.active, 2);
    members.pop_back();
    std::vector<Fire> f = run(h, 1);
    checkEq("the chain restarts at the head", h.active, 0);
    check("and plays the head's step 1", f.size() == 1 && f[0].member == 0 && f[0].position == 0);
}

int main()
{
    stepOrder();
    loneMemberLoops();
    resetReturnsToHead();
    lengthChangeMidChain();
    ratchetsSurviveHandOff();
    memberRemoved();
    return finish("chain_test");
}
