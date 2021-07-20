
//
// Voxglitch "Grain Engine MK2" module for VCV Rack
//

#include <stack>
#include <vector>
#include "plugin.hpp"
#include "osdialog.h"
#include "Common/common.hpp"
#include "Common/sample.hpp"
#include "Common/submodules.hpp"
#include "Common/GrainEngineExpanderMessage.hpp"

#include "GrainEngineMK2/defines.h"
#include "GrainEngineMK2/Grain.hpp"
#include "GrainEngineMK2/GrainEngineMK2Core.hpp"
#include "GrainEngineMK2/GrainEngineMK2.hpp"
#include "GrainEngineMK2/GrainEngineMK2LoadSample.hpp"
#include "GrainEngineMK2/GrainEngineMK2PosDisplay.hpp"
#include "GrainEngineMK2/GrainEngineMK2Widget.hpp"

Model* modelGrainEngineMK2 = createModel<GrainEngineMK2, GrainEngineMK2Widget>("GrainEngineMK2");
