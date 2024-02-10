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

#include "Sampler16P/defines.h"
#include "Sampler16P/Sampler16P.hpp"
#include "Sampler16P/Sampler16PLoadSample.hpp"
#include "Sampler16P/Sampler16PLoadFolder.hpp"
#include "Sampler16P/Sampler16PWidget.hpp"

Model* modelSampler16P = createModel<Sampler16P, Sampler16PWidget>("sampler16p");
