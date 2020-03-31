//
// Voxglitch "wav bank" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"

#include "WavBank/defines.h"
#include "WavBank/WavBank.hpp"
#include "WavBank/WavBankReadout.hpp"
#include "WavBank/MenuItemLoadBank.hpp"
#include "WavBank/WavBankWidget.hpp"

Model* modelWavBank = createModel<WavBank, WavBankWidget>("wavbank");
