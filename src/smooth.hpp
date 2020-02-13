struct Smooth
{
    float loop_smoothing_ramp = 0;
    float previous_voltage = 0;

    void trigger()
    {
        loop_smoothing_ramp = 0;
    }

    float process(float voltage, float smooth_rate)
    {
        if(loop_smoothing_ramp < 1)
        {
            loop_smoothing_ramp += smooth_rate;

            // It's noteworthy that this line of code doesn't really have much effect
            // on CPU consumption.  If you comment it out, the module takes about the
            // same amount of CPU.
            voltage = (previous_voltage * (1.0f - loop_smoothing_ramp)) + (voltage * loop_smoothing_ramp);
        }

        previous_voltage = voltage;
        return voltage;
    }

    void reset()
    {
        loop_smoothing_ramp = 0;
        previous_voltage = 0;
    }
};
