//
// Voxglitch "CueResearch" module for VCV Rack
//

#include <fstream>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/helpers/JSON.hpp"

// For the lower waveform
#include "vgLib-2.0/widgets/WaveformModel.hpp"
#include "vgLib-2.0/widgets/WaveformWidget.hpp"
#include "vgLib-2.0/helpers/JSON.hpp"

using namespace vgLib_v2;

#include "CueResearch/TrackModel.hpp"
#include "CueResearch/TrackWidget.hpp"

#include "CueResearch/CueResearch.hpp"
#include "CueResearch/CueResearchLoadSample.hpp"
#include "CueResearch/CueResearchWidget.hpp"

Model* modelCueResearch = createModel<CueResearch, CueResearchWidget>("cue_research");
