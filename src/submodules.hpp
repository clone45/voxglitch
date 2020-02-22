#define FADE_OUT_ACCUMULATOR 0.01f

struct SmoothSubModule
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

struct StereoSmoothSubModule
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

struct FadeOutSubModule
{
    float ramp = 1;

    void trigger()
    {
        ramp = 0;
    }

    float process(float voltage, float rate)
    {
        if(ramp < 1)
        {
            ramp += rate;
            voltage = voltage * (1.0f - ramp);
        }

        return voltage;
    }
};

struct StereoFadeOutSubModule
{
    float ramp = 0;

    void trigger()
    {
        ramp = 0;
    }

    std::pair<float, float> process(float left_voltage, float right_voltage, float rate)
    {
        if(ramp < 1)
        {
            ramp += rate;
            left_voltage = left_voltage * (1.0f - ramp);
            right_voltage = right_voltage * (1.0f - ramp);
            return {left_voltage, right_voltage};
        }
        else
        {
            return {0.0f, 0.0f};
        }
    }
};

struct StereoFadeInSubModule
{
    float ramp = 1;

    void trigger()
    {
        ramp = 0;
    }

    std::pair<float, float> process(float left_voltage, float right_voltage, float rate)
    {
        if(ramp < 1)
        {
            ramp += rate;
            left_voltage = left_voltage * ramp;
            right_voltage = right_voltage * ramp;
            return {left_voltage, right_voltage};
        }
        else
        {
            return {left_voltage, right_voltage};
        }
    }
};

struct StereoPanSubModule
{
    std::pair<float, float> process(float left_voltage, float right_voltage, float pan)
    {
        if(pan > 0)
        {
            return {left_voltage * (1-pan), right_voltage};
        }
        else if(pan < 0)
        {
            return {left_voltage, right_voltage * (1 - abs(pan))};
        }
        else
        {
            return {left_voltage, right_voltage};
        }
    }
};
