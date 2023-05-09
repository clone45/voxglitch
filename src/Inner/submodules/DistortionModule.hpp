#pragma once

#include <cmath>
#include "../BaseModule.hpp"

class DistortionModule : public BaseModule 
{

public:

    enum INPUTS {
        AUDIO,
        DRIVE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        DRIVE_PARAM,
        NUM_PARAMS
    };

    DistortionModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override
    {
        float input = inputs[AUDIO]->isConnected() ? inputs[AUDIO]->getVoltage() : 0.0f;
        float drive = inputs[DRIVE]->isConnected() ? (inputs[DRIVE]->getVoltage() / 10.0) : params[DRIVE_PARAM]->getValue();

        drive = mapDrive(drive);

        float output = distortion(input, drive);
        outputs[OUTPUT]->setVoltage(output);
    }

private:
    float distortion(float input, float drive) 
    {
        return 2.0f / (1.0f + std::exp(-drive * input)) - 1.0f;
    }

    float mapDrive(float drive)
    {
        return drive * 49.0f + 1.0f;
    }    
};