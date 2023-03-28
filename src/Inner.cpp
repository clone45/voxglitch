#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "Common/constants.h"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "Inner/defines.h"
#include "Inner/Sport.hpp"
#include "Inner/submodules/VCOModule.hpp"
#include "Inner/ModuleConfig.hpp"
#include "Inner/ModuleManager.hpp"
#include "Inner/Inner.hpp"
#include "Inner/InnerWidget.hpp"

Model* modelInner = createModel<Inner, InnerWidget>("inner");