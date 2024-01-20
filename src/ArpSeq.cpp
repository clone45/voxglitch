//
// Voxglitch X Omri Cohen 
// ArpSeq module for VCV Rack
//

#include "plugin.hpp"
#include "osdialog.h"
#include "settings.hpp"

// These two includes are required for Mac
#include <fstream>
#include <array>

#include "ArpSeq/defines.h"

#include "vgLib-2.0/constants.h"
#include "vgLib-2.0/Theme.hpp"
#include "vgLib-2.0/components/VoxglitchComponents.hpp"
#include "vgLib-2.0/dsp/Quantizer.hpp"
#include "vgLib-2.0/dsp/SlewLimiter.hpp"
#include "vgLib-2.0/dsp/SampleAndHold.hpp"
#include "vgLib-2.0/dsp/ClockModifier.hpp"
#include "vgLib-2.0/dsp/ClockDivider.hpp"
#include "vgLib-2.0/sequencer/VoltageSequencerHistory.hpp"
#include "vgLib-2.0/sequencer/SequencerDisplayConfig.hpp"
#include "vgLib-2.0/sequencer/Sequencer.hpp"
#include "vgLib-2.0/sequencer/IndexSequencer.hpp"
#include "vgLib-2.0/sequencer/ArpSequencer.hpp"
#include "vgLib-2.0/sequencer/VoltageSequencer.hpp"
#include "vgLib-2.0/helpers/JSON.hpp"
#include "vgLib-2.0/helpers/SEQUENCERS.hpp"
#include "vgLib-2.0/customParamQuantities.hpp"

using namespace vgLib_v2;

#include "ArpSeq/ArpSeq.hpp"
#include "ArpSeq/SegmentReadoutWidget.hpp"
#include "ArpSeq/ArpSeqWindow.hpp"
#include "ArpSeq/ArpSequencerDisplay.hpp"
#include "ArpSeq/ArpVoltageSequencerDisplay.hpp"
#include "ArpSeq/TabsWidget.hpp"
#include "ArpSeq/SlewSlider.hpp"
#include "ArpSeq/ArpSeqWidget.hpp"

Model* modelArpSeq = createModel<ArpSeq, ArpSeqWidget>("arpseq");
