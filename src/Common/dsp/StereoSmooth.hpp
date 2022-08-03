#pragma once

struct StereoSmooth
{
  float smoothing_ramp = 0;
  float left_previous_voltage = 0;
  float right_previous_voltage = 0;

  void trigger()
  {
    smoothing_ramp = 0;
  }

  void process(float left_voltage, float right_voltage, float smooth_rate, float *left_output, float* right_output)
  {
    if(smoothing_ramp < 1)
    {
      smoothing_ramp += smooth_rate;

      // It's noteworthy that this line of code doesn't really have much effect
      // on CPU consumption.  If you comment it out, the module takes about the
      // same amount of CPU.
      left_voltage = (left_previous_voltage * (1.0f - smoothing_ramp)) + (left_voltage * smoothing_ramp);
      right_voltage = (right_previous_voltage * (1.0f - smoothing_ramp)) + (right_voltage * smoothing_ramp);
    }

    left_previous_voltage = left_voltage;
    right_previous_voltage = right_voltage;

    *left_output = left_voltage;
    *right_output = right_voltage;
  }

  void reset()
  {
    smoothing_ramp = 0;
    left_previous_voltage = 0;
    right_previous_voltage = 0;
  }
};
