//
// Voxglitch "Goblins" module for VCV Rack
//


#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"

#include "Goblins/defines.h"
#include "Goblins/Goblin.hpp"
#include "Goblins/Goblins.hpp"
#include "Goblins/GoblinsSampleReadout.hpp"
#include "Goblins/GoblinsLoadSample.hpp"
#include "Goblins/GoblinsWidget.hpp"

Model* modelGoblins = createModel<Goblins, GoblinsWidget>("goblins");
