//
// Voxglitch "Ghosts" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"

#include "Ghosts/defines.h"
#include "Ghosts/GhostsEx.hpp"
#include "Ghosts/Ghosts.hpp"
#include "Ghosts/LoadSample.hpp"
#include "Ghosts/GhostsWidget.hpp"

Model* modelGhosts = createModel<Ghosts, GhostsWidget>("ghosts");
