
#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"
#include <fstream>
#include <array>

#include "DigitalProgrammer/defines.h"
#include "Common/VoxglitchWidget.hpp"
#include "DigitalProgrammer/DigitalProgrammer.hpp"
#include "DigitalProgrammer/DigitalProgrammerWidget.hpp"

Model* modelDigitalProgrammer = createModel<DigitalProgrammer, DigitalProgrammerWidget>("digitalprogrammer");
