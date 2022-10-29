
//
// Voxglitch "Audio ###" module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include <curl/curl.h>

#include "Common/sample.hpp"
#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"

#include "AudioPoop/defines.h"
#include "AudioPoop/AudioPoop.hpp"
#include "AudioPoop/AudioPoopWidget.hpp"

Model* modelAudioPoop = createModel<AudioPoop, AudioPoopWidget>("AudioPoop");