#pragma once
#include <string>
#include "../BaseModule.hpp"

class NoiseModule : public BaseModule {

private:
    float sample_rate;

public:

    enum NoiseType {
        WHITE,
        PINK,
        BROWN,
        NUMBER_OF_NOISE_TYPES
    };

    Sport *output_port = new Sport(this);
    Sport *noise_type_input_port = new Sport(this);

    NoiseModule() {
        setParameter("amplitude", 1.0f);
        setParameter("noise_voltage", 0.0);
    }

    void process(unsigned int sample_rate) override 
    {
        this->sample_rate = sample_rate;

        // Get input values
        float amplitude = getParameter("amplitude");
        float noise_type_voltage = noise_type_input_port->isConnected() ? noise_type_input_port->getValue() : getParameter("noise_voltage");

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
        output_port->setValue(noise_signal * 5.0f * amplitude);
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

    Sport* getPortByName(std::string portName) override {
        if (portName == "OUTPUT_PORT") {
            return output_port;
        } else if (portName == "NOISE_TYPE_INPUT_PORT") {
            return noise_type_input_port;
        } else {
            return nullptr;
        }
    }

    std::vector<Sport*> getOutputPorts() override {
        std::vector<Sport*> ports;
        ports.push_back(output_port);
        return ports;
    }

    std::vector<Sport*> getInputPorts() override {
        std::vector<Sport*> ports;
        ports.push_back(noise_type_input_port);
        return ports;
    }    
};
