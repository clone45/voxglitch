// TODO: add ability to clear a pattern completely
// TODO: pattern shift using arrow keys

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include "cmath"

#include "GlitchSequencer/defines.h"
#include "GlitchSequencer/CellularAutomatonSequencer.hpp"
#include "GlitchSequencer/GlitchSequencer.hpp"
#include "GlitchSequencer/CellularAutomatonDisplay.hpp"
#include "GlitchSequencer/LengthReadoutDisplay.hpp"
#include "GlitchSequencer/GlitchSequencerWidget.hpp"

Model* modelGlitchSequencer = createModel<GlitchSequencer, GlitchSequencerWidget>("glitchsequencer");
