//
// Voxglitch "Autobreak Studio" module for VCV Rack
//

#include <fstream>

#include "plugin.hpp"
#include "osdialog.h"

/*
#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/sample.hpp"
#include "vgLib-1.0/dsp/DeclickFilter.hpp"
#include "vgLib-1.0/dsp/StereoPan.hpp"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"
#include "vgLib-1.0/sequencer/Sequencer.hpp"
#include "vgLib-1.0/sequencer/VoltageSequencer.hpp"
#include "vgLib-1.0/sequencer/GateSequencer.hpp"
#include "vgLib-1.0/widgets/WaveformModel.hpp"
#include "vgLib-1.0/widgets/WaveformWidget.hpp"

using namespace vgLib_v1;
*/

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/dsp/DeclickFilter.hpp"
#include "vgLib-2.0/dsp/StereoPan.hpp"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sequencer/Sequencer.hpp"
#include "vgLib-2.0/sequencer/VoltageSequencer.hpp"
#include "vgLib-2.0/sequencer/GateSequencer.hpp"
#include "vgLib-2.0/widgets/WaveformModel.hpp"
#include "vgLib-2.0/widgets/WaveformWidget.hpp"
#include "vgLib-2.0/sample/SampleLoaderMenuItem.hpp"
#include "vgLib-2.0/helpers/JSON.hpp"

using namespace vgLib_v2;

#include "AutobreakStudio/defines.h"
#include "AutobreakStudio/AutobreakMemory.hpp"
#include "AutobreakStudio/AutobreakStudio.hpp"
// #include "AutobreakStudio/AutobreakStudioLoadSample.hpp"
#include "AutobreakStudio/AutobreakStudioLoadFolder.hpp"
#include "AutobreakStudio/SequencerDisplayABS.hpp"
#include "AutobreakStudio/VoltageSequencerDisplayABS.hpp"
#include "AutobreakStudio/VoltageToggleSequencerDisplay.hpp"
#include "AutobreakStudio/LcdTabsWidget.hpp"

#include "AutobreakStudio/AutobreakStudioWidget.hpp"

Model* modelAutobreakStudio = createModel<AutobreakStudio, AutobreakStudioWidget>("AutobreakStudio");
