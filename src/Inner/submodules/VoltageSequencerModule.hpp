#pragma once
#include "../BaseModule.hpp"
#include "../dsp/SchmittTrigger.hpp"

class VoltageSequencerModule : public BaseModule {
public:
    enum INPUTS {
        CLOCK,
        RESET,
        NUM_INPUTS
    };

    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    enum PARAMS {
        NUM_PARAMS
    };

    VoltageSequencerModule(json_t* data) 
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        //
        // Read the sequence information from the JSON "data"
        // section of the patch file.  Store it in the "sequence"
        // vector as a series of floats.
        //

        json_t* sequence = json_object_get(data, "sequence");
        if (sequence) 
        {
            json_t* step;
            size_t index;
            json_array_foreach(sequence, index, step) 
            {
                this->sequence.push_back(json_number_value(step));
            }
        }
        else
        {
            DEBUG("No sequence found in JSON data");
        }
    }

    void process(unsigned int sample_rate) override 
    {
        // If the sequence is empty, then just output 0.0 volts.
        if (sequence.size() == 0) 
        {
            outputs[OUTPUT]->setVoltage(0.0f);
            return;
        }

        float clock = inputs[CLOCK]->getVoltage();
        float reset = inputs[RESET]->getVoltage();

        if (clock_input_schmitt_trigger.process(clock)) 
        {
            current_step++;
            if (current_step >= (int) sequence.size())  current_step = 0;
        }

        if (reset_input_schmitt_trigger.process(reset)) current_step = 0;

        outputs[OUTPUT]->setVoltage(sequence[current_step]);
    }

private:
    int current_step = 0;
    std::vector<float> sequence;
    SchmittTrigger clock_input_schmitt_trigger;
    SchmittTrigger reset_input_schmitt_trigger;
};