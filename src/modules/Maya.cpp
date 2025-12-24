//
// Voxglitch "Maya" module for VCV Rack
// A talking word player using AI-generated voice samples
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/dsp/DeclickFilter.hpp"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/SamplePlayer.hpp"

using namespace vgLib_v2;

#include "Maya/defines.h"
#include "Maya/Maya.hpp"
#include "Maya/MayaWordDisplay.hpp"
#include "Maya/MenuItemLoadWords.hpp"
#include "Maya/MayaWidget.hpp"

Model* modelMaya = createModel<Maya, MayaWidget>("maya");
