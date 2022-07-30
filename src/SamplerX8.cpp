//
// Voxglitch "SamplerX8" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"
#include "Common/VoxglitchSamplerModule.hpp"
#include "Common/VoxglitchSamplerModuleWidget.hpp"
#include "Common/submodules.hpp"
#include "Common/SamplePlayer.hpp"

#include "SamplerX8/defines.h"
#include "SamplerX8/SamplerX8.hpp"
#include "SamplerX8/SamplerX8LoadSample.hpp"
#include "SamplerX8/SamplerX8LoadFolder.hpp"
#include "SamplerX8/SamplerX8Widget.hpp"

Model* modelSamplerX8 = createModel<SamplerX8, SamplerX8Widget>("samplerx8");
