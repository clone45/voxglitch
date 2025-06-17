//
// Voxglitch "wav bank MC" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sample_mc.hpp"

using namespace vgLib_v2;

#include "WavBankMC/defines.h"
#include "WavBankMC/WavBankMC.hpp"
#include "WavBankMC/WavBankMCReadout.hpp"
#include "WavBankMC/MenuItemLoadBankMC.hpp"
#include "WavBankMC/WavBankMCWidget.hpp"

Model* modelWavBankMC = createModel<WavBankMC, WavBankMCWidget>("wavbankmc");
