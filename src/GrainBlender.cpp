//
// Voxglitch "Grain Blender" module for VCV Rack
//

#include <stack>
#include <vector>
#include "plugin.hpp"
#include "osdialog.h"
#include "Common/audio_buffer.hpp"
#include "Common/submodules.hpp"

#include "GrainBlender/defines.h"
#include "GrainBlender/Grain.hpp"
#include "GrainBlender/GrainBlenderEx.hpp"
#include "GrainBlender/GrainBlender.hpp"
#include "GrainBlender/GrainBlenderWidget.hpp"

Model* modelGrainBlender = createModel<GrainBlender, GrainBlenderWidget>("grainblender");
