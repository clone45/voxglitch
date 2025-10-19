#include "plugin.hpp"
#include "osdialog.h"

// Include vgLib components if needed
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

// Include module headers
#include "Resomat/Resomat.hpp"
#include "Resomat/ResomatWidget.hpp"

// Create the model
Model* modelResomat = createModel<Resomat, ResomatWidget>("resomat");