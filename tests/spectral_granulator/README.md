# tests/spectral_granulator

Out-of-Rack tools for the Spectral Granulator engine (`src/modules/Spectral Granulator/Spectral GranulatorEngine.hpp`,
which is Rack-free). The plugin itself is never built in this environment; these compile
the engine directly. Run from this directory.

    g++ -std=c++11 -O2 -Wall -Wextra -I ../../src -o engine_test engine_test.cpp -lm && ./engine_test
    g++ -std=c++11 -O2 -I ../../src -o lab lab.cpp -lm

- **engine_test.cpp** — synthetic signals with known answers (75 checks). BLEND 0 is the input
  exactly `FFT_SIZE` samples late; unison at 1x stretch reassembles the input within 1 dB;
  stretched noise keeps its level; IMPRINT barely on is transparent (the STFT path nulls
  against the delayed grains to about -97 dB); every pitch set with STEP, RESYNC, IMPRINT and
  all three steering amounts stays finite and bounded; TONAL reads 1.0 on a sine, 0.75 on a
  220 Hz saw and 0 on white noise; LEVEL reads its dB map; ONSET fires once per burst of a
  4 Hz burst train, never on silence, at most once on a steady tone; a FOCUS band that
  excludes the signal hears nothing; a patched RESYNC trigger brings the read head back; the
  control curves and their inverses agree; TUNE +12 turns 440 Hz into 880 Hz; SPREAD 0 is
  mono for a mono input and SPREAD 1 is not; full FEEDBACK is still sounding 4 s after the
  input stops and stays bounded; and the ends of the widened ranges (1 ms and 1 s grains,
  0.25 and 16 deep, 1 ms and 2 s steps, 128x, TUNE +/-48, full JITTER) stay finite and
  bounded. Exits 1 on failure.
- **lab.cpp** — the listening lab and the calibration tool.
  - `./lab stats <wav>...` prints, per clip, the 10/50/90th percentiles of the raw analysis
    quantities and the onset rate at SENS 0.25 / 0.5 / 0.75.
  - `./lab render <in.wav> <out_dir> [start_sec] [len_sec] [name_prefix ...]` writes one float
    WAV per row of the `VARIANTS` table, `00_dry.wav`, and `features.csv`; with name prefixes,
    only the variants whose names start with one of them. The table sets controls in real
    units through the engine's `*Norm` helpers. Edit it, rebuild, listen.

## Calibration (2026-11-21)

`spectral_granulator::Calibration` was set from 30 clips (48 kHz stereo: voice, drums, plucked
strings, piano, choir, flutes, full mixes), the first 60 s of each:

- **LEVEL**: band RMS medians ran -34 .. -15 dB, 10th..90th percentiles -44 .. -9 dB.
  Mapped -50 .. -6 dB to 0 .. 1.
- **BRIGHT**: magnitude-weighted log2 centroid medians 7.9 .. 11.0, extremes 6.6 .. 12.3.
  Mapped 7.0 .. 12.0 (128 Hz .. 4.1 kHz) to 0 .. 1.
- **TONAL**: whitened flatness medians 0.06 (solo classical) .. 0.41 (dense mixes), white
  noise 0.5 and up. Mapped 0.50 .. 0.12 to 0 .. 1. With a 1024-point frame a 220 Hz saw
  read 0.51 tonal and anything near 100 Hz read as noise (harmonics 2 bins apart), so
  TONAL has its own 2048-point frame: the saw reads 0.75.
- **ONSET**: the flux of sustained material (voice, flute, choir) peaks near 4x its median
  at 0.005 .. 0.02; drums peak at 10x and more, at 0.05 .. 0.2. The threshold is the running
  upper quartile (0.5 s) times a multiple, plus a floor, both exponential in SENS. Onsets
  per second at SENS 0.25 / 0.5 / 0.75: drums and busy mixes 4-6 / 6-7 / 7-8, plucked
  strings 1.5 / 2.5 / 3.5, a clear sung vocal 0.1 / 0.5 / 1.8, choir and drones ~0 until
  SENS is near the top. A first version used the running mean with a higher floor and gave
  no onsets at all on the vocal; the median fired 14 times a second through a narrow bass
  band, where the flux is zero half the time.

## Traps

- **The send starts where BLEND is.** The first control update sets the send without its
  5 ms slew; before that fix the first 5 ms of input reached the grains at BLEND 0 and the
  identity test heard it for seconds, at 4x stretch.
- **The grain gain is measured, not guessed.** The first version chose between the coherent
  and incoherent divisors from STRETCH alone. 5 ms grains 16 deep on the djembe clip came out
  5.5 dB hot (long periods, so the grains were in step) while 2 ms grains on the voice were
  right. The engine now measures the grains' cross terms and slides between the two.
- **FEEDBACK's limiter ceiling is a quarter of full scale.** A saturated loop feeds every grain
  the same waveform; with a full-scale ceiling the output sat at 2.0 rms.
- **128x with full JITTER reads from before the test signal began** for most of a 4 s run, so
  that case in `engine_test` is exempt from the "not silent" check.
- **IMPRINT with a long SMOOTH runs hot** (+6 dB on the djembe clip): the held live shape is
  a peak-hold, so it sits above the average shape. Not compensated yet.
- **Onset detection through a narrow, low band is coarse**: at 48 kHz a bin is 47 Hz, so a
  2-octave band at 90 Hz is four bins.
