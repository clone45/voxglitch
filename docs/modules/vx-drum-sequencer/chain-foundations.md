# VX Drum Sequencer: foundations before chaining

**Status:** implemented 2026-09-07, the same day it was proposed, in the order of
§8. The engine is `VXDrumSequencerEngine.hpp`, the data types
`VXDrumSequencerTypes.hpp`, the tests `tests/vx_drum_sequencer/`, and the chain's
recorded behaviour is `rack-port-design.md` §10. The text below is the proposal as
written; where the implementation differs it is noted inline in *[brackets]*.
**Scope:** the refactors worth doing *before* the expander-based chaining feature
(sequencers placed side by side play one after another as a single longer
pattern), so that the feature lands as an addition rather than a rewrite.

This is a review of `src/modules/VXDrumSequencer/` as it stands after v2.46.0.
The module is well documented and correct, and none of what follows is a bug
report. The problem is shape: everything the sequencer *does* lives inside the
one Rack `Module` subclass, and chaining needs one module to do that work on
behalf of others.

---

## 1. What the code looks like today

`VXDrumSequencer` (the `Module`) owns six jobs at once:

| Job | Where | Rack-bound? |
|---|---|---|
| Pattern storage, double-buffered for the audio thread | `banks[2]`, `publishBank`, `liveBank` | no |
| Clock following: edge detect, period measurement, anchor rule | `clock_trigger`, `clock_sample_counter`, `step_samples` | yes (`dsp::SchmittTrigger`) |
| Playhead: position, advance, rewind, first-step convention | `position`, `advance()`, `stepAt()`, `rewind()` | no |
| Firing a step: accent resolve, lane strikes, ratchet arming, UI report | `fire()`, `strike()`, `rat_*[]`, `fired_mask`, `fired_serial` | no |
| Trigger shaping: pulse length, the gap rule, the ACC cut | `writeTriggerOutputs()`, `pulse_remaining[]`, `gap_sample[]`, `was_high[]` | yes (writes `outputs[]`) |
| Jack and button I/O, persistence, lifecycle | `process()` steps 1-8, `dataToJson`, `onReset` | yes |

Three consequences matter for chaining:

1. **The playhead and the pattern are welded together.** `fire(bank, pos)`
   takes a bank but reads `current_slot` and `mute` from `this`. `lengthOf()`
   reads `current_slot` implicitly. There is no way to say "play *that*
   module's memory 3 with *its* mute mask at *my* position."
2. **The UI reads five raw module fields.** The grid widget reads
   `module->position`, `fired_serial`, `fired_mask`, `current_slot`, and `mute`
   directly. In a chain, a follower's grid must show a playhead and lane
   flashes that the *head* produced. Today that data has no home on the
   follower.
3. **Nothing is testable outside Rack.** Timeline shipped with
   `tests/timeline/` because its engine is Rack-free (`timeline_dsp`). The
   sequencer's most intricate logic, the gap rule and ACC cut in
   `writeTriggerOutputs()`, can only be verified by patching. Chaining adds
   hand-off timing on top of that, which is exactly the kind of thing a test
   catches and an ear does not.

---

## 2. Proposal A: a Rack-free engine, composed from four small objects

Move the sequencing logic into a `vx_drum_sequencer` namespace header with no
Rack includes, the way `TimelineEngine.hpp` does. The module becomes an
adapter: read jacks, call the engine, write jacks.

The engine is not one class. It is four objects with one responsibility each,
composed by a fifth:

```
namespace vx_drum_sequencer {

// Measures the clock period. Owns the anchor rule and the sample-rate rescale.
struct ClockFollower {
    void edge(float sample_rate);          // an accepted clock edge landed this sample
    void tick();                           // every sample
    void rearm();                          // reset/rewind: next edge anchors, does not measure
    void rescale(float old_sr, float new_sr);
    float period_samples = 0.f;            // 0 = unknown
};

// The playhead. Knows nothing about lanes, only length.
struct Playhead {
    int position = -1;                     // -1 = nothing played yet
    int advance(int length);               // returns the step to fire
    void rewind();
};

// One step's worth of decisions: which lanes strike, which ratchets arm.
struct StepFire {
    uint32_t struck;                       // lane bits, bit 6 = accent
    uint8_t  extra_hits[VOICES];           // ratchet extras per voice, 0..3
};
StepFire resolveStep(const Memory& m, uint8_t mute, int pos);   // [shipped with an Rng& too: the chance roll, 2026-09-07]

// In-flight ratchet re-strikes per lane.
struct Ratchets {
    void arm(const StepFire& f, float period_samples);
    uint32_t tick();                       // returns lanes re-striking this sample
    void cancel();
};

// Strike bits in, per-lane voltages out. Owns the gap rule and the ACC cut.
struct TriggerShaper {
    void strike(uint32_t lanes);
    void tick(long pulse_len, float out[LANES]);
};

}
```

