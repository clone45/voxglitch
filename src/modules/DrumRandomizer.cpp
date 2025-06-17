//
// Voxglitch "Drum Randomizer" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "vgLib-1.0/constants.h"


#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/dsp/Random.hpp"

using namespace vgLib_v2;

#include "DrumRandomizer/DrumRandomizer.hpp"
#include "DrumRandomizer/DrumRandomizerReadoutWidget.hpp"
#include "DrumRandomizer/DrumRandomizerWidget.hpp"

Model* modelDrumRandomizer = createModel<DrumRandomizer, DrumRandomizerWidget>("drumrandomizer");
