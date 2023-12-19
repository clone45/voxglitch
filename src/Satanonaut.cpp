//
// Voxglitch "Satanonaut" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v1;

// #include "ByteBeat/calculator.hpp"
#include "Satanonaut/defines.h"
#include "Satanonaut/SatanonautAudioBuffer.hpp"
#include "Satanonaut/SatanonautStereoAudioBuffer.hpp"
#include "Satanonaut/Satanonaut.hpp"
#include "Satanonaut/SatanonautEffectReadout.hpp"
#include "Satanonaut/SatanonautWidget.hpp"

Model* modelSatanonaut = createModel<Satanonaut, SatanonautWidget>("satanonaut");
