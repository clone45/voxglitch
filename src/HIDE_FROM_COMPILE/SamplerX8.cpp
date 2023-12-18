//
// Voxglitch "SamplerX8" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "Common/constants.h"
#include "Common/sample.hpp"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/SamplePlayer.hpp"
#include "Common/dsp/StereoPan.hpp"

#include "SamplerX8/defines.h"
#include "SamplerX8/SamplerX8.hpp"
#include "SamplerX8/SamplerX8LoadSample.hpp"
#include "SamplerX8/SamplerX8LoadFolder.hpp"
#include "SamplerX8/SamplerX8Widget.hpp"

Model* modelSamplerX8 = createModel<SamplerX8, SamplerX8Widget>("samplerx8");
