#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-2.0/constants.h"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/SamplePlayer.hpp"

using namespace vgLib_v2;

#include "vgLib-2.0/widgets/Marker.hpp"
#include "vgLib-2.0/widgets/TrackModel.hpp"
#include "vgLib-2.0/widgets/TrackWidget.hpp"

#include "Looper/defines.h"
#include "Looper/BeatPipeline.hpp"
#include "Looper/BeatAnalyzer.hpp"
#include "Looper/Looper.hpp"
#include "Looper/LooperLoadSample.hpp"
#include "Looper/LooperWaveformGrid.hpp"
#include "Looper/LooperWaveformStrip.hpp"
#include "Looper/LooperWidget.hpp"

Model* modelLooper = createModel<Looper, LooperWidget>("looper");