And the composition:

```
struct Sequencer {
    ClockFollower clock;
    Playhead      head;
    Ratchets      ratchets;
    TriggerShaper shaper;

    struct Report { int position; uint32_t fired_mask; uint32_t fired_serial; };
    Report report;

    // One sample. `source` is whatever memory + mute the caller wants played.
    void process(bool clock_edge, bool reset, const Memory& source, uint8_t mute,
                 float sample_rate, long pulse_len, float out[LANES]);
};
```

### Why this makes chaining easy

The head of a chain owns **one** `Sequencer`. Each sample it decides which
module in the chain is active, resolves that module's effective memory and
mute mask, and passes them in as `source`. The engine never learns that a
chain exists. Followers hold no engine at all while chained; their
`process()` does nothing but copy a report (section 4).

Moving from one module's pattern to the next is a change of *which `Memory`
the caller passes*, not a change inside the engine. The hand-off rule ("after
the active module's last step, the next module's step 1 on the next clock")
becomes a few lines in the head's adapter, and the first-step convention the
module already documents (advance-then-play from -1) applies unchanged at
each hand-off because `Playhead::rewind()` is what a hand-off calls.

### Why this makes the code cleaner regardless

- `fire()` currently does four things: resolve the accent, strike lanes, arm
  ratchets, and write the UI report. `resolveStep()` is a pure function of
  `(Memory, mute, pos)` and returns a value. Arming and reporting happen in
  the caller. Each piece can be read in isolation.
- `writeTriggerOutputs()` is 90 lines whose comment block is longer than its
  code because the gap rule and ACC cut interact. As `TriggerShaper::tick()`
  with a plain float array output, it gets a table-driven test: "two strikes
  N samples apart with pulse length P produce these voltages." Today that
  claim is verified by reasoning in a comment.
- `strike()` carries an `amp` argument that is documented as unused
  ("source parity only"). The refactor is the moment to drop it; parity with
  the source is served by the design record, not by a dead parameter.
- `onSampleRateChange()` rescales `step_samples` and re-anchors. That is one
  line in `ClockFollower::rescale()` next to the counter it affects, instead
  of a lifecycle hook that knows the internals of the period measurement.

### What stays in the module

Jack reads, `dsp::SchmittTrigger` on CLK and RST (Rack's Schmitt with the
house thresholds), MEM CV slot resolution, the sixteen memory buttons, the
bank store, persistence, undo. The module shrinks from ~680 lines to roughly
half, and the half that remains is the Rack glue every module has.

---

## 3. Proposal B: play a "source", not a bank plus a slot

This is the small change inside proposal A that does the most work. Today
the chain of reads that decides what to play is:

```
bank.memories[current_slot]   // fire()
this->mute                    // fire()
bank.memories[current_slot].length   // lengthOf()
```

`current_slot` and `mute` are ambient state. Replace them at the engine
boundary with an explicit value:

```
struct PlaySource {
    const Memory* memory;   // the effective memory this sample
    uint8_t       mute;     // the mute mask that applies to it
};
```

The module resolves a `PlaySource` once per sample from its own MEM CV, its
buttons, and its mute mask, exactly as `process()` step 2 does now, and hands
it to the engine. In a chain, the head resolves the active follower's
`PlaySource` from *the follower's* CV, buttons, and mute. Followers keep their
MEM buttons and MEM CV meaningful while chained: each one still chooses which
of its sixteen memories it contributes. That is a feature for free, and it
falls out of making the source explicit.

`lengthOf()` disappears; `Playhead::advance(source.memory->length)` is the
whole call.

---

## 4. Proposal C: a `Report` struct the UI reads, on every module

The grid widget's five direct reads become one:

```
struct Report {
    int      position;      // -1 = idle
    uint32_t fired_mask;
    uint32_t fired_serial;
    int      slot;          // the effective memory shown and edited
    uint8_t  mute;
};
Report report;              // on the module; audio writes, UI reads
```

The widget's null-safe accessors (`position()`, `slot()`, `muteMask()`, the
flash sync in `step()`) read `module->report.*` and nothing else changes in
the widget. This is a rename today.

In a chain it is the whole UI story. The head's engine produces one `Report`
per sample. The head writes it into *its own* `report` when it is the active
module, and otherwise writes an idle report for itself. Each follower's
`process()` walks left to the head and copies `head->reportFor(me)` into its
own `report`. The grid widget on every module in the chain draws a playhead
and flashes lanes without knowing whether it is head or follower. *[Implemented
as one `ChainReport` on the head, tagged with the member whose step fired
last; every module, head included, applies it through the same
`applyChainReport(report, my_index)`. The hand-off is taken on the clock edge
after the last step fires, not on the fire itself, so the last step keeps its
lamp for the whole step.]*

