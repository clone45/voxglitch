//
// Voxglitch "CueResearchExpander" module for VCV Rack
//

#include <fstream>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/helpers/JSON.hpp"

using namespace vgLib_v2;

#include "CueResearchExpander/CueResearchExpander.hpp"
#include "CueResearchExpander/CueResearchExpanderWidget.hpp"

Model* modelCueResearchExpander = createModel<CueResearchExpander, CueResearchExpanderWidget>("cue_research_expander");
