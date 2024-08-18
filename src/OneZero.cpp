#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sequencer/Sequencer.hpp"
#include "vgLib-2.0/sequencer/GateSequencer.hpp"

using namespace vgLib_v2;

#include "OneZero/defines.h"
#include "OneZero/OneZero.hpp"
#include "OneZero/OneZeroReadoutWidget.hpp"
#include "OneZero/OneZeroWidget.hpp"

Model* modelOneZero = createModel<OneZero, OneZeroWidget>("onezero");
