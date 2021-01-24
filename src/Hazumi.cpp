#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Hazumi/defines.h"
#include "Hazumi/HazumiSequencer.hpp"
#include "Hazumi/Hazumi.hpp"
#include "Hazumi/HazumiSequencerDisplay.hpp"
#include "Hazumi/HazumiWidget.hpp"

Model* modelHazumi = createModel<Hazumi, HazumiWidget>("hazumi");
