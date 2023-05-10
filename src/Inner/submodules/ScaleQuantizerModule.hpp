#include "../BaseModule.hpp"
#include <vector>
#include <cmath>

class ScaleQuantizerModule : public BaseModule {
public:
    enum INPUTS {
        CV_INPUT,
        SCALE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        QUANTIZED_OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        CV_INPUT_PARAM,
        SCALE_PARAM,
        NUM_PARAMS
    };

    std::vector<float> notes;
    std::vector<std::vector<float>> scales = {
        {   // CHROMATIC
            -10.0, -9.917, -9.833, -9.75, -9.667, -9.583, -9.5, -9.417, -9.333, -9.25, -9.167, -9.083,
            -9.0, -8.917, -8.833, -8.75, -8.667, -8.583, -8.5, -8.417, -8.333, -8.25, -8.167, -8.083,
            -8.0, -7.917, -7.833, -7.75, -7.667, -7.583, -7.5, -7.417, -7.333, -7.25, -7.167, -7.083,
            -7.0, -6.917, -6.833, -6.75, -6.667, -6.583, -6.5, -6.417, -6.333, -6.25, -6.167, -6.083,
            -6.0, -5.917, -5.833, -5.75, -5.667, -5.583, -5.5, -5.417, -5.333, -5.25, -5.167, -5.083,
            -5.0, -4.917, -4.833, -4.75, -4.667, -4.583, -4.5, -4.417, -4.333, -4.25, -4.167, -4.083,
            -4.0, -3.917, -3.833, -3.75, -3.667, -3.583, -3.5, -3.417, -3.333, -3.25, -3.167, -3.083,
            -3.0, -2.917, -2.833, -2.75, -2.667, -2.583, -2.5, -2.417, -2.333, -2.25, -2.167, -2.083,
            -2.0, -1.917, -1.833, -1.75, -1.667, -1.583, -1.5, -1.417, -1.333, -1.25, -1.167, -1.083,
            -1.0, -0.917, -0.833, -0.75, -0.667, -0.583, -0.5, -0.417, -0.333, -0.25, -0.167, -0.083,
            0.0, 0.083, 0.167, 0.25, 0.333, 0.417, 0.5, 0.583, 0.667, 0.75, 0.833, 0.917,
            1.0, 1.083, 1.167, 1.25, 1.333, 1.417, 1.5, 1.583, 1.667, 1.75, 1.833, 1.917,
            2.0, 2.083, 2.167, 2.25, 2.333, 2.417, 2.5, 2.583, 2.667, 2.75, 2.833, 2.917,
            3.0, 3.083, 3.167, 3.25, 3.333, 3.417, 3.5, 3.583, 3.667, 3.75, 3.833, 3.917,
            4.0, 4.083, 4.167, 4.25, 4.333, 4.417, 4.5, 4.583, 4.667, 4.75, 4.833, 4.917,
            5.0, 5.083, 5.167, 5.25, 5.333, 5.417, 5.5, 5.583, 5.667, 5.75, 5.833, 5.917,
            6.0, 6.083, 6.167, 6.25, 6.333, 6.417, 6.5, 6.583, 6.667, 6.75, 6.833, 6.917,
            7.0, 7.083, 7.167, 7.25, 7.333, 7.417, 7.5, 7.583, 7.667, 7.75, 7.833, 7.917,
            8.0, 8.083, 8.167, 8.25, 8.333, 8.417, 8.5, 8.583, 8.667, 8.75, 8.833, 8.917,
            9.0, 9.083, 9.167, 9.25, 9.333, 9.417, 9.5, 9.583, 9.667, 9.75, 9.833, 9.917,
            10.0, 10.083, 10.167, 10.25, 10.333, 10.417, 10.5, 10.583, 10.667, 10.75, 10.833, 10.917
        }
    };

    ScaleQuantizerModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override 
    {
        float cv = inputs[CV_INPUT]->isConnected() ? inputs[CV_INPUT]->getVoltage() : 0;
        // float scale = inputs[SCALE]->isConnected() ? inputs[SCALE]->getVoltage() : 0;

        unsigned int scale = 0;

        // Limit the input CV to the range of 0 to 10 volts
        cv = clamp(cv, -10.0f, 10.0f);

        // Find the closest note in the scale to the input CV
        int scale_index = findClosestNoteIndex(cv, scales[scale]);

        // Get the quantized voltage
        float quantized_voltage = scales[scale][scale_index];

        // Output the quantized voltage
        outputs[QUANTIZED_OUTPUT]->setVoltage(quantized_voltage);
    }

private:

    // Find closest distance between the input cv and the notes in the scale
    // and return the index of the closest note

    int findClosestNoteIndex(float cv, std::vector<float> notes)
    {
        float closest_distance = 1000.0f;
        int closest_index = 0;

        for (int i = 0; i < (int) notes.size(); i++)
        {
            float distance = std::abs(cv - notes[i]);
            if (distance < closest_distance)
            {
                closest_distance = distance;
                closest_index = i;
            }
        }

        return closest_index;
    }


};