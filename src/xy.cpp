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

#include "Common/components/PNGPanel.hpp"
#include "Common/components/VoxglitchPorts.hpp"
#include "Common/components/VoxglitchPanel.hpp"
#include "Common/components/VoxglitchSwitches.hpp"
#include "Common/components/VoxglitchModuleWidget.hpp"
#include "Common/VoxglitchWidget.hpp"
#include "XY/defines.h"
#include "XY/XY.hpp"
#include "XY/XYDisplay.hpp"
#include "XY/XYWidget.hpp"

Model* modelXY = createModel<XY, XYWidget>("xy");
