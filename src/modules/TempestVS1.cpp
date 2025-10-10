// TempestVS1 module for VCV Rack
// MIDI controller interface for Dave Smith Instruments Tempest

#include "plugin.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "TempestVS1/TempestVS1.hpp"
#include "TempestVS1/TempestVS1Widget.hpp"

Model* modelTempestVS1 = createModel<TempestVS1, TempestVS1Widget>("tempest-vs1");
