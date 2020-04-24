//
// Voxglitch "Grain Blender" module for VCV Rack
//

#include <stack>
#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"
#include "Common/submodules.hpp"

#include "GrainBlender/defines.h"
#include "GrainBlender/Grain.hpp"
#include "GrainBlender/GrainBlenderEx.hpp"
#include "GrainBlender/GrainBlender.hpp"
#include "GrainBlender/GrainBlenderLoadSample.hpp"
#include "GrainBlender/GrainBlenderWidget.hpp"

Model* modelGrainBlender = createModel<GrainBlender, GrainBlenderWidget>("grainblender");
