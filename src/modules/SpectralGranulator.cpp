//
// Voxglitch "Spectral Granulator" module for VCV Rack.
//
// A granular smear steered by the input's own spectrum: an FFT reduces the
// input to an onset trigger and three slow signals (level, brightness,
// tonality), each with an output jack, and those are normalled into the CV
// inputs of the grain controls; every knob has a CV jack. SpectralGranulator/SpectralGranulatorEngine.hpp is the
// design record and all of the DSP.
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "plugin.hpp"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "SpectralGranulator/SpectralGranulatorEngine.hpp"
#include "SpectralGranulator/SpectralGranulator.hpp"
#include "SpectralGranulator/SpectralGranulatorDisplay.hpp"
#include "SpectralGranulator/SpectralGranulatorWidget.hpp"

Model* modelSpectralGranulator = createModel<SpectralGranulator, SpectralGranulatorWidget>("spectral_granulator");
