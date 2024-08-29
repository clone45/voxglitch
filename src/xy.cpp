//
// Voxglitch "xy" module for VCV Rack
//
// TODO:
// * add reset input

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <vector>
#include "cmath"

#include "vgLib-2.0/constants.h"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "XY/defines.h"
#include "XY/XY.hpp"
#include "XY/XYDisplay.hpp"
#include "XY/XYWidget.hpp"

Model* modelXY = createModel<XY, XYWidget>("xy");
