//
// Voxglitch "DigitalDrumSequencer" module for VCV Rack
// by Bret Truchan
//
// Probably influenced by Nord Modular and Reaktor, but it's been too long for
// me to remember.
//
// Special thanks to Artem Leonov for his testing and suggestions.
// Special thanks to Marc Boulé for his help with reset trigger behavior.
// Special thanks to the entire VCV Rack community for their support.
//

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>


#include "Common/submodules.hpp"

#include "DigitalDrumSequencer/defines.h"
#include "DigitalDrumSequencer/Sequencer.hpp"
#include "DigitalDrumSequencer/VoltageSequencer.hpp"
#include "DigitalDrumSequencer/GateSequencer.hpp"
#include "DigitalDrumSequencer/DigitalDrumSequencer.hpp"
#include "DigitalDrumSequencer/SequencerDisplay.hpp"
#include "DigitalDrumSequencer/VoltageSequencerDisplay.hpp"
#include "DigitalDrumSequencer/GateSequencerDisplay.hpp"
#include "DigitalDrumSequencer/DigitalDrumSequencerWidget.hpp"

Model* modelDigitalDrumSequencer = createModel<DigitalDrumSequencer, DigitalDrumSequencerWidget>("DigitalDrumSequencer");
