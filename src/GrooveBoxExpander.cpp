
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>

#include "GrooveBoxExpander/defines.h"

#include "Common/VoxglitchWidget.hpp"
#include "GrooveBoxExpander/ExpanderToGrooveboxMessage.hpp"
#include "GrooveBox/GrooveboxToExpanderMessage.hpp"
#include "GrooveBoxExpander/GrooveBoxExpander.hpp"
#include "GrooveBoxExpander/GrooveBoxExpanderWidget.hpp"

Model* modelGrooveBoxExpander = createModel<GrooveBoxExpander, GrooveBoxExpanderWidget>("GrooveBoxExpander");
