// TODO: add ability to clear a pattern completely
// TODO: pattern shift using arrow keys

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/constants.h"
#include "GlitchSequencer/defines.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "GlitchSequencer/CellularAutomatonSequencer.hpp"
#include "GlitchSequencer/GlitchSequencer.hpp"
#include "GlitchSequencer/CellularAutomatonDisplay.hpp"
#include "GlitchSequencer/LengthReadoutDisplay.hpp"
#include "GlitchSequencer/GlitchSequencerWidget.hpp"

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
