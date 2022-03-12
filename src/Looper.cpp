#include "plugin.hpp"
#include "osdialog.h"
#include "Common/sample.hpp"
#include "Common/components/BlankPort.hpp"
#include "Common/components/InputPort.hpp"
#include "Common/components/OutputPort.hpp"
#include "Common/components/VoxglitchPanel.hpp"

#include "settings.hpp"
#include "Looper/defines.h"
#include "Looper/SamplePlayer.hpp"
#include "Looper/Looper.hpp"
#include "Looper/LooperLoadSample.hpp"
#include "Looper/LooperWaveformDisplay.hpp"
#include "Looper/LooperWidget.hpp"

Model* modelLooper = createModel<Looper, LooperWidget>("looper");
