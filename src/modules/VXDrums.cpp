//
// Voxglitch "VXDrums" module for VCV Rack
//
// The kit half of the VX Drum Machine port: six analogue-modelled drum voices
// (BD, SD, CP, RS, CH, OH) with a shared accent gate, a stereo mix behind a
// one-knob drive, and six dry individual outs. Triggered over one polyphonic
// cable; pairs with VX Drum Sequencer, or with anything that emits gates.
//
// Ported from vxsynth's vxdrumvoices.c, whose voices are the Machine's
// (vxdrums.c) verbatim.
//

#include <cmath>
#include <cstdint>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "VXDrums/VXDrumVoices.hpp"
#include "VXDrums/VXDrums.hpp"
#include "VXDrums/VXDrumsWidget.hpp"

Model* modelVXDrums = createModel<VXDrums, VXDrumsWidget>("VXDrums");
