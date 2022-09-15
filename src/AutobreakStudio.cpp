//
// Voxglitch "Autobreak Studio" module for VCV Rack
//

#include <fstream>

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"
#include "Common/dsp/DeclickFilter.hpp"
#include "Common/dsp/StereoPan.hpp"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/sequencer/Sequencer.hpp"
#include "Common/sequencer/VoltageSequencer.hpp"
#include "Common/sequencer/GateSequencer.hpp"

#include "AutobreakStudio/defines.h"
#include "AutobreakStudio/AutobreakMemory.hpp"
#include "AutobreakStudio/AutobreakStudio.hpp"
#include "AutobreakStudio/AutobreakStudioLoadSample.hpp"
#include "AutobreakStudio/SequencerDisplayABS.hpp"
#include "AutobreakStudio/VoltageSequencerDisplayABS.hpp"
#include "AutobreakStudio/VoltageToggleSequencerDisplay.hpp"
#include "AutobreakStudio/LcdTabsWidget.hpp"
#include "AutobreakStudio/WaveformWidget.hpp"
#include "AutobreakStudio/AutobreakStudioWidget.hpp"

Model* modelAutobreakStudio = createModel<AutobreakStudio, AutobreakStudioWidget>("AutobreakStudio");
