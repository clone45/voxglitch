//
// Voxglitch "SamplerX8" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "Common/constants.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"

// #include "ByteBeat/calculator.hpp"
#include "ByteBeat/defines.h"
#include "ByteBeat/ByteBeat.hpp"
#include "ByteBeat/ByteBeatWidget.hpp"

Model* modelByteBeat = createModel<ByteBeat, ByteBeatWidget>("bytebeat");
