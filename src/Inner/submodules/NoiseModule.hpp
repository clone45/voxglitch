#pragma once
#include "../BaseModule.hpp"

class NoiseModule : public BaseModule 
{
    public:
    
    float sample_rate;
    
    enum NoiseType {
        WHITE,
        PINK,
        BROWN,
        NUMBER_OF_NOISE_TYPES
    };

    enum INPUTS {
        TYPE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        AMPLITUDE_PARAM,
        TYPE_PARAM,
        NUM_PARAMS
    };

    NoiseModule() 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        params[AMPLITUDE_PARAM]->setValue(1.0f);
        params[TYPE_PARAM]->setValue(0.0f);
    }

    void process(unsigned int sample_rate) override 
    {
        this->sample_rate = sample_rate;

        // Get input values
        float amplitude = params[AMPLITUDE_PARAM]->getValue();
        float noise_type_voltage = inputs[TYPE]->isConnected() ? inputs[TYPE]->getVoltage() : params[TYPE_PARAM]->getValue();

        // Clamp noise voltage to [0,1] range
        noise_type_voltage = clamp(noise_type_voltage, 0.0f, 1.0f);

        // convert noise voltage to noise type
        NoiseType noise_type = noise_type_voltage < 0.33f ? WHITE : noise_type_voltage < 0.66f ? PINK : BROWN;

        // Generate noise signal
        float noise_signal;

        switch (noise_type) {
            case WHITE:
                noise_signal = whiteNoise();
                break;
            case PINK:
                noise_signal = pinkNoise();
                break;
            case BROWN:
                noise_signal = brownNoise();
                break;
            default:
                noise_signal = 0.0f;
                break;
        }

        // Set output value
        outputs[OUTPUT]->setVoltage(noise_signal * 5.0f * amplitude);
    }

    float whiteNoise()
    {
        float noise_signal = (float)rand() / (float)RAND_MAX;
        noise_signal = 2.0f * noise_signal - 1.0f;
        return(noise_signal);
    }

    float pinkNoise()
    {
        static float b0, b1, b2, b3, b4, b5, b6;
        float white = (float)rand() / (float)RAND_MAX;
        b0 = 0.99886f * b0 + white * 0.0555179f;
        b1 = 0.99332f * b1 + white * 0.0750759f;
        b2 = 0.96900f * b2 + white * 0.1538520f;
        b3 = 0.86650f * b3 + white * 0.3104856f;
        b4 = 0.55000f * b4 + white * 0.5329522f;
        b5 = -0.7616f * b5 - white * 0.0168980f;
        float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
        pink *= 0.01f;
        b6 = white * 0.115926f;
        return pink;
    }

    float brownNoise()
    {
        static float last_value = 0.0f;
        float white = (float)rand() / (float)RAND_MAX;
        float brown = (last_value + (0.02f * white)) / 1.02f;
        last_value = brown;
        return brown;
    }   
};
