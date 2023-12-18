#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/constants.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "Hazumi/defines.h"
#include "Hazumi/HazumiSequencer.hpp"
#include "Hazumi/Hazumi.hpp"
#include "Hazumi/HazumiSequencerDisplay.hpp"
#include "Hazumi/HazumiWidget.hpp"

Model* modelHazumi = createModel<Hazumi, HazumiWidget>("hazumi");
