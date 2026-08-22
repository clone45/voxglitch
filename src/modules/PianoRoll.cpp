//
// Voxglitch "PianoRoll" module for VCV Rack
//

#include <algorithm>
#include <fstream>
#include <map>
#include <cmath>
#include <random>
#include <set>
#include <vector>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "PianoRoll/PianoRollGeometry.hpp"
#include "PianoRoll/PianoRollNote.hpp"
#include "PianoRoll/PianoRollPalette.hpp"
#include "PianoRoll/PianoRollMidi.hpp"

#include "PianoRoll/PianoRoll.hpp"
#include "PianoRoll/PianoRollControlBar.hpp"
#include "PianoRoll/PianoRollEditorWidget.hpp"
#include "PianoRoll/PianoRollWidget.hpp"

Model *modelPianoRoll = createModel<PianoRoll, PianoRollWidget>("piano_roll");
