//
// Voxglitch "Drum Randomizer" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/constants.h"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/dsp/Random.hpp"

#include "DrumRandomizer/DrumRandomizer.hpp"
#include "DrumRandomizer/DrumRandomizerReadoutWidget.hpp"
#include "DrumRandomizer/DrumRandomizerWidget.hpp"

Model* modelDrumRandomizer = createModel<DrumRandomizer, DrumRandomizerWidget>("drumrandomizer");
