#pragma once

struct Smooth
{
    float loop_smoothing_ramp = 1;
    float previous_voltage = 0;

    void trigger()
    {
        loop_smoothing_ramp = 0;
    }

    float process(float voltage, float smooth_rate)
    {
        if (loop_smoothing_ramp >= 1)
        {
            previous_voltage = voltage;
            return voltage;
        }

        loop_smoothing_ramp += smooth_rate;

        voltage = (previous_voltage *  (1.0f - loop_smoothing_ramp)) + (voltage * loop_smoothing_ramp);
        previous_voltage = voltage;

        return voltage;
    }
};
