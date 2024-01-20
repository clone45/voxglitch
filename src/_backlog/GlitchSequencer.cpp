// TODO: add ability to clear a pattern completely
// TODO: pattern shift using arrow keys

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v1;

#include "GlitchSequencer/defines.h"
#include "GlitchSequencer/CellularAutomatonSequencer.hpp"
#include "GlitchSequencer/GlitchSequencer.hpp"
#include "GlitchSequencer/CellularAutomatonDisplay.hpp"
#include "GlitchSequencer/LengthReadoutDisplay.hpp"
#include "GlitchSequencer/GlitchSequencerWidget.hpp"

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
