//
// Voxglitch "MellowRabbit" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/constants.h"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "MellowRabbit/MellowRabbit.hpp"
#include "MellowRabbit/MellowRabbitWidget.hpp"

Model* modelMellowRabbit = createModel<MellowRabbit, MellowRabbitWidget>("mellowRabbit");