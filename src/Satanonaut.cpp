//
// Voxglitch "Satanonaut" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

// #include "ByteBeat/calculator.hpp"
#include "Satanonaut/defines.h"
#include "Satanonaut/SatanonautAudioBuffer.hpp"
#include "Satanonaut/SatanonautStereoAudioBuffer.hpp"
#include "Satanonaut/Satanonaut.hpp"
#include "Satanonaut/SatanonautEffectReadout.hpp"
#include "Satanonaut/DrawTimer.h"
#include "Satanonaut/SatanonautWidget.hpp"

Model* modelSatanonaut = createModel<Satanonaut, SatanonautWidget>("satanonaut");
