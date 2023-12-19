
//
// Voxglitch "Grain Engine MK2" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/sample.hpp"
#include "vgLib-1.0/GrainEngineExpanderMessage.hpp"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v1;

#include "GrainEngineMK2Expander/defines.h"
#include "GrainEngineMK2Expander/GrainEngineMK2Expander.hpp"
#include "GrainEngineMK2Expander/GrainEngineMK2ExpanderWidget.hpp"

Model* modelGrainEngineMK2Expander = createModel<GrainEngineMK2Expander, GrainEngineMK2ExpanderWidget>("GrainEngineMK2Expander");
