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

struct StereoSmooth
{
    float smoothing_ramp = 0;
    float left_previous_voltage = 0;
    float right_previous_voltage = 0;

    void trigger()
    {
        smoothing_ramp = 0;
    }

    std::pair<float, float> process(float left_voltage, float right_voltage, float smooth_rate)
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

        return {left_voltage, right_voltage};
    }

    void reset()
    {
        smoothing_ramp = 0;
        left_previous_voltage = 0;
        right_previous_voltage = 0;
    }
};
