//
// Voxglitch "Note Detector" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/widgets/NoteReadoutWidget.hpp"
#include "vgLib-2.0/helpers/JSON.hpp"
#include "vgLib-2.0/helpers/NOTES.hpp"
#include "vgLib-2.0/customParamQuantities.hpp"

using namespace vgLib_v2;

#include "NoteDetector/NoteDetector.hpp"
#include "NoteDetector/NoteDetectorWidget.hpp"

Model* modelNoteDetector = createModel<NoteDetector, NoteDetectorWidget>("noteDetector");
