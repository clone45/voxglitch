//
// Voxglitch "OneShot" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sample_mc.hpp"

using namespace vgLib_v2;

#include "OneShot/defines.h"
#include "OneShot/OneShot.hpp"
#include "OneShot/MenuItemLoadBankOneShot.hpp"
#include "OneShot/OneShotWidget.hpp"

Model* modelOneShot = createModel<OneShot, OneShotWidget>("one_shot");
