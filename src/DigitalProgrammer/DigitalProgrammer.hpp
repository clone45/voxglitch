/*

Where I left off:

The draw area for the slider is currently very wide.  That's certainly
an option, and would be good if people wanted to draw slopes and stuff,
but for a programmer, I actually do NOT want that behavior.  Next steps
is to set the draw area width to match the slider width.  After that,
it's probably time to add the additional sliders!

*/


struct DigitalProgrammer : Module
{
  // test

  DPSlider sliders[NUMBER_OF_SLIDERS];

  enum ParamIds {
    EXAMPLE_KNOB,
    NUM_PARAMS
  };
  enum InputIds {
    BANK_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CV_OUTPUT_0,
    CV_OUTPUT_1,
    CV_OUTPUT_2,
    CV_OUTPUT_3,
    CV_OUTPUT_4,
    CV_OUTPUT_5,
    CV_OUTPUT_6,
    CV_OUTPUT_7,
    CV_OUTPUT_8,
    CV_OUTPUT_9,
    CV_OUTPUT_10,
    CV_OUTPUT_11,
    NUM_OUTPUTS
  };

  enum LightIds {
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  DigitalProgrammer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  }


  /*
  ==================================================================================================================================================
    SAVE & LOAD
  ==================================================================================================================================================
  */


  json_t *dataToJson() override
  {
    json_t *json_root = json_object();
/*

    //
    // Save patterns
    //

    json_t *sequences_json_array = json_array();

    for(int sequencer_number=0; sequencer_number<NUMBER_OF_SEQUENCERS; sequencer_number++)
    {
      json_t *pattern_json_array = json_array();

      for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
      {
        json_array_append_new(pattern_json_array, json_integer(this->voltage_sequencers[sequencer_number].getValue(i)));
      }

      json_array_append_new(sequences_json_array, pattern_json_array);
    }

    json_object_set(json_root, "patterns", sequences_json_array);
    json_decref(sequences_json_array);
    */
    return json_root;
  }

  // Autoload settings
  void dataFromJson(json_t *json_root) override
  {
  /*
    //
    // Load patterns
    //

    json_t *pattern_arrays_data = json_object_get(json_root, "patterns");

    if(pattern_arrays_data)
    {
      size_t pattern_number;
      json_t *json_pattern_array;

      json_array_foreach(pattern_arrays_data, pattern_number, json_pattern_array)
      {
        for(int i=0; i<MAX_SEQUENCER_STEPS; i++)
        {
          this->voltage_sequencers[pattern_number].setValue(i, json_integer_value(json_array_get(json_pattern_array, i)));
        }
      }
    }
    */

  }


  /*

  ______
  | ___ \
  | |_/ / __ ___   ___ ___  ___ ___
  |  __/ '__/ _ \ / __/ _ \/ __/ __|
  | |  | | | (_) | (_|  __/\__ \__ \
  \_|  |_|  \___/ \___\___||___/___/


  */

  void process(const ProcessArgs &args) override
  {
    // yay!
  }

};
