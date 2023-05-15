#pragma once
#include "../BaseModule.hpp"

class ConstantModule : public BaseModule {
public:
    enum OUTPUTS {
        OUTPUT,
        NUM_OUTPUTS
    };

    ConstantModule(json_t* data) 
    {
        config(0, 0, NUM_OUTPUTS);  // No parameters or inputs for this module

        //
        // Read the value from the JSON "data"
        // section of the patch file.  Store it in the "value"
        // variable as a float.
        //

        json_t* value_json = json_object_get(data, "value");
        if (value_json) 
        {
            value = json_number_value(value_json);
        }
        else
        {
            DEBUG("No value found in JSON data");
        }
    }

    void process(unsigned int sample_rate) override 
    {
        // Set the output to the value.
        outputs[OUTPUT]->setVoltage(value);
    }

private:
    float value = 0.0f;
};
