#include "plugin.hpp"
#include "Common/constants.cpp"

Plugin* pluginInstance;

void init(Plugin* p) {
  pluginInstance = p;
  p->addModel(modelAutobreak);
  p->addModel(modelAutobreakStudio);  
  p->addModel(modelByteBeat);
  p->addModel(modelDigitalProgrammer);
  p->addModel(modelDigitalSequencer);
  p->addModel(modelDigitalSequencerXP);
  p->addModel(modelDrumRandomizer);  
  p->addModel(modelGlitchSequencer);
  p->addModel(modelGhosts);
  p->addModel(modelGrainEngineMK2);
  p->addModel(modelGrainEngineMK2Expander);
  p->addModel(modelGrainFx);
  p->addModel(modelGrooveBox);
  p->addModel(modelGrooveBoxExpander);
  p->addModel(modelHazumi);
  p->addModel(modelOnePoint);
  p->addModel(modelOneZero);
  p->addModel(modelLooper);
  p->addModel(modelRepeater);
  p->addModel(modelSamplerX8);
  p->addModel(modelSampler16P);
  p->addModel(modelSatanonaut);
  p->addModel(modelWavBank);
  p->addModel(modelWavBankMC);
  p->addModel(modelXY);
}
