//
// Voxglitch "VXDrumSequencer" module for VCV Rack
//
// The sequencer half of the vxsynth VX Drum Machine: a 16-step x 6-lane
// trigger grid with an accent lane, per-step ratchets and sixteen pattern
// memories, driven by an external clock. It drives the VXDrums kit over a
// polyphonic TRIG cable plus a separate ACC gate cable.
//

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "VXDrumSequencer/VXDrumSequencerPattern.hpp"
#include "VXDrumSequencer/VXDrumSequencer.hpp"
#include "VXDrumSequencer/VXDrumSequencerGridWidget.hpp"
#include "VXDrumSequencer/VXDrumSequencerMemoryButton.hpp"
#include "VXDrumSequencer/VXDrumSequencerWidget.hpp"

Model* modelVXDrumSequencer = createModel<VXDrumSequencer, VXDrumSequencerWidget>("VXDrumSequencer");
