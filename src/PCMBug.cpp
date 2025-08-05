#include "plugin.hpp"
#include "osdialog.h"

// Include vgLib components if needed
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

// Include module headers
#include "PCMBug/PCMBug.hpp"
#include "PCMBug/PCMBugWidget.hpp"

// Create the model
Model* modelPCMBug = createModel<PCMBug, PCMBugWidget>("pcm_bug");