//
// Voxglitch "SamplerX8" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/SamplePlayer.hpp"
#include "vgLib-2.0/dsp/StereoPan.hpp"

using namespace vgLib_v2;

#include "SamplerX8/defines.h"
#include "SamplerX8/SamplerX8.hpp"
#include "SamplerX8/SamplerX8LoadSample.hpp"
#include "SamplerX8/SamplerX8LoadFolder.hpp"
#include "SamplerX8/SamplerX8Widget.hpp"

Model* modelSamplerX8 = createModel<SamplerX8, SamplerX8Widget>("samplerx8");
