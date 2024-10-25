//
// Voxglitch "FourTrack" module for VCV Rack
//

#include <fstream>

#include "plugin.hpp"
#include "osdialog.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/sample.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"

// For the lower waveform
#include "vgLib-2.0/widgets/WaveformModel.hpp"
#include "vgLib-2.0/widgets/WaveformWidget.hpp"

using namespace vgLib_v2;

#include "FourTrack/TrackModel.hpp"
#include "FourTrack/TrackWidget.hpp"

#include "FourTrack/FourTrack.hpp"
#include "FourTrack/FourTrackLoadSample.hpp"
#include "FourTrack/FourTrackWidget.hpp"

Model* modelFourTrack = createModel<FourTrack, FourTrackWidget>("fourtrack");
