//
// Voxglitch "wav bank MC" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"

#include "WavBankMC/defines.h"
#include "WavBankMC/WavBankMC.hpp"
#include "WavBankMC/WavBankMCReadout.hpp"
#include "WavBankMC/MenuItemLoadBank.hpp"
#include "WavBankMC/WavBankMCWidget.hpp"

Model* modelWavBankMC = createModel<WavBankMC, WavBankMCWidget>("wavbankmc");
