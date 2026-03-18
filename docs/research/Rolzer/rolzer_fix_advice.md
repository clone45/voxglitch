 Summary: What the Current Implementation Gets Wrong and How to Fix It

  The Real Hardware Circuit (Per Stage)

  Each "roll" stage in the Rollz-5/Rolzer is a capacitor-coupled inverting transistor stage (BC547B NPN / BC557B PNP). The
  three actions at each node are:

  1. Charge: A timing capacitor slowly charges through a resistor toward the supply voltage (~9V). The charging follows an
  exponential RC curve — V(t) = V_target × (1 - e^(-t/RC)) — not a linear ramp.
  2. Threshold: When the capacitor voltage reaches the transistor's base-emitter threshold (~0.6V), the transistor switches
  on (saturates). Its collector drops from near Vcc to near ground.
  3. Inverted pulse to next stage: This collector voltage drop is AC-coupled through a coupling capacitor to the next
  transistor's base. The coupling cap delivers a sharp negative-going spike (measured at -9.4V on a 9V supply!) followed by
  a slow exponential rise back toward +1.2V. Each stage inverts — this is the critical detail.

  What the Current Code Misses

  Looking at Rolzer.hpp, the current process() loop has several significant simplifications:

  1. No inversion tracking — The code has no concept of polarity. Each node just accumulates charge and fires. This means
  there's no mechanism for even/odd rolls to behave differently, which is the entire point of the instrument. Even and odd
  rolls currently differ only in node count, not in any emergent behavior.
  2. Linear charging instead of exponential RC — nodeState[i] += (chargeRate - decayRate * nodeState[i]) * dt is a leaky
  integrator, which is closer to exponential than pure linear, but the constants aren't calibrated to real RC time
  constants. More importantly, the charge source is abstract rather than modeled on a capacitor charging toward a supply
  rail.
  3. Impulse coupling instead of AC coupling — When a node fires, the code injects a fixed charge (ringImpulse = 0.5f) into
  the next node. In the real circuit, the coupling is AC-coupled (capacitive): it transfers the change in voltage, not a
  fixed amount. The characteristic waveform at each brown/sandrode jack is a sharp negative spike followed by exponential
  rise — the code doesn't produce this at all.
  4. No negative voltages — The real circuit swings between -9.4V and +1.2V at the sandrode nodes. The code clamps to [0,
  1]. The negative voltage excursions are musically important — they're what create the "fuzz-burst" character when odd
  rolls go chaotic.
  5. Cross-patching is too simple — Real sandrode connections are capacitor-coupled, meaning they transfer transients (AC),
  not DC levels. The current patchCoupling = 0.3f fixed impulse doesn't capture this. In the real hardware, connecting two
  brown jacks creates mutual perturbation where each node's voltage changes affect the other — this is what makes
  cross-patched rolls create such complex evolving rhythms.

  What a More Faithful Model Would Look Like

  Here's a node model that captures the key hardware behaviors without oversampling:

  Per node, per sample:

    // 1. Exponential RC charging toward supply rail (like a real cap)
    //    RC time constant derived from E6 capacitor value
    float chargeAlpha = 1.0f - expf(-dt / rcTimeConstant);
    capVoltage += (supplyTarget - capVoltage) * chargeAlpha;

    // 2. Accumulate coupling from ring predecessor (AC-coupled, inverted)
    //    The coupling cap passes the CHANGE in the predecessor's voltage, inverted
    float predDelta = predecessorVoltage - predecessorPrevVoltage;  // differentiated
    capVoltage -= couplingStrength * predDelta;  // inverted: minus sign

    // 3. Accumulate coupling from cross-patch connections (AC-coupled)
    for each connection:
        float extDelta = connectedNodeVoltage - connectedNodePrevVoltage;
        capVoltage += patchCoupling * extDelta;

    // 4. Threshold and fire
    if (capVoltage >= threshold) {
        // Transistor saturates — collector drops sharply
        // This creates the negative spike at the output
        capVoltage = resetVoltage;  // sharp discharge (can go negative)
        fired = true;
    }

    // 5. Store previous voltage for AC coupling differentiation
    prevVoltage = capVoltage;

  Key differences from current code:

  ┌────────────────┬────────────────────────────────┬───────────────────────────────────────────────┐
  │     Aspect     │          Current Code          │                Faithful Model                 │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ Charging       │ Linear-ish accumulation        │ Exponential RC toward supply                  │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ Coupling       │ Fixed impulse injection (+0.5) │ AC-coupled: differentiated voltage, inverted  │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ Polarity       │ No inversion                   │ Each stage inverts (minus sign on coupling)   │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ Voltage range  │ [0, 1]                         │ Bipolar, allowing negative spikes             │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ Even/Odd chaos │ Doesn't emerge                 │ Emerges naturally from odd # of inversions    │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ Cross-patch    │ Fixed impulse                  │ AC-coupled (passes transients only)           │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ Waveform       │ Sawtooth-ish ramp              │ Sharp negative spike + exponential rise       │
  ├────────────────┼────────────────────────────────┼───────────────────────────────────────────────┤
  │ RC constants   │ Abstract chargeRate table      │ Derived from E6 capacitor values × resistance │
  └────────────────┴────────────────────────────────┴───────────────────────────────────────────────┘

  E6 Capacitor → RC Time Constant Mapping

  The Rolzer uses the E6 series to grade tempos. With a typical charge resistor (the schematics suggest values in the
  10K-100K range), the time constants would be:

  ┌───────────┬───────────────────┬───────────────────────┐
  │ Cap Value │ With 47K Resistor │ Approximate Frequency │
  ├───────────┼───────────────────┼───────────────────────┤
  │ 1.0µF     │ 47ms              │ ~21 Hz                │
  ├───────────┼───────────────────┼───────────────────────┤
  │ 1.5µF     │ 70ms              │ ~14 Hz                │
  ├───────────┼───────────────────┼───────────────────────┤
  │ 2.2µF     │ 103ms             │ ~9.7 Hz               │
  ├───────────┼───────────────────┼───────────────────────┤
  │ 3.3µF     │ 155ms             │ ~6.5 Hz               │
  ├───────────┼───────────────────┼───────────────────────┤
  │ 4.7µF     │ 221ms             │ ~4.5 Hz               │
  ├───────────┼───────────────────┼───────────────────────┤
  │ 6.8µF     │ 320ms             │ ~3.1 Hz               │
  ├───────────┼───────────────────┼───────────────────────┤
  │ 10.0µF    │ 470ms             │ ~2.1 Hz               │
  └───────────┴───────────────────┴───────────────────────┘

  These are for even rolls. Odd rolls would naturally oscillate much faster (potentially ultrasonic ~100kHz on real
  hardware, but at audio rate simulation they'd produce high-frequency chaotic patterns within the Nyquist limit).

  The Sandrode (Brown Jack) Model

  The sandrode is the junction point at the coupling capacitor between stages. Electrically it:
  - Outputs: the voltage across the coupling cap (the sharp spike + exponential rise waveform)
  - Inputs: any external current injected here directly perturbs the capacitor voltage, affecting when the next stage fires
  - No current limiting resistor on brown jacks (unlike orange jacks which have one) — this is essential for propagating the
   paradox wave

  For VCV Rack, this means the output voltage at each node should be the actual capacitor voltage (scaled to ±5V or 0-10V),
  and inputs should inject current proportional to the external voltage directly into the capacitor state.

  Summary of Recommended Changes

  1. Replace the node model with capacitor voltage + exponential RC charging
  2. Add inversion to the ring coupling (each stage inverts the pulse to the next)
  3. Use AC coupling (differentiated/transient) instead of fixed impulse for both ring and cross-patch connections
  4. Allow bipolar voltage at nodes (the negative spike is musically essential)
  5. Derive RC time constants from actual E6 capacitor values
  6. Output the capacitor waveform at each node for the sandrode jacks — this is what gives the Rolzer its characteristic
  sound

  These changes should make even rolls produce stable periodic pulse trains and odd rolls produce genuine chaotic behavior
  that emerges from the topology — no special-casing needed.

  Sources:
  - https://pugix.com/synth/rollz-5-schematics/
  - https://pugix.com/synth/diy-5-6-roolz/
  - https://www.ciat-lonbarde.net/ciat-lonbarde/rolzer/index.html
  - https://ciat-lonbarde.net/ciat-lonbarde/rollz5/index.html
  - https://www.perfectcircuit.com/ciat-lonbarde-rolzer.html
  - https://www.modwiggler.com/forum/viewtopic.php?t=164239
  - https://modwiggler.com/forum/viewtopic.php?t=135235
  - https://modwiggler.com/forum/viewtopic.php?t=232686
  - http://ciat-lonbarde.net/plumbutter/labrolzpapersz.pdf