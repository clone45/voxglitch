
//
// Voxglitch "Grain Engine MK2" module for VCV Rack
//

#include <stack>
#include <vector>
#include "plugin.hpp"
#include "osdialog.h"
#include "Common/common.hpp"
#include "Common/sample.hpp"
#include "Common/SamplePlayer.hpp"
#include "Common/dsp/StereoPan.hpp"
#include "Common/dsp/StereoFadeIn.hpp"
#include "Common/dsp/StereoFadeOut.hpp"
#include "Common/VoxglitchSamplerModule.hpp"
#include "Common/VoxglitchSamplerModuleWidget.hpp"
#include "Common/GrainEngineExpanderMessage.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "GrainEngineMK2/defines.h"
#include "GrainEngineMK2/Grain.hpp"
#include "GrainEngineMK2/GrainManager.hpp"
#include "GrainEngineMK2/GrainEngineMK2.hpp"
#include "GrainEngineMK2/GrainEngineMK2LoadSample.hpp"
#include "GrainEngineMK2/GrainEngineMK2PosDisplay.hpp"
#include "GrainEngineMK2/GrainEngineMK2Widget.hpp"

Model* modelGrainEngineMK2 = createModel<GrainEngineMK2, GrainEngineMK2Widget>("GrainEngineMK2");
