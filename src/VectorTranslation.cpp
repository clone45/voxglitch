#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

#include "vgLib-2.0/constants.h"

#include "vgLib-2.0/components/VoxglitchComponents.hpp"

using namespace vgLib_v2;

#include "VectorTranslation/VectorTranslation.hpp"
#include "VectorTranslation/VectorTranslationWidget.hpp"

Model* modelVectorTranslation = createModel<VectorTranslation, VectorTranslationWidget>("vector_translation");
