#pragma once

struct SampleAndHold
{
    float value = 0;
    bool triggered = false;
    
    void trigger(float value)
    {
        this->value = value;
        triggered = true;
    }
    
    float getValue()
    {
        return(value);
    }

    // Nothing has been sampled yet, so held output is not meaningful.
    // Callers should pass the live signal through until this is true.
    bool hasSampled()
    {
        return(triggered);
    }

    void reset()
    {
        value = 0;
        triggered = false;
    }
};
