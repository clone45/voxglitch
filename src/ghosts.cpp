//
// Voxglitch "Ghosts" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "Common/sample.hpp"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/dsp/StereoSmooth.hpp"

#include "Ghosts/defines.h"
#include "Ghosts/GhostsEx.hpp"
#include "Ghosts/Ghosts.hpp"
#include "Ghosts/LoadSample.hpp"
#include "Ghosts/GhostsWidget.hpp"

Model* modelGhosts = createModel<Ghosts, GhostsWidget>("ghosts");
