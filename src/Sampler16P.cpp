//
// Voxglitch "SamplerX8" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "Common/sample.hpp"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/SamplePlayer.hpp"
#include "Common/dsp/StereoPan.hpp"

#include "Sampler16p/defines.h"
#include "Sampler16p/Sampler16P.hpp"
#include "Sampler16p/Sampler16PLoadSample.hpp"
#include "Sampler16p/Sampler16PLoadFolder.hpp"
#include "Sampler16p/Sampler16PWidget.hpp"

Model* modelSampler16P = createModel<Sampler16P, Sampler16PWidget>("sampler16p");
