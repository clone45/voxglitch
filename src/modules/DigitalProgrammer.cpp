
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes are required for Mac
#include <fstream>
#include <array>

#include "DigitalProgrammer/defines.h"
#include "vgLib-2.0/constants.h"

using namespace digital_programmer;


#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "DigitalProgrammer/DPSlider.hpp"
#include "DigitalProgrammer/DigitalProgrammer.hpp"
#include "DigitalProgrammer/CopyPasteLabel.hpp"
#include "DigitalProgrammer/DPSliderDisplay.hpp"
#include "DigitalProgrammer/DPBankButtonDisplay.hpp"
#include "DigitalProgrammer/DigitalProgrammerWidget.hpp"

Model* modelDigitalProgrammer = createModel<DigitalProgrammer, DigitalProgrammerWidget>("digitalprogrammer");
