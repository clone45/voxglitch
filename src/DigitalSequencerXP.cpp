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
#include <fstream>
#include <array>

#include "DigitalSequencerXP/defines.h"
using namespace digital_sequencer_xp;

#include "Common/VoxglitchWidget.hpp"
/*
#include "DigitalSequencerXP/Sequencer.hpp"
#include "DigitalSequencerXP/VoltageSequencer.hpp"
#include "DigitalSequencerXP/GateSequencer.hpp"
*/
#include "DigitalSequencerXP/DigitalSequencerXP.hpp"
/*
#include "DigitalSequencerXP/SequencerDisplay.hpp"
#include "DigitalSequencerXP/VoltageSequencerDisplay.hpp"
#include "DigitalSequencerXP/GateSequencerDisplay.hpp"
*/
#include "DigitalSequencerXP/DigitalSequencerXPWidget.hpp"

Model* modelDigitalSequencerXP = createModel<DigitalSequencerXP, DigitalSequencerXPWidget>("digitalsequencerxp");
