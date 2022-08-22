/*
  // todo: add intro documentation


*/


#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "GrooveBox/defines.h"
using namespace groove_box;

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "GrooveBox/SamplePlaybackSettings.hpp"
#include "GrooveBoxExpander/ExpanderToGrooveboxMessage.hpp"
#include "GrooveBox/GrooveboxToExpanderMessage.hpp"

// Sample players and such
#include "Common/sample.hpp"
#include "Common/common.hpp"
#include "Common/dsp/ADSR.cpp"
#include "Common/dsp/SimpleDelay.hpp"
#include "Common/dsp/StereoFadeOut.hpp"
#include "Common/dsp/StereoPan.hpp"
#include "Common/SamplePlayer.hpp"

// Core components
#include "GrooveBox/Track.hpp"
#include "GrooveBox/MemorySlot.hpp"
#include "GrooveBox/GrooveBox.hpp"

#include "GrooveBox/GrooveBoxWidget.hpp"

Model* modelGrooveBox = createModel<GrooveBox, GrooveBoxWidget>("groovebox");
