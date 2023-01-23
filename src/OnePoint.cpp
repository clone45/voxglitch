#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/constants.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/sequencer/Sequencer.hpp"
#include "Common/sequencer/GateSequencer.hpp"

#include "OnePoint/OnePoint.hpp"
#include "OnePoint/OnePointReadoutWidget.hpp"
#include "OnePoint/OnePointWidget.hpp"

Model* modelOnePoint = createModel<OnePoint, OnePointWidget>("onepoint");
