//
// Voxglitch "Autobreak" module for VCV Rack
//
// This code is heavily based on Clément Foulc's PLAY module
// which can be found here:  https://github.com/cfoulc/cf/blob/v1/src/PLAY.cpp

#include <fstream>

#include "plugin.hpp"
#include "osdialog.h"

/*
#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/sample.hpp"
#include "vgLib-1.0/dsp/DeclickFilter.hpp"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v1;
*/

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/dsp/DeclickFilter.hpp"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sample/SampleLoaderMenuItem.hpp"

using namespace vgLib_v2;

#include "Autobreak/defines.h"
#include "Autobreak/Autobreak.hpp"
#include "Autobreak/AutobreakWidget.hpp"

Model* modelAutobreak = createModel<Autobreak, AutobreakWidget>("autobreak");
