#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-2.0/constants.h"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "GlitchSequencer/defines.h"
#include "GlitchSequencer/PatternMemory.hpp"
#include "GlitchSequencer/CellularAutomatonSequencer.hpp"
#include "GlitchSequencer/GlitchSequencer.hpp"
#include "GlitchSequencer/CellularAutomatonDisplay.hpp"
#include "GlitchSequencer/GlitchSequencerWidget.hpp"

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
