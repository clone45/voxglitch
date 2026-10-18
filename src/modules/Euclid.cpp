//
// Voxglitch "Euclid" module for VCV Rack
//
// A Euclidean rhythm sequencer: HITS hits spread as evenly as Bresenham
// allows over STEPS steps, shifted by ROT, stepped by an external clock, with
// a trigger output and a ring display that shows the live pattern
// and rotates it when dragged. The Rack port of the vxsynth Euclidean
// Sequencer (euclid.c); docs/modules/euclid/rack-port-design.md is the record.
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "plugin.hpp"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "Euclid/EuclidEngine.hpp"
#include "Euclid/Euclid.hpp"
#include "Euclid/EuclidRingWidget.hpp"
#include "Euclid/EuclidWidget.hpp"

Model* modelEuclid = createModel<Euclid, EuclidWidget>("euclid");
