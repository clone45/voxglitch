//
// Voxglitch "repeater" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/sample.hpp"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"
#include "vgLib-1.0/SamplePlayer.hpp"
#include "vgLib-1.0/dsp/DeclickFilter.hpp"

using namespace vgLib_v1;

#include "Repeater/defines.h"
#include "Repeater/Repeater.hpp"
#include "Repeater/Readout.hpp"
#include "Repeater/MenuItemLoadSample.hpp"
#include "Repeater/RepeaterWidget.hpp"

Model* modelRepeater = createModel<Repeater, RepeaterWidget>("repeater");
