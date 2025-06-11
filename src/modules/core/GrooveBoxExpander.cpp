
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes seem only necessary for Mac:
#include <fstream>
#include <array>

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "GrooveBoxExpander/defines.h"
#include "GrooveBoxExpander/ExpanderToGrooveboxMessage.hpp"
#include "GrooveBox/GrooveboxToExpanderMessage.hpp"
#include "GrooveBoxExpander/GrooveBoxExpander.hpp"
#include "GrooveBoxExpander/GrooveBoxExpanderWidget.hpp"

Model* modelGrooveBoxExpander = createModel<GrooveBoxExpander, GrooveBoxExpanderWidget>("GrooveBoxExpander");