Thread note: both sides of that copy are on the audio thread, but Rack can
run modules of one frame on different worker threads, so the follower reads a
struct the head may be writing. The fields are single aligned words and the
module already accepts torn reads of exactly this kind for `fired_mask`
(the comment at the grid's flash sync says "one word behind the serial at
worst: one frame misdrawn"). Keeping the report to aligned ints keeps that
argument valid. Nothing the head *reads* from a follower is written by the
follower's audio thread except `slot` and `mute`, which are the same class of
field.

---

## 5. Proposal D: chain topology in one helper, outside the engine

The expander mechanics belong in one small object on the module, not spread
through `process()`:

```
struct ChainLink {
    VXDrumSequencer* head = nullptr;      // nullptr when I am the head
    int index = 0;                        // 0 = head, 1.. = position in the chain
    int length = 1;                       // modules in the chain, head included

    void resolve(VXDrumSequencer* self);  // walk leftExpander/rightExpander, checking the model
    bool isHead() const { return head == nullptr; }
};
```

`resolve()` runs in `onExpanderChange()` (the engine is locked, so the walk is
safe) and again on each `process()` call to pick up the pointers, which is a
loop of at most a handful of steps. The house precedent is GrooveBox's
`onExpanderChange` and its `expander_connected` flag; this is that pattern
generalised to a walk instead of a single neighbour.

The head does **not** relay messages hop by hop through
`producerMessage`/`consumerMessage`. It reads each follower's bank through
the follower's own `liveBank()` (UI-written, atomically swapped, safe from
any thread) and each follower's `slot` and `mute` directly. The VCV community
thread on daisy-chained expanders reached the same conclusion: one brain
pulling beats N modules pushing copies rightward, and it has no per-hop
latency.

Identity is by direct pointer and adjacency, never by a stored name or id,
which is the rule in the house `CLAUDE.md`. `index` is structure, valid only
for the frame it was computed in, and is never persisted.

---

## 6. Proposal E: a test harness, `tests/vx_drum_sequencer/`

With proposal A done, this is mechanical and follows `tests/timeline/README.md`
exactly:

- **engine_test.cpp**: `Playhead` first-step convention, `ClockFollower`
  anchor and rescale rules, `resolveStep` with mutes and the accent lane,
  `Ratchets` subdivision and the "no period, no extras" rule.
- **shaper_test.cpp**: `TriggerShaper` gap rule and ACC cut, as voltage
  tables. The five paragraphs of comment above `writeTriggerOutputs()` become
  five test cases.
- **chain_test.cpp**: three banks, one `Sequencer`, a simulated head that
  switches `PlaySource` on hand-off. Asserts the step order across the chain,
  that a reset returns to module 1 step 1, that a follower's length change
  mid-chain is honoured, and that ratchets armed on the last step of one
  module finish while the next module's step 1 fires.

None of these exist today, and the sequencer is the only recent module
without them. *[Written: 113 + 29 + 27 checks, all passing.]*

---

## 7. What this does *not* change

- The persisted JSON. Nothing new is stored; `position` and the clock period
  were already deliberately unpersisted, which is what a chain wants.
- The panel. Chaining by adjacency needs no jacks.
- The undo model. `VXDrumSequencerBankAction` and `commitBankEdit` are
  untouched; edits on a follower stay local to that follower's bank.
- The clock/reset semantics in the design record. The first-step convention,
  the no-ignore-window rule, and the period measurement move into named
  objects but keep their behaviour and their comments.
- The `VoxglitchModule` base, the `PanelHelper` marker contract, the grid
  widget's geometry and paint gesture.

---

## 8. Order of work

1. **Proposal C** (the `Report` struct). One hour. Pure rename, zero risk,
   and it is the seam the chain UI hangs on.
2. **Proposal A with B** (the engine split with an explicit `PlaySource`).
   Half a day. Compile-check against the SDK with the usual `-fsyntax-only`
   gate, then patch-test that a lone sequencer is bit-identical: same steps,
   same ratchets, same pulse shapes. The design record gets a short note
   that the algorithm now lives in `VXDrumSequencerEngine.hpp`.
3. **Proposal E** (tests) for the engine and the shaper. Two hours, and it
   locks step 2 in place before step 4 starts changing timing.
4. **Proposal D and the chain feature itself.** With 1-3 done this is: the
   `ChainLink` walk, the head's per-sample "which follower is active" loop,
   the hand-off at the last step, and a follower `process()` that copies its
   report. Plus `chain_test.cpp`, the manual, and the design record.

Steps 1-3 are worth doing even if chaining is never built. Step 4 without them
means threading a chain index through `fire()`, `lengthOf()`, `advance()`,
`process()`, and the grid widget, and testing the result by ear.
