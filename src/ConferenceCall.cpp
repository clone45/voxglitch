//
// Voxglitch "Conference Call" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/dsp/AdaptiveGainControl.hpp"

using namespace vgLib_v2;

#include "ConferenceCall/ConferenceCall.hpp"
#include "ConferenceCall/ConferenceCallWidget.hpp"
Model* modelConferenceCall = createModel<ConferenceCall, ConferenceCallWidget>("conferencecall");
