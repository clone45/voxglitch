//
// Voxglitch "Grain Engine" module for VCV Rack
//

#include <stack>
#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"
#include "Common/submodules.hpp"

#include "GrainEngine/defines.h"
#include "GrainEngine/GrainAmpSlopes.hpp"
#include "GrainEngine/Grain.hpp"
#include "GrainEngine/GrainEngineEx.hpp"
#include "GrainEngine/GrainEngine.hpp"
#include "GrainEngine/GrainEngineLoadSample.hpp"
#include "GrainEngine/GrainEngineWidget.hpp"

Model* modelGrainEngine = createModel<GrainEngine, GrainEngineWidget>("grainengine");
