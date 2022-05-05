
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>

#include "DigitalProgrammer/defines.h"
using namespace digital_programmer;

#include "Common/VoxglitchWidget.hpp"
#include "DigitalProgrammer/DPSlider.hpp"
#include "DigitalProgrammer/DigitalProgrammer.hpp"
#include "DigitalProgrammer/CopyPasteLabel.hpp"
#include "DigitalProgrammer/DPSliderDisplay.hpp"
#include "DigitalProgrammer/DPBankButtonDisplay.hpp"
#include "DigitalProgrammer/DigitalProgrammerWidget.hpp"

Model* modelDigitalProgrammer = createModel<DigitalProgrammer, DigitalProgrammerWidget>("digitalprogrammer");
