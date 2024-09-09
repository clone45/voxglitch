#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes seem only necessary for Mac:
#include <fstream>
#include <array>

#include "GrooveBox/defines.h"
using namespace groove_box;

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "GrooveBox/ParameterLockSettings.hpp"
#include "GrooveBoxExpander/ExpanderToGrooveboxMessage.hpp"
#include "GrooveBox/GrooveboxToExpanderMessage.hpp"

// Sample players and such
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/common.hpp"
#include "vgLib-2.0/dsp/ADSR.cpp"
#include "vgLib-2.0/dsp/SimpleDelay.hpp"
#include "vgLib-2.0/dsp/StereoFadeOut.hpp"
#include "vgLib-2.0/dsp/StereoPan.hpp"
#include "vgLib-2.0/dsp/Filter.hpp"
#include "vgLib-2.0/dsp/FastSlewLimiter.hpp"
#include "vgLib-2.0/dsp/Random.hpp"
#include "vgLib-2.0/SamplePlayer.hpp"

using namespace vgLib_v2;

// Core components
#include "GrooveBox/widgets/LCDColorScheme.hpp"
#include "GrooveBox/TrackModel.hpp"
#include "GrooveBox/Track.hpp"
#include "GrooveBox/MemorySlot.hpp"
#include "GrooveBox/GrooveBox.hpp"

#include "GrooveBox/GrooveBoxWidget.hpp"

Model* modelGrooveBox = createModel<GrooveBox, GrooveBoxWidget>("groovebox");
