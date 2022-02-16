
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>

#include "Scalar110/defines.h"
using namespace scalar_110;

#include "Common/VoxglitchWidget.hpp"
#include "Scalar110/StepParams.hpp"
#include "Scalar110/Engine.hpp"
#include "Scalar110/engines/Foo.hpp"
#include "Scalar110/Track.hpp"
#include "Scalar110/Scalar110.hpp"
#include "Scalar110/Scalar110Widget.hpp"

Model* modelScalar110 = createModel<Scalar110, Scalar110Widget>("scalar110");
