
//
// Voxglitch "Grain Engine MK2" module for VCV Rack
//

// These two may be required only for Mac
#include <stack>
#include <vector>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/SamplePlayer.hpp"
#include "vgLib-2.0/dsp/StereoPan.hpp"
#include "vgLib-2.0/dsp/StereoFadeIn.hpp"
#include "vgLib-2.0/dsp/StereoFadeOut.hpp"
#include "vgLib-2.0/GrainEngineExpanderMessage.hpp"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "GrainEngineMK2/defines.h"
#include "GrainEngineMK2/Grain.hpp"
#include "GrainEngineMK2/GrainManager.hpp"
#include "GrainEngineMK2/GrainEngineMK2.hpp"
#include "GrainEngineMK2/GrainEngineMK2LoadSample.hpp"
#include "GrainEngineMK2/GrainEngineMK2PosDisplay.hpp"
#include "GrainEngineMK2/GrainEngineMK2Widget.hpp"

Model* modelGrainEngineMK2 = createModel<GrainEngineMK2, GrainEngineMK2Widget>("GrainEngineMK2");
