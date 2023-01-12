#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/sequencer/Sequencer.hpp"
#include "Common/sequencer/GateSequencer.hpp"

#include "Common/wren.c"
#include "Common/wren.h"

#include "OneZero/defines.h"
#include "OneZero/OneZero.hpp"
#include "OneZero/OneZeroReadoutWidget.hpp"
#include "OneZero/OneZeroWidget.hpp"

Model* modelOneZero = createModel<OneZero, OneZeroWidget>("onezero");
