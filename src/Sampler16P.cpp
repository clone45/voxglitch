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

#include "Sampler16P/defines.h"
#include "Sampler16P/Sampler16P.hpp"
#include "Sampler16P/Sampler16PLoadSample.hpp"
#include "Sampler16P/Sampler16PLoadFolder.hpp"
#include "Sampler16P/Sampler16PWidget.hpp"

Model* modelSampler16P = createModel<Sampler16P, Sampler16PWidget>("sampler16p");
