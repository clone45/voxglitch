
//
// Voxglitch "Grain Engine MK2" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/common.hpp"
#include "Common/sample.hpp"
#include "Common/GrainEngineExpanderMessage.hpp"

#include "GrainEngineMK2Expander/defines.h"
#include "GrainEngineMK2Expander/GrainEngineMK2Expander.hpp"
#include "GrainEngineMK2Expander/GrainEngineMK2ExpanderWidget.hpp"

Model* modelGrainEngineMK2Expander = createModel<GrainEngineMK2Expander, GrainEngineMK2ExpanderWidget>("GrainEngineMK2Expander");
