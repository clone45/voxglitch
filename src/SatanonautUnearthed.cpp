//
// Voxglitch "SatanonautUnearthed" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

// #include "ByteBeat/calculator.hpp"
#include "SatanonautUnearthed/SatanonautUnearthed.hpp"
#include "SatanonautUnearthed/SatanonautUnearthedWidget.hpp"

Model* modelSatanonautUnearthed = createModel<SatanonautUnearthed, SatanonautUnearthedWidget>("satanonaut_unearthed");
