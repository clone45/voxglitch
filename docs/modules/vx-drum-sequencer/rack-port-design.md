# VX Drum Sequencer — VCV Rack Port Design

Where the Rack port of the vxsynth **VX Drum Machine** (and its trigger-input twin,
**VX Drums**) deliberately diverges from the source, and why. Read alongside the two
user manuals, `docs/modules/vx-drums/user_manual.md` and the one in this directory.
The port ships as two modules, `VXDrums` (the kit) and `VXDrumSequencer` (the
sequencer); this document covers both, because the biggest divergence is the split
itself.

`vxdrums.c:NNN` citations are to the vxsynth source as it stood on 2026-09-01, the
day the port was specified; `§n` references are to the port's binding design spec of
the same date, and *brief-clock-reset §n* to the clock/reset investigation that spec
adopts.

**Status:** design bound. Every behaviour below is either fixed by the spec or copied
from the source unchanged. Nothing here is provisional. Two revisions after the initial
port shipped: on 2026-09-02 the owner dropped the internal clock and swing, so the
sequencer is external-clock only (§1, §3, §4); on 2026-09-07 the sequencing logic moved
into a Rack-free engine and sequencers gained chaining by adjacency (§10,
`chain-foundations.md`).

---

## 1. Decisions locked

| Decision | Value |
|---|---|
| Modules | **Two**: `VXDrums` and `VXDrumSequencer`, 33 HP (495 px) each, stacked kit-over-sequencer. The initial port was 30 HP; the width grew on 2026-09-02 for the kit's CV jack column, and the sequencer follows so the pair still lines up |
| Kit CV | **Four polyphonic CV inputs** on the kit, one per knob row (TUNE / DECAY / SHAPE / LEVEL), in a jack column left of the knobs. Channel c (1-6) modulates column c's knob; channel 7 the row's master knob (TUNE -> ACCENT, SHAPE -> DRIVE, LEVEL -> VOLUME; DECAY has none). `getPolyVoltage`, so a mono cable normals to every column. 10 V = the knob's full range, added to the knob position and clamped: `effective = clamp(knob + cv / 10 * range, min, max)`. The engine sees the effective value; the knobs and tooltips show the knob position. Knobs + CV are read through a 16-sample `dsp::ClockDivider`, on change only, so a fast LFO on TUNE never recomputes coefficients per sample; the trigger/accent path stays per-sample. Added 2026-09-02 at the owner's request |
| Link | **Two cables**: a 6-channel polyphonic **TRIG** plus a separate mono **ACC**. No seven-channel bus, no mono trigger jacks on the kit |
| Trigger pulses | 10 V, **1 ms** default, context-menu selectable 1-200 ms, one-sample forced 0 V gap on a re-strike |
| Accent | A **gate sampled at the trigger instant**; poly ACC accents per voice, mono ACC normals to all six |
| Clock | **External only.** No internal clock, no BPM control, no RUN transport; Song dropped with the rest. The initial port shipped Ext/Int; the owner dropped Int on 2026-09-02 |
| Reset | `position = -1`, advance-then-play, **no clock-ignore window**, the clock Schmitt is never reset; RST = the RWD button |
| Swing | **None.** Dropped with the internal clock on 2026-09-02 (it only ever applied in Int); swing the clock upstream instead |
| Ratchets | x1-x4 per hit, subdividing the measured clock period (dropped until one is measured) |
| Memories | 16 x (a 7 x 16 grid of pads, each `{on, ratchet, chance}`, + length); mutes and kit knobs are global. The source's lane masks and ratchet words were unpacked into a per-pad struct on 2026-09-07 (§7.1) |
| Chance | Per pad, 0-100 %, rolled once per step reached; a failed roll neither strikes nor arms ratchets nor flashes. The accent lane rolls too. Set from the pad's right-click menu (presets) or, in **Chance mode** (a persisted panel-menu checkbox), by a vertical drag on the pad, which the grid then draws as a bar. Added 2026-09-07 at a user's request |
| Random | Acts on the **effective** memory (MEM CV when patched, else the buttons), rewriting the lanes chosen in "Randomize settings" (all seven by default; a persisted mask, added 2026-09-07 at a user's request). Every lane is still DRAWN whatever the mask, so a kept lane does not shift the random stream for the others |
| Clipboard | The **OS clipboard**, as JSON text |
| Undo | Rack's history system, one `history::ModuleAction` per gesture |
| Pattern hand-off to audio | Double-buffered `Bank` + atomic index (the PianoRoll idiom) |
| Chaining | **By adjacency** (Rack expanders), no jacks: the leftmost sequencer is the head and owns the one engine; members to its right play in turn. The head pulls each member's bank, slot and mute directly; no hop-by-hop messages (§10). Added 2026-09-07 |
| Panels | Authored by the port from a generator script; light plus the generated dark theme |

---

## 2. One panel became two, one wire became two

The source is a single panel and a single DSP state: the kit strip across the top,
the grid, memory row and clock row below (`vxdrums.c`, `vxdrums-faceplate.js`). In
Rack the two halves are separate modules of the same width, meant to be stacked and
joined by cables (§2). Reasons:

- The kit is useful on its own, and the source already had that variant
  (`vxdrumvoices.c`, "VX Drums"). The port's kit is that variant with one polyphonic
  trigger input in place of its six mono jacks.
- The sequencer is useful on its own: TRIG is an ordinary six-channel trigger bus that
  will drive any drum module.
- Mixed-purpose channels break when a user splits a bus into other modules (owner's
  instruction), so the accent travels on its own cable rather than as a seventh channel.

What moved between the halves:

- **SWING** left the kit's master strip, which therefore has a gap at row 2 - ACCENT,
  (gap), DRIVE, VOLUME - exactly as `vxdrumvoices` already laid it out
  (`vxdrumvoices.js:23-25`). The initial port moved it to the sequencer, where the timing
  was; the 2026-09-02 revision removed it altogether (§3).
- The kit gained six **strike lights**, one above each voice column, since it can no
  longer see the sequencer's lane flashes.
- The sequencer keeps the grid, the lamp row, the memory row, the CLK / RST / MEM
  jacks, the rewind and Random buttons and the two outputs. It has no tempo, transport
  or swing control of its own: the clock row is jacks and buttons only.

### 2.1 The protocol

| Sequencer jack | Kit jack | Signal |
|---|---|---|
| TRIG (poly, 6 ch) | TRIG (poly) | channel c = voice c in strip order 0 BD, 1 SD, 2 CP, 3 PERC, 4 CH, 5 OH; 10 V pulses (§5) |
| ACC (mono) | ACC | 10 V pulse, same length, raised on the same sample as every accented strike |

Kit side: one `dsp::SchmittTrigger` per channel with the house thresholds
`constants::gate_low_trigger` / `constants::gate_high_trigger` (0.1 V / 2.0 V) rather
than the source's 1.0 V / 0.1 V arm (`vxdrumvoices.c:372-381`), matching every other
Voxglitch clocked input. TRIG is read with `getVoltage(c)`, so a mono cable fires BD
only and channels beyond the cable's count are 0 V; ACC is read with
`getPolyVoltage(c)`, so a mono accent normals to every voice and a poly accent is per
voice. On a rising edge on channel c, voice c is struck with
`amp = (ACC(c) >= 2 V) ? 1 + accent : 1` - the source's accent-as-a-gate semantics
(`vxdrumvoices.c:30-35`), unchanged. Retrigger mid-tail restarts the voice, as in the
source. Both modules process the pair on the same sample, so a Rack cable delivers the
TRIG and ACC edges together.

---

## 3. Dropped from vxsynth

- **The internal clock** (the Int branch of the source's step engine) and the
  `CLOCK: Ext|Int|Song` pill. The initial port kept Ext / Int behind a two-position
  INT switch; on 2026-09-02 the owner dropped Int as well, so the sequencer steps only
  on CLK edges. Rack patches already have a clock master, and one fewer timing source
  means one fewer way for the kit and the rest of the patch to disagree.
- **Song clock mode** and the whole derived-position branch (`vxdrums.c:630-663`).
  Rack has no host playhead.
- **The ▶ / ⏹ / ⏮ transport** (`panel-transport.js`). There is no RUN: with no internal
  clock there is nothing to start or stop, and the clock cable is the transport. ⏮ is
  the RWD button.
- **The BPM control** (`panel-valuebox.js`) and its readout. The initial port replaced
  the value box with a BPM knob and a read-only display; both went with the internal
  clock.
- **Swing** (`vxdrums.c:404-414`, `:577`). In the source it only applied in Int, and the
  port kept that rule while it had Int (delaying a step against an explicit clock moves
  where beats land); once Int went, so did the knob. Users swing the clock upstream.
- **The in-app clipboard** (`memory-slots.js`), replaced by the OS clipboard (§7.3).
- **The 1 ms post-reset clock-ignore window and the hand-rolled `clkArmed` arm** (§4).
- **The kit's six mono trigger inputs** (`vxdrumvoices.c:37-39`): one polyphonic input.
- **`rewind` as a store-bypassing live param**: a plain momentary button.
- **The flash-timing quirk**: the source flashes lane labels on a *step* change and, for
  a swung step, can read the previous step's `fired` mask. The port flashes on a serial
  that increments inside `fire()`, so a flash always shows what just fired.

---

## 4. Clock, reset and the first note

The owner's requirement: reset must be right and the first note must play correctly in
every situation. The engine is *brief-clock-reset §5* verbatim, with the decisions in
§4.4 confirmed, minus the Int branch removed on 2026-09-02. This section records why the
source could not be copied here.

### 4.1 Defects in the source the port does not copy

| # | Source behaviour | Effect | Port |
|---|---|---|---|
| 1 | RST arms a 1 ms window during which clock edges are swallowed (`vxdrums.c:593`) | Reset and clock on the same sample: the clock consumes the arm inside the window and is rejected, so **step 1 plays one whole period late**. The same symptom ArpSeq fixed in #278 and DigitalSequencer still has | **No window, ever.** Reset is processed before the clock on the same sample, so a coincident edge fires step 1 immediately |
| 2 | A rejected edge does not zero `extClkCounter` (only the accept branch does, `:613`) | The next accepted edge measures **two** periods, so ratchets run at half speed | Zero the counter on **every** detected edge, accepted or not (the PianoRoll port found and fixed the same bug in `pianoroll.c`) |
| 3 | Reset and mode change set `clkArmed = 1` (`:522`, `:588-600`) | A clock line that is high at the moment of a reset (or, in the source, a switch to Ext) fires a **phantom step** mid-pulse | One `dsp::SchmittTrigger`, polled every sample, never `reset()`. A high line is `HIGH`, not an edge |
| 4 | `extClkCounter` only counts in Ext (`:603`) and is not zeroed by a mode change | The first edge after Int -> Ext measures a frozen count plus the time since the switch: garbage, accepted if under 4 s | No mode change exists any more. The counter runs every sample and the measurement **anchor** is invalidated on load, reset and rewind (and on a sample-rate change, row 6); the first edge after any of those anchors without measuring |
| 5 | RST in Int mode leaves `phase` and `stepNow` alone (`:593-596`), unlike the rewind button (`:548`) | A reset pulse while running restarts the pattern **up to a full step late** and out of phase with whoever sent it | RST does exactly what RWD does; there is one reset path, not two (and no Int phase to leave alone) |
| 6 | `sampleRate` is cached at create (`:459`); `phase`, `pendingCount` and `ratInterval` are in samples | Rack changes the rate at runtime | Ratchet intervals and pulse lengths are computed from `args.sampleRate` when armed. `onSampleRateChange` has one job: the measured clock period (`step_samples`) is stored in samples and by design survives resets, so it is rescaled by `new / old` rate (ratchets stay right until the next edge re-measures) and the measurement is re-anchored (`clock_anchor_valid = false`, counter zeroed), because the running sample counter straddles two rates and must not be read as a period; the first edge after a rate change therefore anchors without measuring. Nothing else in the clock path needs it. Rack dispatches the event on add as well, which seeds `last_sample_rate`; that first call, with no previous rate, is a no-op |
| 7 | `clkArmed = 1` at create (`:452`) | A line already high at patch load fires a step on the first sample | Rack's Schmitt starts `UNINITIALIZED` and `UNINITIALIZED -> HIGH` reports no edge |

### 4.2 The rules

- **`position = -1` and advance-then-play.** -1 means nothing has played since load,
  reset or rewind; the next step to fire is step 1 (index 0). The source already does
  this (`vxdrums.c:117`, `:431-435`); PianoRoll and ArpSeq do the same. The clock edge
  that accompanies a reset **is** the step-1 edge; nothing fires at the reset instant.
- **Reset = `rewind()`:** cancel every ratchet re-strike (the source's `vxd_silence`,
  `:424-430`), `position = -1`, invalidate the measurement anchor, zero the sample
  counter. Voices ring out; trigger pulses already high finish.
- **Order inside one sample:** RWD button, **RST input, then CLK input**, then ratchet
  re-strikes, then the outputs.
- **Period measurement:** count samples since the last detected edge; measure only
  when the anchor is valid and the count is `> 1` and `< 4 s` (the source's debounce,
  `:610`); the measured period **survives reset** and is 0 only at load (so ratchets on
  the first step after a reset still subdivide, and ratchets on the very first step
  after connecting are dropped, exactly as the source manual promised); an edge always
  steps, whether or not it measured. A sample-rate change rescales the stored period to
  the new rate and invalidates the anchor (zeroing the counter), so the first edge after
  it anchors without measuring and the one after re-measures.
- **No internal clock, no transport, no swing** (2026-09-02). Every step is a CLK edge;
  there is no run state, no mode change and no pending swung fire, so the only deferred
  work in the engine is the ratchet re-strike queue.
- **Ratchets:** on a step fire, per unmuted voice lane with a hit, `extra` (0-3) extra
  hits arm only if the step length is known (`> 4` samples); interval
  `max(2, stepSamples / (extra + 1))`, first re-strike one interval after the fire, each
  at the amplitude the step fired at (accent included). Re-strikes take the same strike
  path as a step, so CH re-chokes OH on every hit and the ACC pulse rides along when
  accented. Source `:383-394`, `:674-682`, unchanged.
- **Mute:** a muted voice lane neither strikes nor pulses at TRIG; a muted accent lane
  removes both the boost and the accent lamp bit (source `:373-379`).

### 4.3 What happens, case by case

| Situation | Result |
|---|---|
| Reset while the clock is low | Silent; the next rising edge plays step 1 |
| Reset and clock edge on the same sample | **Step 1 plays on that sample** |
| Reset while the clock line is still high | Silent; the line must fall and rise; that rise plays step 1 |
| Reset mid-period, clock free-running | Silent; the next edge plays step 1; the partial interval is *not* measured; ratchets use the previous period; the edge after re-measures |
| First clock after patch load | Step 1 with single hits (no period yet); the second edge measures |
| Clock line already high at load | Nothing until it falls and rises |
| Clock unpatched mid-pattern | Silent; position kept; the next edge after re-patching plays the following step |
| Sample-rate change | Ratchets keep their subdivision from the rescaled period; the next edge anchors without measuring; the edge after re-measures |
| Patch reload | `position = -1` regardless of where it was saved |

The manual's advice follows from the fourth row: a clock master that restarts its own
clock on reset turns a mid-bar reset into the second row, so users who want resets to
land where they press them should patch the master's reset into RST as well as its
clock into CLK.

---

## 5. Trigger outputs

- **10 V, 1 ms default**, from ArpSeq's list `{1, 2, 5, 10, 20, 50, 100, 200} ms`,
  chosen from the module context menu and persisted as an index. 1 ms is the VCV
  standard for triggers and Timeline's choice; drum voices fire on the edge, so a longer
  pulse buys them nothing; and a x4 ratchet at high tempo, or a fast external clock, can
  space hits closer than 10 ms, where a 10 ms pulse would merge them (the GrooveBox
  expander's symptom today). Users who need a wider pulse downstream get it from the
  menu.
- **Re-strike gap.** `dsp::PulseGenerator::trigger()` only extends an existing pulse, so
  it cannot emit a second edge inside one. The port uses per-lane sample countdowns
  instead, with one rule: a strike on a lane whose output is still high forces that
  lane to **0 V for exactly one sample**, and the new pulse starts on the following
  sample. One sample is enough for every Rack Schmitt, and it is the only gap that fits
  under the source's two-sample ratchet floor. (PianoRoll's 1 ms gate gap is the
  in-repo precedent for a forced-low retrigger; 1 ms would push re-strikes late here.)
  The rule is applied to the sample's whole strike set, not lane by lane: if any lane
  struck on this sample (ACC included) is still high, every lane struck on this sample
  takes the one-sample gap, so ACC and the triggers it accents always rise on the same
  sample. The kit samples the ACC gate on the sample the trigger's Schmitt fires, so a
  per-lane gap (*brief-clock-reset §7.2*'s pseudocode) would, with a long pulse length,
  drop a still-high ACC for one sample while a freshly struck low voice rose
  immediately, and that hit would lose its accent. The cost is that a low lane struck
  alongside a high one starts one sample late. The gap keys on whether the lane's output
  was high on the *previous* sample, not on the countdown, so a strike landing exactly
  one pulse length after the last one (100 ms triggers at 150 BPM, or a clock whose
  period equals the pulse) still gets its own 0 V sample instead of merging.
- **ACC** is lane 7 of the same mechanism, raised on the same sample as any accented
  strike, step fire or ratchet re-strike alike, whether or not a voice fires on that step,
  and silent when the accent lane is muted. An **un-accented** strike cuts any ACC pulse
  still high from an earlier accented one on that same sample: the kit samples ACC at the
  trigger edge, and with a pulse longer than the step spacing (200 ms above 75 BPM) the
  gate would otherwise carry one step's accent onto the next hit.
- `setChannels(6)` is called **every sample** (it is a no-op while the port is
  disconnected, so a one-time call would leave the jack mono when first patched).
- Mutes silence the jack as well as the voice, because the source skips muted lanes
  before striking (`:379`).

---

## 6. The voices: what changed in the DSP port (§3.2)

`VXDrumVoices.hpp` is `vxdrumvoices.c` ported line for line - the coefficient formulas,
strike amplitudes, the click, the clap burst scheduler, the six hat ratios, the snare
shell pair, the fixed pans, `dr = 1 + drive x 5` and the normalised `fastTanh` drive,
the x5 V output scale. Seven things differ, all deliberately:

1. **`fastTanh`** - the Padé approximation `x(27 + x^2) / (27 + 9x^2)`, clamped to +/-1
   beyond |x| > 3, is copied verbatim. `std::tanh` audibly softens the BD, rimshot and master
   drive.
2. **`fastSin`** - replaced by `std::sin`. It is only used to compute filter
   coefficients at knob time, where the table's 3.5e-7 error is irrelevant.
3. **`js_exp`, `fast_floor`, `fast_abs`, `dsp_clamp`** - the standard library
   equivalents, with a local double clamp (`rack::math::clamp` has no double overload).
4. **RNG** - a per-instance xorshift32, seeded from `random::u32() | 1`. The source's
   generator is only verified statistically, so any uniform source is equivalent.
5. **Sample-rate correctness** - the source hard-codes two per-sample constants tuned at
   48 kHz: the clap burst decay `0.9931` and the hat choke fade `0.9977`. They become
   time constants (`exp(-1 / (0.0030 x sr))` and `exp(-1 / (0.00905 x sr))`), equal to
   the source at 48 kHz and correct at every other rate. The hat HP/BP tilt
   coefficients are *not* rate-corrected in the source either, but they define the
   timbre, so they are kept verbatim and the code says so.
6. **Setter clamps** - the Machine's clamps (hat tone and metal, accent, drive to 0..1)
   are applied even though `vxdrumvoices.c` omits them.
7. **Denormals** - the source's `< 1e-4 -> 0` envelope floors and the NaN / 1e6 guards
   are kept.

### 6.0 VARY (added 2026-09-02)

A master-strip knob with no counterpart in the source. Each strike on a column draws four
uniform random offsets in -1..1 (one per knob row) and holds them for the hit; the
effective knob value becomes `knob + CV + VARY * depth(row) * offset * range`, clamped, with
depths TUNE 0.08, DECAY 0.25, SHAPE 0.30, LEVEL 0.25 of the knob range. The offsets are
pushed into the engine on the strike sample, ahead of the strike, so the coefficients the
hit rings with are already the varied ones; the master knobs are never varied. A
sample-and-hold per hit was chosen over a continuous random CV so a hit keeps one character
for its whole ring. Nothing is persisted beyond the knob.

### 6.1 Kits and models

The kit module grew a layer above the six ported voices: twenty **models** and seven
named **kits**. A model is one drum kernel reduced to a fixed topology and three knobs,
which is exactly what `vxdrumvoices.c` already did for its six voices; the seven new
kernels (`kickdrum`, `kickfm`, `kickdist`, `snare808`, `snaredrum`, `snarering`, and the
`kickdrum` kernel again as a tom) are ported under the same §3.2 rules. Three more snares,
**Snare 606**, **Snare Sweep** and **Snare Gate**, are original designs written for the
port on 2026-09-02, not ports of anything in vxsynth: they follow the same discipline
(cached coefficients, every decay a T60 in seconds, envelope floors, NaN guards, a strike
at amp 1.0 peaking near +/-1.0) but cite no source. Snare 606 is a thin two-ping body
under a band-limited noise snap whose length is `DECAY` and whose level is `SHAPE`;
Snare Sweep is a sine whose pitch drops from up to four times `TUNE` over 60 to 180 ms,
with `SHAPE` setting the drop's depth and time; Snare Gate is a short body plus a
band-passed noise tail that decays as flat as `SHAPE` says and is cut hard, with a 5 ms
fade, at the gate time `DECAY`. Their filtered noise is scaled by sqrt(sr / 44100) so
the band-limited noise power is the same at every rate (constant-variance white noise
halves its power per Hz at twice the rate); the ported kernels keep the source's noise
verbatim. Four more claps, **Clap 909**, **Clap Trap**, **Clap Lo-Fi** and **Clap Gate**,
are original designs of the same date under the same discipline, informed by the TR-808
and TR-909 service-manual clap recipes, the Simmons Claptrap and the Linn sampled clap
(the original `Clap` is now **Clap 808**; its uuid and internals are unchanged). Clap 909
is the 909's two-path recipe: a 31-stage shift-register noise source clocked at a fixed
48 kHz and never reset, so every hit differs, through a bandpass at `TUNE`, into a
sawtooth burst train whose count and spacing `SHAPE` (Density) sets and a slower
attack-release tail with T60 `DECAY`. Clap Trap is the Claptrap: six short clicks ring a
sharply resonant bandpass at `TUNE`, their spacing and level re-drawn per hit by `SHAPE`
(Humanize), over a highpassed noise "reverb" with T60 `DECAY`. Clap Lo-Fi is the Linn /
TR-707 character synthesized: one dense burst into a short tail through a bandpass, then
a deliberate hold-and-quantise stage on the output whose rate (28 to 11 kHz) and depth
(12 to 6 bits) `SHAPE` (Crunch) sets, the hold running on a fractional accumulator so the
effective rate is the same at every sample rate. Clap Gate is the 808 burst train over
Snare Gate's room: a band-limited noise tail with a 5 ms attack, a decay as flat as
`SHAPE` (Hold) says, cut hard with a 5 ms fade at the gate time `DECAY`. Each is trimmed
so its ear-weighted 80 ms RMS at default knobs matches Clap 808's. The sequencer is
untouched except for the lane name: column 3 is **PERC**
(percussion), a rimshot in the House, TR-808 and TR-909 kits, a tom, a ringing snare or
the sweep snare in the others.

- **Normalized knobs.** `TUNE`, `DECAY` and `SHAPE` are 0..1 params mapped linearly onto
  each model's natural range (`natural = min + v01 x (max - min)`); the tooltip shows the
  natural value in the model's units. `LEVEL` is the plain 0..1.2 mix gain as before.
- **Fixed source knobs.** A model exposes three knobs; the standalone module's others are
  fixed. Kick 808: drive 0.25, click as is. Kick Sine: click 0.5. Kick FM: ratio 1.0,
  punch 0.5, drive 0.3. Kick Distort: punch 0.5, tone 0.6 (fold follows SHAPE above 0.6).
  Snare 808: tone 0.5, snap 0.2. Snare Layered: noise 0.6, body 0.6. Snare Ring: snappy
  0.5, tone 0.5. Rimshot: no punch sweep. Tom: click 0.1. Snare 909, Clap 808 and the hats:
  nothing fixed, same internals as the original port. The standalone modules' level,
  velocity, CV and 1V/oct plumbing is stripped: the kit strikes with `amp` (accent
  already applied) and levels at the mix, and each model is normalised so a strike at
  amp 1.0 peaks around +/-1.0 internally, so kits sit at similar loudness.
- **Eighties.** `KIT_EIGHTIES` (Kick Sine, Snare Gate, Clap Gate, Snare Sweep, Closed Hat,
  Open Hat) is the seventh kit, built on the original snares and the gated clap above;
  `KIT_TR909` carries Clap 909 in its clap column.
- **House is the original sound.** `KIT_HOUSE` (Kick 808, Snare 909, Clap 808, Rimshot,
  Closed Hat, Open Hat) is the six voices this document describes above, the default on
  a fresh module and after Initialize.
- **Knob positions are kept.** Changing kit or model never moves a param; only the
  display mapping and the double-click default change (the widget keeps each
  `ParamQuantity::defaultValue` equal to the effective model's default). "Reset knobs to
  model defaults" is the explicit, undoable way to land on a model's defaults.
- **Overrides.** Effective model of column c = `override[c]` if set, else the kit's
  model for c. Overrides survive a kit change; "Kit default" clears one. Kit and
  override changes go through one undoable module action.
- **Choke is a column rule.** A strike in column 4 calls `choke()` on column 5's active
  model, whatever it is; hat models fade as before, every other model's `choke()` is a
  no-op.
- **Identity.** Every `ModelId` and `KitId` carries a frozen v4 UUID string. Persistence
  (`{"version":"1.1.0","kit":"<uuid>","model_overrides":["<uuid>" or "" x 6]}`) stores
  the UUIDs, never the enum value or the name; an unknown UUID falls back to House / no
  override. Names are display only.

Kit knob ranges, defaults and units are the source's exactly (§3.1). Knob values are
pushed into the DSP only when they change.

---

Changing the kit clears every per-column override (owner decision 2026-09-02), so a kit always starts as designed; the kit action records the overrides it removed, so undo restores them.

## 7. Memories, Random and the clipboard

### 7.1 Data model

A memory is a 7 x 16 grid of pads (lane 6 = accent) and a length 1-16. Each pad is a
`Step { bool on; uint8_t ratchet; uint8_t chance; }`: lit or not, 0-3 extra hits (the
accent lane's is unused), and the percent probability it plays when reached, 100 by
default. Mutes and the kit knobs are global, not part of a memory (source
`vxdrums-faceplate.js:36-40`).

The source packs the same information into seven 16-bit lane masks and six
2-bit-per-step ratchet words (`vxdrums.c:106-112`), and the port shipped that way in
v2.46.0. On 2026-09-07 the owner unpacked it into the per-pad struct: a pad's fields
belong together, a new attribute (chance) is then a field rather than a fourth word to
shift, and equality is elementwise instead of `memcmp` over a padding-free layout. The
packed form survives only as the 1.0.0 JSON, which is still read (§8.5). The seed pattern in memory 1 of
a fresh module - and after Initialize - is the source's (`vxdrums.js:80-87`): BD on
1/5/9/13, CP on 5/13, CH on the eighths, OH on 7/15, accents on 1/9, length 16.

MEM CV owns the effective memory every sample, `clamp(int(cv / 10 x 16), 0, 15)`,
locking the buttons while patched; the grid edits the effective memory, latched at the
start of a drag (source `panel-vxdrumsgrid.js:100-103`, `:326`). All unchanged.

### 7.2 Random acts on the effective memory - a divergence

In the source the grid edits the CV-effective memory but the RANDOM button rewrites the
**button** memory (`vxdrums-faceplate.js:113`), so with MEM patched a press could rewrite a
memory you were neither hearing nor looking at. The port's RND (and Rack's Randomize)
rewrites the memory that is playing and shown. The generator itself
(`vxdrums-random.js`) is copied exactly: the per-lane probabilities, the forced kick on
step 1, the x2 hat and x3 late-snare seasoning, length kept. The audio thread only
raises a request flag on the button edge; the UI thread performs the randomize so it
can be wrapped in an undo action.

### 7.3 The OS clipboard

The source's Copy/Paste is an in-app, module-scope clipboard shared by every VX Drum
Machine in the page (`memory-slots.js`). Rack has no such scope, so Copy writes the memory
to the OS clipboard as JSON tagged `"format": "voxglitch_vx_drum_memory"` and Paste
validates the tag and bounds before writing - the DigitalSequencerXP #220 idiom. This is
strictly better: it works across instances, across patches and across sessions, and the
text can be kept in a file. Paste is shown disabled, never hidden, when the clipboard
holds no valid memory, as in the source. Paste and Clear are undoable bank actions.

The module menu's **Export pattern… / Import pattern…** (added 2026-09-02) write and read
the same JSON as a file through the house `osdialog` idiom (PianoRoll's MIDI export), always
for the *effective* memory (MEM CV when patched, else the buttons). Import replaces that
memory as one undoable bank action; a file without the format tag is refused with a dialog.

---

## 8. UI and persistence

### 8.1 No BPM display

The source's BPM box (vertical drag, right-click entry) has no counterpart: with no
internal clock there is no tempo to show. The initial port carried a snapped `BPM` knob
and a read-only readout beside it; both were removed on 2026-09-02, and the panel's
only dark plate is now the grid itself.

### 8.2 Grid

Geometry is scaled from `panel-vxdrumsgrid.js` into the panel's `grid_area`; the HexDrums
livery (dark body, yellow pads, red accent pads, alternating 4-step wells) is drawn
regardless of the Rack theme, as the source drew it regardless of the page theme. Lit
pads and lamps draw in layer 1, the lights layer, so they stay lit when the room is dimmed. Interaction is the
source's: click/paint pads with the value of the first pad, lamp click sets length, label
click mutes, right-click on a lit voice pad opens the Single / x2 / x3 / x4 list plus a
Chance submenu, right-click on a lit accent pad opens the Chance submenu alone (both
2026-09-07), shift-click on a lit voice pad cycles it one step through the ratchet list (an
addition to the source), and a right-click on an unlit pad or anywhere else falls through
to the module menu. In Chance mode (`chance_mode`, 2026-09-07) lit pads draw as bars
whose fill is their chance over a faint full-pad ghost, and the left button changes
meaning: the press only arms; a vertical move past 3 px on a lit pad becomes a chance
drag that tracks the pointer's height within the pad, top 100 % / bottom 0 %, pinned
past the edges — Digital Sequencer's `editBar` rule, so the bar sits under the cursor
(published live, one undo step on release); a release without one
toggles the pad as a click would; there is no paint-across. The owner chose a mode over
always-on bars so the default grid stays as the source drew it (the feature was requested
by one user). Painting a pad off leaves its ratchet and chance on the pad, so
relighting it brings them back (the source's rule for ratchets, `panel-vxdrumsgrid.js:345-350`,
extended to chance); the pad's dimming and its menu show them. In the module browser
(`module == NULL`) the grid draws the seed pattern.

### 8.3 Undo

Rack's history system, the PianoRoll idiom: one `history::ModuleAction` per gesture
carrying the whole bank before and after (plus the mute byte, so a mute travels in the
same action type), applied first and then pushed, skipped when nothing changed. A drag
publishes after every changed pad so it is audible while painting, but pushes one action
at drag end.

### 8.4 Hand-off to the audio thread

The bank is UI-owned and double-buffered with an atomic index: the UI mutates a copy and
publishes it; `process()` reads whichever buffer is live. `mute` is a single byte written
by the UI and read by the audio thread. The `Report` (`position`, `slot`, `fired_mask`,
`fired_serial`; 2026-09-07, formerly loose fields) is written by the audio thread and read
by the widgets for drawing; in a chain the head's `ChainReport` feeds it (§10).

### 8.5 Persistence

```json
{ "version": "1.1.0",
  "memories": [ { "steps": [ { "on": [0|1 x16], "ratchet": [0..3 x16], "chance": [0..100 x16] }, x7 ],
                  "length": 16 }, ... 16 ],
  "mute": 0, "memory_slot": 0, "trigger_length_index": 0, "chance_mode": false }
```

One object per lane, three arrays of sixteen, so a memory reads the way the grid does.
The same body is the clipboard and pattern-file shape (with a `"format"` tag). The
1.0.0 body - `"lanes": [7 bit-mask words], "ratchets": [6 two-bit-per-step words]` - is
still read whenever `"steps"` is absent, so every patch and file saved before
2026-09-07 loads with every pad at 100 % chance; it is never written.

Loading starts from a seeded bank and overwrites from JSON with bounds checks (memories
capped at 16, lanes at 7, steps at 16, ratchet clamped 0-3, chance 0-100, length
1-16), probing every key so a missing one keeps its default. `position` is **not** persisted: it is -1 after
every load, as PianoRoll does. Neither is the measured period nor any pending ratchet
state. The sequencer has no clock, tempo or swing params; its remaining params are
momentary buttons.

### 8.6 Identity

Lanes, steps and memories are fixed-size indices of a fixed grid - that is structure,
not identity - and the lane names `BD SD CP PERC CH OH AC` are display strings only,
never keys.

---

## 9. What survives unchanged

Position -1 and advance-then-play; silence on every discontinuity with voices left to
ring; ratchets from the measured period with the `> 4`-sample guard and the two-sample
interval floor; the `> 1 && < 4 s` measurement debounce; accent as a strike multiplier
`1 + accent` that enters each voice's own nonlinearity; the CH -> OH choke; per-memory
length with wrap; global mutes including the accent lane; the seed pattern; the Random
generator; the memory data model; and every voice algorithm.

---

## 10. Chaining by adjacency (2026-09-07)

Sequencers placed side by side, touching, play one after another as one longer
pattern. The design and the refactor that made it a small change are in
`chain-foundations.md`; this section records the behaviour.

### 10.1 Why adjacency and not jacks

The alternatives were surveyed on 2026-09-07: the classic Nord Modular's Link -> Rst
recipe (the first sequencer counts past its own pattern; only really works for two),
the Nord G2's Park / Link / Loop trio (every sequencer one-shot, a dedicated Loop input
on the head so patch load knows who starts, and every row output OR'd with a chain
input so no mixer is needed), the Division 6 Mini Sequencer's XP jacks, and Spellbook's
Page expanders in Rack. Jacks need three things a Rack expander chain gets for free:
a way to know which module is the head after a reset, a merge of every member's TRIG
into the kit's single input, and a hand-off signal that survives Rack's one-sample
cable delay. Adjacency answers all three: the leftmost module is the head by
construction, the head drives the one TRIG / ACC pair, and there is no cable.

### 10.2 The rules

- **Roles** are resolved from `leftExpander` / `rightExpander` on every `process()`
  call (a walk of at most 16 pointers) and in `onExpanderChange`. The module with no
  VX Drum Sequencer on its left is the head; every VX Drum Sequencer to its right,
  contiguous, is a member with index 1, 2, ... Roles are never persisted; a loaded
  patch resolves them on its first frame.
- **The head owns the engine.** Its CLK and RST are the chain's transport; its trigger
  length applies to all. A member's CLK, RST and RWD are inert.
- **Every member's TRIG and ACC mirror the head's** (owner request, 2026-09-07: a row
  of sequencers patched into the kit from its right end reads better than a cable
  back from the left). The head writes each frame's voltages into a two-slot buffer
  indexed by `ProcessArgs::frame & 1`; a member reads the other slot, the one the head
  wrote last frame and does not touch this frame. That is a fixed one-sample delay
  with no race: reading the current slot could catch a half-written sample or, if the
  member sampled it twice, skip the one-sample gap between two pulses and merge them.
  Only one member's outputs should reach the kit.
- **A member chooses its contribution.** Its memory buttons, MEM CV and mute mask are
  read by the head each sample, through the member's own module: `liveBank()` (the
  UI-written double buffer, safe from any thread), `report.slot` and `mute` (single
  aligned words, one-sample stale at worst).
- **The hand-off.** When the active member fires its last step (`position >=
  length - 1`), the head marks a hand-off pending. On the **next clock edge**, before
  the advance, it makes the next member active and rewinds the playhead only
  (`Sequencer::handOff`, not `rewind`): ratchets armed on the last step keep running,
  the clock anchor keeps its period, and the advance from -1 lands on the new member's
  step 0 on that very edge. So the last step keeps its lamp lit for the whole step and
  the next member's step 1 is exactly one period later. The last member hands back
  to the head. A chain of one never marks a hand-off and wraps as before.
- **Reset, RWD and Initialize** on the head rewind the engine and return the chain to
  the head (member 0). A member removed from under the playhead does the same.
- **Telemetry** flows one way. The head writes a `ChainReport` (active member, the
  member whose step fired last, the playhead, the fired mask and serial); each member
  copies it into its own `Report` in its `process()`: the playhead only while it is
  active, the fired mask and serial only when the fire is tagged with its index. The
  grid widget reads `report` and does not know whether it is on a head or a member.
- **Becoming a member** (a sequencer placed on the left) rewinds and silences the
  engine once; **becoming the head** (the left neighbour removed) rewinds once.

### 10.3 Threads

The head reads a member's fields and a member reads the head's `ChainReport` while
the other may be writing them: Rack can run one frame's modules on several workers.
Every shared field is a single aligned word, and the worst case is one frame's flash
misdrawn or a one-sample-stale slot or mute, the class of read the module already
accepted for `fired_mask` (§8). No expander messages, no locks on the audio path.

### 10.4 The engine split

`VXDrumSequencerEngine.hpp` (Rack-free) holds `ClockFollower`, `Playhead`,
`resolveStep`, `Ratchets`, `TriggerShaper` and the `Sequencer` that composes them;
`VXDrumSequencerTypes.hpp` holds the pattern data types. The module is the Rack
adapter. Every rule in §4 and §5 moved with its comment and is now a case in
`tests/vx_drum_sequencer/`. The `amp` argument the old `strike()` carried for source
parity was dropped; parity is recorded here, not in a dead parameter.

---

## 11. VX Drums: the user's kits (2026-09-07)

Users asked to save their custom kits and have them appear on every VX Drums, new
instances included. Recorded here because the two modules share this document.

### 11.1 Where they live

One file, `<Rack user folder>/voxglitch/vx_drums_kits.json` (`asset::user`), read on
first use and rewritten atomically (temp file, then `system::rename`) the moment the
library changes. Three Rack mechanisms were weighed:

| Mechanism | Verdict |
|---|---|
| Module presets (`Preset > Save as`) | Already work and still do. But they capture the whole module (master strip, panning), live in Rack's Preset menu rather than the KIT display, and are files by name, not kits with an identity |
| Plugin settings hooks (`settingsToJson` / `settingsFromJson`, stored in Rack's `settings.json`) | Written when Rack saves its own settings, mostly at quit: a kit saved before a crash is lost. Meant for preferences; on the owner's machine two plugins use it, for ~150 bytes each |
| **A plugin-owned file in the user folder** | **Chosen.** The idiom Bidoo, Geodesics, Stoermelder and voxglitch-devices use; written on change, independent of Rack's save cycle, human-readable, shareable |

Shape: `{ "version": "1.0.0", "kits": [ { "uuid", "name", "models": [6 model uuids],
"knobs": [[tune, decay, shape, level] x6] } ] }`. Models by uuid; an unknown one falls
back to House's model for that column rather than dropping the kit. A duplicate uuid in
the file is refused on load, not shadowed.

### 11.2 What a kit is

`KitState` (VXDrumKit.hpp, Rack-free): uuid, display name, the six models, and a
`custom` flag. The module's `kit` is one of these, a RESOLVED COPY, replacing the
enum into the factory table it was until now. `UserKit` is a `KitState` plus the 24
voice knobs: a user's kit is their tuned sound, not only the circuits. The master
strip and the panning switch are not part of a kit. `KitLibrary` is the in-memory
list with add / update / rename / remove; identity is the uuid, minted once at save
(`mintUuid`, a v4 from `rack::random`); names are display only and may collide.

### 11.3 Rules

- **Loading a factory kit keeps the knobs** (the physical-knob rule, §8 of the kits
  design); **loading a user kit sets them**, as one `history::ComplexAction` holding
  the kit action plus Rack's `ParamChange` entries. Overrides clear either way.
- **Save kit as…** captures the EFFECTIVE model of every column (overrides baked in)
  and the knobs, mints a uuid, writes the file, then loads the new kit into the module.
  **Update "<name>"** overwrites the loaded user kit in place. **Manage kits** offers
  Rename… and Delete… per kit. Names come from `osdialog_prompt`. The library changes
  are not undoable (they are a file); the module change that accompanies them is.
- **Patches stay portable.** The patch stores the resolved kit (`kit`, `kit_name`,
  `kit_custom`, `kit_models`) plus the overrides, format 1.2.0. A factory uuid is
  re-resolved from the table on load so a retuned factory kit wins; any other uuid
  loads from the stored name and models, whether or not this machine's library has it.
  Deleting a kit therefore never silences a module. 1.1.0 patches (kit uuid + overrides
  only) load unchanged.
- **Threads.** The library is UI-thread only (every caller is a menu). The audio thread
  reads `kit.models[]`, aligned ints; the strings in `KitState` are never read there.
- **Tests.** `tests/vx_drums/kit_test.cpp` covers `KitState` and `KitLibrary`.

