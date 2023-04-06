#pragma once
#include <cmath>
#include "../BaseModule.hpp"

class ParamModule : public BaseModule {

private:

public:

    float *parameter_value_ptr = nullptr;

    enum INPUTS {
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    ParamModule(float *p) 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        this->parameter_value_ptr = p;
    }

    void process(unsigned int sample_rate) override 
    {
        outputs[OUTPUT]->setVoltage(*parameter_value_ptr);
    }  
};