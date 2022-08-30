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

#include "DigitalSequencer/defines.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/sequencer/Sequencer.hpp"
#include "Common/sequencer/VoltageSequencer.hpp"

namespace dseq
{

  // #include "DigitalSequencer/VoltageSequencer.hpp"
  #include "DigitalSequencer/GateSequencer.hpp"
  #include "DigitalSequencer/DigitalSequencer.hpp"
  #include "DigitalSequencer/SequencerDisplay.hpp"
  #include "DigitalSequencer/VoltageSequencerDisplay.hpp"
  #include "DigitalSequencer/GateSequencerDisplay.hpp"
  #include "DigitalSequencer/DigitalSequencerWidget.hpp"
}

Model* modelDigitalSequencer = createModel<dseq::DigitalSequencer, dseq::DigitalSequencerWidget>("digitalsequencer");
