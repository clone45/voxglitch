chromatic_scale = [0.0, 0.083, 0.167, 0.25, 0.333, 0.417, 0.5, 0.583, 0.667, 0.75, 0.833, 0.917]
major_scale = [0.0, 0.167, 0.333, 0.5, 0.667, 0.833, 1.0]
natural_minor_scale = [0.0, 0.167, 0.25, 0.5, 0.583, 0.75, 0.917]
harmonic_minor_scale = [0.0, 0.167, 0.25, 0.5, 0.583, 0.75, 1.0]
melodic_minor_scale = [0.0, 0.167, 0.25, 0.5, 0.667, 0.833, 1.0]
dorian_scale = [0.0, 0.167, 0.25, 0.5, 0.667, 0.75, 0.917]
phrygian_scale = [0.0, 0.083, 0.25, 0.5, 0.583, 0.75, 0.917]
lydian_scale = [0.0, 0.167, 0.333, 0.417, 0.667, 0.833, 1.0]
mixolydian_scale = [0.0, 0.167, 0.333, 0.5, 0.667, 0.75, 0.917]
locrian_scale = [0.0, 0.083, 0.25, 0.417, 0.583, 0.75, 0.917]
pentatonic_major_scale = [0.0, 0.167, 0.333, 0.583, 0.75]

mystical = [0.0, 0.25, 0.333, 0.583, 0.75, 0.917]
dreamy = [0.0, 0.167, 0.417, 0.5, 0.667, 0.833]
japanese = [0.0, 0.167, 0.333, 0.5, 0.667, 0.917]
exotic = [0.0, 0.083, 0.25, 0.5, 0.667, 0.75, 1.0]




# Extrapolate the voltage values for the 21 octaves, ranging from -10 to 10 volts
octaves = []
for octave in range(-10, 11):
    octave_start_voltage = octave * 1.0
    octave_voltages = [round(volt + octave_start_voltage, 3) for volt in pentatonic_major_scale]
    octaves.append(octave_voltages)

# Print the voltage values for the 21 octaves
for octave_voltages in octaves:
    print(", ".join([str(voltage) for voltage in octave_voltages]))
