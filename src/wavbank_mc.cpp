//
// Voxglitch "wav bank MC" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/components/PNGPanel.hpp"
#include "Common/components/VoxglitchPorts.hpp"
#include "Common/components/VoxglitchKnobs.hpp"
#include "Common/components/VoxglitchSwitches.hpp"
#include "Common/components/VoxglitchPanel.hpp"
#include "Common/sample_mc.hpp"

#include "WavBankMC/defines.h"
#include "WavBankMC/WavBankMC.hpp"
#include "WavBankMC/WavBankMCReadout.hpp"
#include "WavBankMC/MenuItemLoadBankMC.hpp"
#include "WavBankMC/WavBankMCWidget.hpp"

Model* modelWavBankMC = createModel<WavBankMC, WavBankMCWidget>("wavbankmc");
