#pragma once

struct StereoPan
{
  void process(float *left_voltage, float *right_voltage, float pan)
  {
    if(pan > 0)
    {
      *left_voltage = *left_voltage * (1-pan);
    }
    else if(pan < 0)
    {
      *right_voltage = *right_voltage * (1 - abs(pan));
    }
  }
};
