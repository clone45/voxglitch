#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/constants.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "Inner/defines.h"
#include "Inner/Sport.hpp"
#include "Inner/Patch.hpp"
#include "Inner/PatchLoader.hpp"
#include "Inner/ModuleConfig.hpp"
#include "Inner/PatchConstructor.hpp"
#include "Inner/PatchRunner.hpp"
#include "Inner/Inner.hpp"
#include "Inner/InnerWidget.hpp"

Model* modelInner = createModel<Inner, InnerWidget>("inner");