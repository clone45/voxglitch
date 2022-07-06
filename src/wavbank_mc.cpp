//
// Voxglitch "wav bank MC" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample_mc.hpp"
#include "Common/VoxglitchSamplerModule.hpp"
#include "Common/VoxglitchSamplerModuleWidget.hpp"
#include "WavBankMC/defines.h"
#include "WavBankMC/WavBankMC.hpp"
#include "WavBankMC/WavBankMCReadout.hpp"
#include "WavBankMC/MenuItemLoadBankMC.hpp"
#include "WavBankMC/WavBankMCWidget.hpp"

Model* modelWavBankMC = createModel<WavBankMC, WavBankMCWidget>("wavbankmc");
