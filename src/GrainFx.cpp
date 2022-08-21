//
// Voxglitch "Grain Fx" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/common.hpp"
#include "Common/audio_buffer.hpp"
#include "Common/dsp/StereoPan.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "GrainFx/defines.h"
#include "GrainFx/SimpleTableOsc.hpp"
#include "GrainFx/Grain.hpp"
#include "GrainFx/GrainFxCore.hpp"
#include "GrainFx/GrainFx.hpp"
#include "GrainFx/GrainFxWidget.hpp"

Model* modelGrainFx = createModel<GrainFx, GrainFxWidget>("grainfx");
