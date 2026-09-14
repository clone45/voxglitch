# tests/vx_drums

Out-of-Rack tests for the VX Drums kit model (`VXDrumKit.hpp`, Rack-free).
The plugin itself is never built in this environment.

    g++ -std=c++11 -O2 -I ../../src/modules/VXDrums -o kit_test kit_test.cpp && ./kit_test
    g++ -std=c++11 -O2 -I ../../src/modules/VXDrums -o sample_test sample_test.cpp && ./sample_test

- **kit_test.cpp** — `KitState` (factory resolution by uuid, identity by
  uuid versus equality) and `KitLibrary` (add / find / update / rename /
  remove, and what each refuses: duplicate or empty uuids, unknown uuids,
  empty names, no-op updates). The JSON file behind the library
  (`VXDrumKitStore.hpp`) is Rack-bound and is not covered.
- **sample_test.cpp** — `SampleModel` (`VXDrumVoices.hpp`): silence without a
  file, bit-exact passthrough at full DECAY, the T60 decay envelope (60 dB
  down at DECAY seconds, only ever shortens a file), semitone tune, file-rate to
  engine-rate conversion, the start offset, retrigger, choke, reset, swapping
  the file, and the model's spec and bank wiring.
