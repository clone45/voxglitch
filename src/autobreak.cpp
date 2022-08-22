//
// Voxglitch "Autobreak" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"
#include "Common/dsp/DeclickFilter.hpp"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "Autobreak/defines.h"
#include "Autobreak/Autobreak.hpp"
#include "Autobreak/AutobreakLoadSample.hpp"
#include "Autobreak/AutobreakWidget.hpp"

Model* modelAutobreak = createModel<Autobreak, AutobreakWidget>("autobreak");
