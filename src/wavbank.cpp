//
// Voxglitch "wav bank" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/sample.hpp"
#include "vgLib-1.0/dsp/DeclickFilter.hpp"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"
#include "vgLib-1.0/SamplePlayer.hpp"

using namespace vgLib_v1;

#include "WavBank/defines.h"
#include "WavBank/WavBank.hpp"
#include "WavBank/WavBankReadout.hpp"
#include "WavBank/MenuItemLoadBank.hpp"

#include "WavBank/WavBankWidget.hpp"

Model* modelWavBank = createModel<WavBank, WavBankWidget>("wavbank");
