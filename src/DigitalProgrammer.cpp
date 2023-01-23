
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes are required for Mac
#include <fstream>
#include <array>

#include "DigitalProgrammer/defines.h"
#include "Common/constants.h"

using namespace digital_programmer;

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "DigitalProgrammer/DPSlider.hpp"
#include "DigitalProgrammer/DigitalProgrammer.hpp"
#include "DigitalProgrammer/CopyPasteLabel.hpp"
#include "DigitalProgrammer/DPSliderDisplay.hpp"
#include "DigitalProgrammer/DPBankButtonDisplay.hpp"
#include "DigitalProgrammer/DigitalProgrammerWidget.hpp"

Model* modelDigitalProgrammer = createModel<DigitalProgrammer, DigitalProgrammerWidget>("digitalprogrammer");
