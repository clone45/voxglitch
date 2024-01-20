#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v1;

#include "Hazumi/defines.h"
#include "Hazumi/HazumiSequencer.hpp"
#include "Hazumi/Hazumi.hpp"
#include "Hazumi/HazumiSequencerDisplay.hpp"
#include "Hazumi/HazumiWidget.hpp"

Model* modelHazumi = createModel<Hazumi, HazumiWidget>("hazumi");
