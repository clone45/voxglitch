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

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/sequencer/Sequencer.hpp"
#include "Common/sequencer/VoltageSequencer.hpp"

#include "DigitalSequencerXP/defines.h"
#include "DigitalSequencerXP/GateSequencer.hpp"
#include "DigitalSequencerXP/DigitalSequencerXP.hpp"
#include "DigitalSequencerXP/SequencerDisplay.hpp"
#include "DigitalSequencerXP/VoltageSequencerDisplayXP.hpp"
#include "DigitalSequencerXP/GateSequencerDisplayXP.hpp"
#include "DigitalSequencerXP/DigitalSequencerXPWidget.hpp"

Model* modelDigitalSequencerXP = createModel<DigitalSequencerXP, DigitalSequencerXPWidget>("digitalsequencerxp");
