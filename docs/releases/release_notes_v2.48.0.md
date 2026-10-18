# Voxglitch 2.48.0

Now available in the VCV Rack library.

## Euclid (new module)

- **A Euclidean rhythm sequencer** in 9 HP. Set **HITS** and **STEPS** (up to 32) and the hits are spread as evenly as possible around the cycle; **ROT** turns the whole pattern. Clock it from **CLK**, reset it from **RST**.
- **TRIG** fires a pulse on every hit, with its own rising edge even when hits come faster than the pulse. Trigger length is 1 ms by default and selectable up to 200 ms in the right-click menu.
- **CV over all three knobs**, each through its own attenuverter. The attenuverters start at 0, so a cable in a CV jack is silent until its attenuverter is turned up.
- **The ring** shows the pattern that is actually playing, CV included, with the playhead on it. Drag the rim to rotate the pattern; double-click it to set the rotation back to 0.
- Reset follows the VCV Rack standard: a reset puts Euclid on step 1 and plays it, the next clock is step 2, and clock pulses within 1 ms of a reset are ignored, so it stays in step with VCV's SEQ3, Impromptu Clocked and the other sequencers built to that rule.
- The rhythms are the ones from the Voxglitch Devices and vxsynth Euclid modules, so patterns carry over.

## VX Drums

- **Load your own samples.** Any column can play an audio file instead of a circuit: **Kit Customization** > the column's model submenu > **Load sample…** (WAV or MP3, stereo summed to mono, any sample rate). TUNE pitches the sample in semitones, DECAY shortens it (fully up plays the file as recorded) and SHAPE moves the start point. Saved kits and patches carry the samples.
- **KIT CV input**, right of ACC. 0 to 10 V in equal bands across the kit list, the factory kits first and then your saved kits, with a little hysteresis at the band edges. While patched the CV owns the kit; unpatch it and the kit you chose yourself comes back, overrides included. A user request.

## VX Drum Sequencer

- **TWK input: Tweak.** 0-10 V evolves the pattern as it plays without changing what you drew. Each 1.25 V step applies one more musical edit, written for each lane's instrument, to the lanes ticked in **Tweak settings**. No randomness: the same level on the same pattern always sounds the same. An experiment in this release; the edit lists may change.
- **Rotate pattern** on the panel menu shifts the playing memory left or right by 1, 2 or 4 steps, wrapping within its length. One undo step.
- **A trigger jack for RND**, right of the button. A mono cable randomizes the pattern like the button; a polyphonic cable re-rolls one lane per channel, 1 to 6 the voices and 7 the accent lane, so a divider can re-roll the hats every few bars while the kick stays. A user request.

- **Reset now follows the VCV Rack standard.** A reset puts the sequencer on step 1 and plays it at the reset instant; the next clock pulse is step 2; clock pulses arriving within 1 ms of a reset are treated as the reset's own clock and ignored. Loading a patch (and Initialize) puts the sequencer on step 1 without playing it, so the first clock after loading plays step 2 unless a reset arrives first.
- This is the fix for the "does not sync" report. The previous rule (nothing at the reset, step 1 on the next clock) was one step behind SEQ3 and Impromptu Clocked whenever a reset arrived without a clock pulse of its own: after every stop / reset / run with Clocked at its defaults, and after every running reset that landed while the clock output was high. Chains of sequencers follow the same rule from the head.
- The manual's "Reset and the First Step" section describes each situation.

## Piano Roll

- **Focus active track** on the right-click menu hides the other tracks' ghost notes, so only the active track is drawn. A user request.

## Compatibility

Older patches load unchanged; the new KIT CV and RND jacks are appended after the existing ones, so every cable in a saved patch lands where it did. VX Drums patches upgrade to kit format 1.4.0, which an older plugin still reads as the kit that was playing. A VX Drum Sequencer patch that relied on the old reset timing (step 1 on the first clock after a reset) now plays step 1 at the reset and step 2 on that clock, which is the corrected behaviour; patch **CLK** and **RST** from the same clock master and every reset lands on the downbeat.
