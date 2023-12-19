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

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes are required for Mac
#include <fstream>
#include <array>

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"
#include "vgLib-1.0/sequencer/Sequencer.hpp"
#include "vgLib-1.0/sequencer/VoltageSequencer.hpp"

using namespace vgLib_v1;

#include "DigitalSequencerXP/defines.h"
#include "DigitalSequencerXP/GateSequencer.hpp"
#include "DigitalSequencerXP/DigitalSequencerXP.hpp"
#include "DigitalSequencerXP/SequencerDisplay.hpp"
#include "DigitalSequencerXP/VoltageSequencerDisplayXP.hpp"
#include "DigitalSequencerXP/GateSequencerDisplayXP.hpp"
#include "DigitalSequencerXP/DigitalSequencerXPWidget.hpp"

Model* modelDigitalSequencerXP = createModel<DigitalSequencerXP, DigitalSequencerXPWidget>("digitalsequencerxp");
