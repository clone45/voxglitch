// src\modules\core\plugin.cpp
#define DR_MP3_IMPLEMENTATION
#include "vgLib-2.0/dr_mp3.h"
#include "plugin.hpp"
#include "vgLib-2.0/constants.cpp"

Plugin* pluginInstance;

void init(Plugin* p) {
  pluginInstance = p;

  // Core
  p->addModel(modelArpSeq);
  p->addModel(modelAutobreak);
  p->addModel(modelAutobreakStudio);  
  p->addModel(modelByteBeat);
  p->addModel(modelDigitalProgrammer);
  p->addModel(modelDigitalSequencer);
  p->addModel(modelDigitalSequencerXP);
  p->addModel(modelDrumRandomizer);
  p->addModel(modelCueResearch); 
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
  p->addModel(modelMaya);
  p->addModel(modelNoteDetector);
  p->addModel(modelRepeater);
  p->addModel(modelSamplerX8);
  p->addModel(modelSampler16P);
  p->addModel(modelSatanonaut);
  p->addModel(modelSatanonautUnearthed);
  p->addModel(modelVectorRotation);
  p->addModel(modelWavBank);
  p->addModel(modelWavBankMC);
  // p->addModel(modelNetrunner);
  p->addModel(modelTempestVS1);
  p->addModel(modelXY);
  p->addModel(modelKaiseki);

}
