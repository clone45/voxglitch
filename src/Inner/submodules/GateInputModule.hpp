#pragma once 
#include <cmath>
#include "../BaseModule.hpp"

class GateInputModule : public BaseModule 
{
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

    float *gate_value_ptr = nullptr;

    GateInputModule(float *gate_ptr) 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        this->gate_value_ptr = gate_ptr;
    }

    void process(unsigned int sample_rate) override 
    {
        outputs[OUTPUT]->setValue(*gate_value_ptr);
    }    
};