#define DR_WAV_IMPLEMENTATION
#include "Common/dr_wav.h"

#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
  pluginInstance = p;

  // Add modules here
  p->addModel(modelRepeater);
  p->addModel(modelWavBank);
  p->addModel(modelXY);
  p->addModel(modelGlitchSequencer);
  p->addModel(modelGhosts);
  p->addModel(modelGoblins);
  p->addModel(modelGrainEngine);
  p->addModel(modelAutobreak);
  p->addModel(modelDigitalSequencer);
  p->addModel(modelSamplerX8);

  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
