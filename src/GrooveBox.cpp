
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>

#include "GrooveBox/defines.h"
using namespace groove_box;

#include "Common/VoxglitchWidget.hpp"
#include "GrooveBox/SamplePlaybackSettings.hpp"
#include "GrooveBoxExpander/ExpanderToGrooveboxMessage.hpp"
#include "GrooveBox/GrooveboxToExpanderMessage.hpp"

// Sample players and such
#include "Common/sample.hpp"
#include "Common/common.hpp"
#include "Common/submodules.hpp"
#include "Common/DSP/ADSR.cpp"
// #include "Common/DSP/freeverb/revmodel.cpp"
#include "Common/DSP/SimpleDelay.hpp"
#include "Common/VoxglitchSamplerModule.hpp"
#include "Common/VoxglitchSamplerModuleWidget.hpp"

// Core components
#include "GrooveBox/SamplePlayer.hpp"
#include "GrooveBox/Track.hpp"
#include "GrooveBox/MemorySlot.hpp"
#include "GrooveBox/GrooveBox.hpp"

#include "GrooveBox/GrooveBoxWidget.hpp"

Model* modelGrooveBox = createModel<GrooveBox, GrooveBoxWidget>("groovebox");
