#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-1.0/constants.h"
#include "vgLib-1.0/Theme.hpp"
#include "vgLib-1.0/components/VoxglitchComponents.hpp"
#include "vgLib-1.0/sample.hpp"
#include "vgLib-1.0/SamplePlayer.hpp"

using namespace vgLib_v1;

#include "Looper/defines.h"
#include "Looper/Looper.hpp"
#include "Looper/LooperLoadSample.hpp"
#include "Looper/LooperWaveformDisplay.hpp"
#include "Looper/LooperWidget.hpp"

Model* modelLooper = createModel<Looper, LooperWidget>("looper");
