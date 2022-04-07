
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>

#include "Scalar110/defines.h"
using namespace scalar_110;

#include "Common/VoxglitchWidget.hpp"
#include "Scalar110/SamplePlaybackSettings.hpp"

// Sample players and such
#include "Common/sample.hpp"
#include "Common/common.hpp"
#include "Common/submodules.hpp"

// Core components
#include "Scalar110/SamplePlayer.hpp"
#include "Scalar110/Track.hpp"
#include "Scalar110/Pattern.hpp"
#include "Scalar110/Scalar110.hpp"


// #include "Scalar110/FileSelectWidget.hpp"
// #include "Scalar110/ParamEditorDisplay.hpp"
// #include "Scalar110/LCDDisplay/LCDWidget.hpp"
#include "Scalar110/Scalar110Widget.hpp"

Model* modelScalar110 = createModel<Scalar110, Scalar110Widget>("scalar110");
