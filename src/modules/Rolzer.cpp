#include "../plugin.hpp"
#include "osdialog.h"

// Include vgLib components
#include "../vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

// Include module headers
#include "Rolzer/Rolzer.hpp"
#include "Rolzer/RolzerCanvas.hpp"
#include "Rolzer/RolzerWidget.hpp"

// Create the model
Model* modelRolzer = createModel<Rolzer, RolzerWidget>("rolzer");
