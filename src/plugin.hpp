#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
extern Model *modelRepeater;
extern Model *modelWavBank;
extern Model *modelXY;
extern Model *modelGhosts;
extern Model *modelGlitchSequencer;
extern Model *modelGoblins;
extern Model *modelGrainEngine;
extern Model *modelAutobreak;
extern Model *modelDigitalSequencer;
