//
// Voxglitch "Note Detector" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "Common/constants.h"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"
#include "Common/widgets/NoteReadoutWidget.hpp"

#include "Common/helpers/JSON.hpp"
#include "Common/helpers/NOTES.hpp"
#include "Common/customParamQuantities.hpp"

#include "NoteDetector/NoteDetector.hpp"
#include "NoteDetector/NoteDetectorWidget.hpp"

Model* modelNoteDetector = createModel<NoteDetector, NoteDetectorWidget>("noteDetector");
