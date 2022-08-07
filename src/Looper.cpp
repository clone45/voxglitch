#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/sample.hpp"
#include "Common/VoxglitchSamplerModule.hpp"
#include "Common/VoxglitchSamplerModuleWidget.hpp"
#include "Common/SamplePlayer.hpp"

#include "Looper/defines.h"
// #include "Looper/SamplePlayer.hpp"
#include "Looper/Looper.hpp"
#include "Looper/LooperLoadSample.hpp"
#include "Looper/LooperWaveformDisplay.hpp"
#include "Looper/LooperWidget.hpp"

Model* modelLooper = createModel<Looper, LooperWidget>("looper");
