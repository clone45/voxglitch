#pragma once
#include <cmath>
#include "../BaseModule.hpp"

class PitchInputModule : public BaseModule {

private:

public:

    float *pitch_value_ptr = nullptr;

    enum INPUTS 
    {
        NUM_INPUTS
    };

    enum OUTPUTS 
    {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS 
    {
        NUM_PARAMS
    };

    PitchInputModule(float *pitch_ptr) 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        this->pitch_value_ptr = pitch_ptr;
    }

    void process(unsigned int sample_rate) override 
    {
        outputs[OUTPUT]->setVoltage(*pitch_value_ptr);
    }
};