//
// Voxglitch "Galacto" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

// #include "ByteBeat/calculator.hpp"
#include "Galacto/defines.h"
#include "Galacto/GalactoAudioBuffer.hpp"
#include "Galacto/GalactoSamples.hpp"
#include "Galacto/Galacto.hpp"
#include "Galacto/GalactoWidget.hpp"

Model* modelGalacto = createModel<Galacto, GalactoWidget>("galacto");
