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

#include "Common/constants.h"
#include "ArpSeq/defines.h"

#include "Common/Theme.hpp"
#include "Common/components/VoxglitchComponents.hpp"


#include "Common/dsp/Quantizer.hpp"
#include "Common/dsp/SlewLimiter.hpp"
#include "Common/dsp/SampleAndHold.hpp"
#include "Common/dsp/ClockModifier.hpp"
#include "Common/dsp/ClockDivider.hpp"
#include "Common/sequencer/VoltageSequencerHistory.hpp"
#include "Common/sequencer/SequencerDisplayConfig.hpp"
#include "Common/sequencer/Sequencer.hpp"
#include "Common/sequencer/IndexSequencer.hpp"
#include "Common/sequencer/ArpSequencer.hpp"
#include "Common/sequencer/VoltageSequencer.hpp"
#include "Common/helpers/JSON.hpp"
#include "Common/helpers/NOTES.hpp"
#include "Common/customParamQuantities.hpp"

#include "ArpSeq/ArpSeq.hpp"
#include "ArpSeq/SegmentReadoutWidget.hpp"
#include "ArpSeq/ArpSeqWindow.hpp"
#include "ArpSeq/ArpSequencerDisplay.hpp"
#include "ArpSeq/ArpVoltageSequencerDisplay.hpp"
#include "ArpSeq/TabsWidget.hpp"
#include "ArpSeq/SlewSlider.hpp"
#include "ArpSeq/ArpSeqWidget.hpp"

Model* modelArpSeq = createModel<ArpSeq, ArpSeqWidget>("arpseq");
