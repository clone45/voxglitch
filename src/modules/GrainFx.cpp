//
// Voxglitch "Grain Fx" module for VCV Rack
//
#include <stack>
#include <vector>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/common.hpp"
#include "vgLib-2.0/audio_buffer.hpp"
#include "vgLib-2.0/dsp/StereoPan.hpp"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "GrainFx/defines.h"
#include "GrainFx/SimpleTableOsc.hpp"
#include "GrainFx/Grain.hpp"
#include "GrainFx/GrainFxCore.hpp"
#include "GrainFx/GrainFx.hpp"
#include "GrainFx/GrainFxWidget.hpp"

Model* modelGrainFx = createModel<GrainFx, GrainFxWidget>("grainfx");
