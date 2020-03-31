//
// Voxglitch "xy" module for VCV Rack
//
// TODO:
// * add reset input

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include "Common/dr_wav.h"
#include <vector>
#include "cmath"

#include "XY/defines.h"
#include "XY/XY.hpp"
#include "XY/XYDisplay.hpp"
#include "XY/XYWidget.hpp"

Model* modelXY = createModel<XY, XYWidget>("xy");
