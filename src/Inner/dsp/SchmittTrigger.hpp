#pragma once

struct SchmittTrigger 
{
  float low_threshold = 0.25f;
  float high_threshold = 0.75f;
  bool state = false;

  bool process(float in) {
    if (state == false && in > high_threshold) {
      state = true;
      return true; // Only return true when the state changes from low to high
    }
    if (state == true && in < low_threshold) {
      state = false;
    }
    return false;
  }
};