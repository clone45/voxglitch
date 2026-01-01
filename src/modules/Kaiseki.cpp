#include "../plugin.hpp"
#include "osdialog.h"

// Include vgLib components if needed
#include "../vgLib-2.0/components/VoxglitchComponents.hpp"
#include "../vgLib-2.0/sample.hpp"

using namespace vgLib_v2;

// Include module headers
#include "Kaiseki/Kaiseki.hpp"
#include "Kaiseki/KaisekiWidget.hpp"

// Create the model
Model* modelKaiseki = createModel<Kaiseki, KaisekiWidget>("kaiseki");