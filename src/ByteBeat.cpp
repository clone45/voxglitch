//
// Voxglitch "SamplerX8" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

// #include "ByteBeat/calculator.hpp"
#include "ByteBeat/defines.h"
#include "ByteBeat/SegmentReadoutWidget.hpp"
#include "ByteBeat/ByteBeat.hpp"
#include "ByteBeat/ByteBeatWidget.hpp"

Model* modelByteBeat = createModel<ByteBeat, ByteBeatWidget>("bytebeat");
