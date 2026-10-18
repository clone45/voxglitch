#pragma once
//
// ResetClockWindow — the VCV Rack reset/clock rule, written once. Rack-free.
//
// The VCV Voltage Standards (https://vcvrack.com/manual/VoltageStandards) say:
//
//   "Each cable in Rack induces a 1-sample delay [...] a pulse sent through a
//    utility module and then to a sequencer's CLOCK input will arrive one sample
//    later than the same pulse sent directly to the sequencer's RESET input. This
//    will cause the sequencer to reset to step 1, and one sample later, advance to
//    step 2, which is undesired behavior. Therefore, modules with a CLOCK and RESET
//    input, or similar variants, should ignore CLOCK triggers up to 1 ms after
//    receiving a RESET trigger."
//
// The model behind that sentence, which Fundamental SEQ3 and Impromptu Modular
// implement and Impromptu Clocked's defaults assume:
//
//   * a reset puts the sequencer ON step 1 and plays it, at the reset instant;
//   * every clock edge inside the window that follows is the reset's own clock
//     and is swallowed;
//   * the next clock edge after the window is step 2's;
//   * power-up / patch load is an implicit reset (step 1 current, window armed,
//     nothing fires).
//
// Sequencers that instead go to "no step" on reset and play step 1 on the next
// clock are one step behind those modules whenever a reset arrives without a
// coincident edge (a stopped Clocked held high, a running reset that lands in
// the clock's high half). See docs/implementation_plans/euclid-migration.md §4.1.
//
// Per sample, in this order:
//
//   if (reset edge)              window.arm(ResetClockWindow::samplesFor(sampleRate));
//                                 ... go to step 1 and play it
//   if (clock edge && !window.open())   ... advance and play
//   window.tick();
//
// arm() on the reset sample and the same-sample clock check both see open() ==
// true, so a same-sample reset + clock plays step 1 once. The window covers the
// reset sample plus ceil(1 ms) - 1 more: 48 samples at 48 kHz, 45 at 44.1 kHz.
// That is the span of SEQ3's float resetPulse (trigger(1e-3f) read through
// PulseGenerator::process) at 44.1 and 48 kHz and at every other Rack rate
// but three: at 96, 192 and 384 kHz SEQ3's float runs one sample longer
// (97 / 193 / 385), so the boundary sample there is the one place the two can
// differ. Measured at all fifteen Rack rates, 11025 to 768000, under -O2 and
// under Rack's own -O3 -funsafe-math-optimizations -march=nehalem, with the
// same result; tests/euclid/seq3_parity_test.cpp pins the 96 kHz case.
//

namespace vgLib_v2
{

struct ResetClockWindow
{
    static const int MIN_SAMPLES = 1;

    // Samples for the standard's 1 ms at this rate (sampleRate / 1000, rounded
    // up), never fewer than one. Integer arithmetic on purpose: 0.001f is not
    // representable, 0.001f * 48000.f is 48.0000038 in float and ceil() of that
    // is 49, one sample past the contract. (rate + 999) / 1000 rounds up
    // exactly at every integer rate: 44100 -> 45, 48000 -> 48, 96000 -> 96,
    // 192000 -> 192.
    static int samplesFor(float sampleRate)
    {
        const int n = ((int)sampleRate + 999) / 1000;
        return n < MIN_SAMPLES ? MIN_SAMPLES : n;
    }

    int remaining = 0;

    // Arm (or re-arm) the window. A reset inside an open window extends it to
    // the full length again; it never shortens.
    void arm(int samples)
    {
        if (samples > remaining) remaining = samples;
    }

    bool open() const { return remaining > 0; }

    // Call once per sample, after the clock check.
    void tick()
    {
        if (remaining > 0) remaining--;
    }

    void clear() { remaining = 0; }
};

} // namespace vgLib_v2
