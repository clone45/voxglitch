//
// Voxglitch "Timeline" module for VCV Rack
//
// A 16-lane automation timeline: draw breakpoint curves against a musical
// playhead and send them out as one polyphonic cable. Every lane owns its own
// transport, so the polyphonic START/STOP/RESET inputs drive each lane
// independently.
//
// Ported from the voxglitch_devices collection (branch dev/timeline), itself a
// port of the vxsynth studio's automation system.
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "Timeline/TimelineEngine.hpp"
#include "Timeline/Timeline.hpp"
#include "Timeline/TimelineEditor.hpp"
#include "Timeline/TimelineWidget.hpp"

Model* modelTimeline = createModel<Timeline, TimelineWidget>("timeline");
