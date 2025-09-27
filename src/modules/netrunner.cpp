#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/dsp/DeclickFilter.hpp"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/SamplePlayer.hpp"

using namespace vgLib_v2;

#include "Netrunner/defines.h"
#include "Netrunner/AsyncSampleLoader.hpp"
#include "Netrunner/Netrunner.hpp"
#include "Netrunner/NetrunnerReadout.hpp"
#include "Netrunner/MenuItemLoadConfig.hpp"

#include "Netrunner/NetrunnerWidget.hpp"

Model* modelNetrunner = createModel<Netrunner, NetrunnerWidget>("netrunner");