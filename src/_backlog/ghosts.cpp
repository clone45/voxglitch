//
// Voxglitch "Ghosts" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/sample.hpp"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"
#include "vgLib-1.0/dsp/Random.hpp"
#include "vgLib-1.0/dsp/StereoSmooth.hpp"

using namespace vgLib_v1;

#include "Ghosts/defines.h"
#include "Ghosts/GhostsEx.hpp"
#include "Ghosts/Ghosts.hpp"
#include "Ghosts/LoadSample.hpp"
#include "Ghosts/GhostsWidget.hpp"

Model* modelGhosts = createModel<Ghosts, GhostsWidget>("ghosts");
