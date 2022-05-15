#define DR_WAV_IMPLEMENTATION
#include "Common/dr_wav.h"
// #include "Common/tables.hpp"
#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
  pluginInstance = p;

  p->addModel(modelAutobreak);
  p->addModel(modelByteBeat);
  p->addModel(modelDigitalProgrammer);
  p->addModel(modelDigitalSequencer);
  p->addModel(modelDigitalSequencerXP);
  p->addModel(modelGlitchSequencer);
  p->addModel(modelGhosts);
  p->addModel(modelGoblins);
  p->addModel(modelGrainEngine);
  p->addModel(modelGrainEngineMK2);
  p->addModel(modelGrainEngineMK2Expander);
  p->addModel(modelGrainFx);
  p->addModel(modelGrooveBox);
  p->addModel(modelGrooveBoxExpander);
  p->addModel(modelHazumi);
  p->addModel(modelLooper);
  p->addModel(modelRepeater);
  p->addModel(modelSamplerX8);
  p->addModel(modelSatanonaut);
  p->addModel(modelWavBank);
  p->addModel(modelWavBankMC);
  p->addModel(modelXY);
}
