#include "../BaseModule.hpp"
#include <vector>
#include <cmath>

class ScaleQuantizerModule : public BaseModule {
public:
    enum INPUTS {
        CV_INPUT,
        SCALE,
        ROOT_NOTE,
        OCTAVE,
        NUM_INPUTS
    };

    enum OUTPUTS {
        QUANTIZED_OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    ScaleQuantizerModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    }

    void process(unsigned int sample_rate) override {
        float cv = inputs[CV_INPUT]->isConnected() ? inputs[CV_INPUT]->getVoltage() : 0.0f;
        int scaleSelection = static_cast<int>(inputs[SCALE]->isConnected() ? inputs[SCALE]->getVoltage() : 0.0f);
        int rootNote = static_cast<int>(inputs[ROOT_NOTE]->isConnected() ? inputs[ROOT_NOTE]->getVoltage() : 0.0f);
        int octave = static_cast<int>(inputs[OCTAVE]->isConnected() ? inputs[OCTAVE]->getVoltage() : 0.0f);

        const std::vector<std::vector<float>> scales = {
            {0, 2, 4, 5, 7, 9, 11}, // Major
            {0, 2, 3, 5, 7, 8, 10}, // Minor
            {0, 2, 4, 7, 9},        // Pentatonic
            {0, 3, 6, 9, 12},       // Diminished
            {0, 1, 3, 5, 6, 8, 10}  // Harmonic minor
        };

        const auto& scale = scales[scaleSelection % scales.size()];

        float inVoltage = cv + rootNote + 12.0f * octave;
        int inNote = std::round(inVoltage);
        float closestNoteDiff = 1e9;
        int closestNote = 0;

        for (const auto& note : scale) {
            int scaledNote = inNote - static_cast<int>(note);
            int quantizedNote = static_cast<int>(note) + 12 * (scaledNote / 12);
            float diff = std::abs(inVoltage - static_cast<float>(quantizedNote));

            if (diff < closestNoteDiff) {
                closestNoteDiff = diff;
                closestNote = quantizedNote;
            }
        }

        outputs[QUANTIZED_OUTPUT]->setVoltage(static_cast<float>(closestNote));
    }
};
