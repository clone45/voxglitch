//
// Voxglitch "DigitalSequencer" module for VCV Rack
// by Bret Truchan
//
// Probably influenced by Nord Modular and Reaktor, but it's been too long for
// me to remember.
//
// Special thanks to Artem Leonov for his testing and suggestions.
// Special thanks to Marc Boulé for his help with reset trigger behavior.
// Special thanks to the entire VCV Rack community for their support.
//

// These two includes are required for Mac
#include <fstream>
#include <array>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sequencer/VoltageSequencerHistory.hpp"
#include "vgLib-2.0/sequencer/Sequencer.hpp"
#include "vgLib-2.0/sequencer/VoltageSequencer.hpp"
#include "vgLib-2.0/sequencer/GateSequencer.hpp"

using namespace vgLib_v2;

#include "DigitalSequencer/defines.h"
#include "DigitalSequencer/DigitalSequencer.hpp"
#include "DigitalSequencer/SequencerDisplay.hpp"
#include "DigitalSequencer/VoltageSequencerDisplay.hpp"
#include "DigitalSequencer/GateSequencerDisplay.hpp"
#include "DigitalSequencer/DigitalSequencerWidget.hpp"

Model* modelDigitalSequencer = createModel<DigitalSequencer, DigitalSequencerWidget>("digitalsequencer");


/*
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes are required for Mac
#include <fstream>
#include <array>

#include "DigitalSequencer/defines.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

// #include "vgLib-2.0/sequencer/VoltageSequencerHistory.hpp"
// #include "vgLib-2.0/sequencer/Sequencer.hpp"
// #include "vgLib-2.0/sequencer/VoltageSequencer.hpp"
// #include "vgLib-2.0/sequencer/GateSequencer.hpp"

using namespace vgLib_v2;

#include "DigitalSequencer/DigitalSequencer.hpp"
// #include "DigitalSequencer/SequencerDisplay.hpp"
// #include "DigitalSequencer/VoltageSequencerDisplay.hpp"
// #include "DigitalSequencer/GateSequencerDisplay.hpp"
#include "DigitalSequencer/DigitalSequencerWidget.hpp"

Model* modelDigitalSequencer = createModel<DigitalSequencer, DigitalSequencerWidget>("digitalsequencer");

*/
