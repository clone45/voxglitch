//
// Voxglitch "wav bank MC" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/sample_mc.hpp"
#include "WavBankMC/defines.h"
#include "WavBankMC/WavBankMC.hpp"
#include "WavBankMC/WavBankMCReadout.hpp"
#include "WavBankMC/MenuItemLoadBankMC.hpp"
#include "WavBankMC/WavBankMCWidget.hpp"

Model* modelWavBankMC = createModel<WavBankMC, WavBankMCWidget>("wavbankmc");
