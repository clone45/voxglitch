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

#include "Common/components/VoxglitchComponents.hpp"

namespace dsxp
{
  #include "DigitalSequencerXP/defines.h"
  #include "DigitalSequencerXP/Sequencer.hpp"
  #include "DigitalSequencerXP/VoltageSequencer.hpp"
  #include "DigitalSequencerXP/GateSequencer.hpp"
  #include "DigitalSequencerXP/DigitalSequencerXP.hpp"
  #include "DigitalSequencerXP/SequencerDisplay.hpp"
  #include "DigitalSequencerXP/VoltageSequencerDisplayXP.hpp"
  #include "DigitalSequencerXP/GateSequencerDisplayXP.hpp"
  #include "DigitalSequencerXP/DigitalSequencerXPWidget.hpp"
}

Model* modelDigitalSequencerXP = createModel<dsxp::DigitalSequencerXP, dsxp::DigitalSequencerXPWidget>("digitalsequencerxp");
