#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "DualSlope/defines.h"
#include "DualSlope/DualSlope.hpp"
#include "DualSlope/DualSlopeCanvas.hpp"
#include "DualSlope/DualSlopeWidget.hpp"

Model* modelDualSlope = createModel<DualSlope, DualSlopeWidget>("dualslope");
