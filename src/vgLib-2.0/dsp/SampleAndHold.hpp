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
};
