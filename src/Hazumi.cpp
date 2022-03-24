#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/components/PNGPanel.hpp"
#include "Common/components/VoxglitchPorts.hpp"
#include "Common/components/VoxglitchPanel.hpp"
#include "Common/components/VoxglitchSwitches.hpp"
#include "Common/components/VoxglitchModuleWidget.hpp"
#include "Common/VoxglitchWidget.hpp"

#include "Hazumi/defines.h"
#include "Hazumi/HazumiSequencer.hpp"
#include "Hazumi/Hazumi.hpp"
#include "Hazumi/HazumiSequencerDisplay.hpp"
#include "Hazumi/HazumiWidget.hpp"

Model* modelHazumi = createModel<Hazumi, HazumiWidget>("hazumi");
