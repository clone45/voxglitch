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

    const float VOLTAGE_RANGE = 5.0f;
    std::vector<float> notes;
    std::vector<std::vector<int>> scales = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, // CHROMATIC
        {0, 2, 4, 5, 7, 9, 11},                // IONIAN
        {0, 2, 3, 5, 7, 9, 10},                // DORIAN
        {0, 2, 4, 6, 7, 9, 11},                // LYDIAN
        {0, 1, 3, 5, 7, 8, 10},                // PHRYGIAN
        {0, 2, 4, 5, 7, 9, 10},                // MIXOLYDIAN
        {0, 2, 3, 5, 7, 8, 10},                // AEOLIAN
        {0, 1, 3, 5, 6, 8, 10},                // LOCRIAN
        {0, 2, 4, 7, 9},                       // MAJOR_PENTATONIC
        {0, 3, 5, 7, 10},                      // MINOR_PENTATONIC
        {0, 3, 5, 6, 7, 10},                   // BLUES
        {0, 2, 3, 5, 6, 8, 9, 11},             // DIMINISHED
        {0, 1, 4, 5, 7, 8, 11},                // ARABIAN
        {0, 4, 7},                             // MAJOR
        {0, 3, 7}                              // MINOR
    };

    ScaleQuantizerModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        // Fill the notes vector using math
        for (int note_number = 0; note_number <= 240; ++note_number) 
        {
            float voltage = (note_number - 69) / 12.0f;
            notes.push_back(voltage);
        }
    }

    void process(unsigned int sample_rate) override 
    {
        float cv = inputs[CV_INPUT]->isConnected() ? inputs[CV_INPUT]->getVoltage() : params[CV_INPUT]->getValue();
        float scale = inputs[SCALE]->isConnected() ? inputs[SCALE]->getVoltage() : params[SCALE]->getValue();

        // Limit the input CV to the range of 0 to 10 volts
        cv = clamp(cv, -10.0f, 10.0f);

        // Limit the scale to the range of 0 to 10 volts
        scale = clamp(scale, 0.0f, 10.0f);

        // Select the scale based on the SCALE input
        std::vector<int> selected_scale = scales[static_cast<int>(scale / 10.0f * scales.size())];

        // Find the closest note in the scale to the input CV
        float closestNoteVoltage = 0.0f;
        float closestNoteDistance = 1000.0f;

        for (int note_index = 0; note_index < (int) notes.size(); ++note_index) {
            float note_voltage = notes[note_index];
            float distance = std::abs(note_voltage - cv);

            if (distance < closestNoteDistance) {
                closestNoteDistance = distance;
                closestNoteVoltage = note_voltage;
            }
        }

        outputs[QUANTIZED_OUTPUT]->setVoltage(closestNoteVoltage);
    }
};