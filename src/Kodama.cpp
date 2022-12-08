#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/sequencer/Sequencer.hpp"
#include "Common/sequencer/GateSequencer.hpp"

#include "Kodama/defines.h"
#include "Kodama/Kodama.hpp"
#include "Kodama/KodamaGridWidget.hpp"
#include "Kodama/KodamaWidget.hpp"

Model* modelKodama = createModel<Kodama, KodamaWidget>("kodama");
