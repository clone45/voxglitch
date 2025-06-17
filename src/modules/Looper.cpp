#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-2.0/constants.h"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/SamplePlayer.hpp"

using namespace vgLib_v2;

#include "Looper/defines.h"
#include "Looper/Looper.hpp"
#include "Looper/LooperLoadSample.hpp"
#include "Looper/LooperWaveformDisplay.hpp"
#include "Looper/LooperWidget.hpp"

Model* modelLooper = createModel<Looper, LooperWidget>("looper");
