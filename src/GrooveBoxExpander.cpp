
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes seem only necessary for Mac:
#include <fstream>
#include <array>

#include "GrooveBoxExpander/defines.h"

#include "Common/constants.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "GrooveBoxExpander/ExpanderToGrooveboxMessage.hpp"
#include "GrooveBox/GrooveboxToExpanderMessage.hpp"
#include "GrooveBoxExpander/GrooveBoxExpander.hpp"
#include "GrooveBoxExpander/GrooveBoxExpanderWidget.hpp"

Model* modelGrooveBoxExpander = createModel<GrooveBoxExpander, GrooveBoxExpanderWidget>("GrooveBoxExpander");
