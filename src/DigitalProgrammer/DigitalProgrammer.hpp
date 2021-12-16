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

  dsp::SchmittTrigger bank_1_button_trigger;
  dsp::SchmittTrigger bank_2_button_trigger;
  dsp::SchmittTrigger bank_3_button_trigger;
  dsp::SchmittTrigger bank_4_button_trigger;
  dsp::SchmittTrigger bank_5_button_trigger;
  dsp::SchmittTrigger bank_6_button_trigger;
  dsp::SchmittTrigger bank_7_button_trigger;
  dsp::SchmittTrigger bank_8_button_trigger;

  bool bank_1_button_is_triggered;
  bool bank_2_button_is_triggered;
  bool bank_3_button_is_triggered;
  bool bank_4_button_is_triggered;
  bool bank_5_button_is_triggered;
  bool bank_6_button_is_triggered;
  bool bank_7_button_is_triggered;
  bool bank_8_button_is_triggered;

  unsigned int selected_bank_index = 0;

  // test

  DPSlider sliders[NUMBER_OF_BANKS][NUMBER_OF_SLIDERS];

  enum ParamIds {
    BANK_1_BUTTON,
    BANK_2_BUTTON,
    BANK_3_BUTTON,
    BANK_4_BUTTON,
    BANK_5_BUTTON,
    BANK_6_BUTTON,
    BANK_7_BUTTON,
    BANK_8_BUTTON,
    NUM_PARAMS
  };
  enum InputIds {
    BANK_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    // Take care here that the CV outputs MUST be the first modules in this
    // list because the widget addresses these by index starting at 0
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
    BANK_1_LIGHT,
    BANK_2_LIGHT,
    BANK_3_LIGHT,
    BANK_4_LIGHT,
    BANK_5_LIGHT,
    BANK_6_LIGHT,
    BANK_7_LIGHT,
    BANK_8_LIGHT,
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

    for(int bank_number=0; bank_number<NUMBER_OF_SEQUENCERS; bank_number++)
    {
      json_t *pattern_json_array = json_array();

      for(int i=0; i<MAX_bank_STEPS; i++)
      {
        json_array_append_new(pattern_json_array, json_integer(this->voltage_sequencers[bank_number].getValue(i)));
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
        for(int i=0; i<MAX_bank_STEPS; i++)
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
    // Handle bank seletion
    //
    // See if someone pressed one of the green sequence selection buttons
    //
    bank_1_button_is_triggered = bank_1_button_trigger.process(params[BANK_1_BUTTON].getValue());
    bank_2_button_is_triggered = bank_2_button_trigger.process(params[BANK_2_BUTTON].getValue());
    bank_3_button_is_triggered = bank_3_button_trigger.process(params[BANK_3_BUTTON].getValue());
    bank_4_button_is_triggered = bank_4_button_trigger.process(params[BANK_4_BUTTON].getValue());
    bank_5_button_is_triggered = bank_5_button_trigger.process(params[BANK_5_BUTTON].getValue());
    bank_6_button_is_triggered = bank_6_button_trigger.process(params[BANK_6_BUTTON].getValue());
    bank_7_button_is_triggered = bank_7_button_trigger.process(params[BANK_7_BUTTON].getValue());
    bank_8_button_is_triggered = bank_8_button_trigger.process(params[BANK_8_BUTTON].getValue());

    // If any of the green sequence buttons were pressed, set the index "selected_bank_index"
    // which will be used to look up the selected voltage and gate sequencers from
    // the voltage_sequencers[] and gate_sequencers[] arrays

    if(bank_1_button_is_triggered) selected_bank_index = 0;
    if(bank_2_button_is_triggered) selected_bank_index = 1;
    if(bank_3_button_is_triggered) selected_bank_index = 2;
    if(bank_4_button_is_triggered) selected_bank_index = 3;
    if(bank_5_button_is_triggered) selected_bank_index = 4;
    if(bank_6_button_is_triggered) selected_bank_index = 5;
    if(bank_7_button_is_triggered) selected_bank_index = 6;
    if(bank_8_button_is_triggered) selected_bank_index = 7;

    // Output values
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      outputs[column].setVoltage(sliders[selected_bank_index][column].getValue());
    }

    lights[BANK_1_LIGHT].setBrightness(selected_bank_index == 0);
    lights[BANK_2_LIGHT].setBrightness(selected_bank_index == 1);
    lights[BANK_3_LIGHT].setBrightness(selected_bank_index == 2);
    lights[BANK_4_LIGHT].setBrightness(selected_bank_index == 3);
    lights[BANK_5_LIGHT].setBrightness(selected_bank_index == 4);
    lights[BANK_6_LIGHT].setBrightness(selected_bank_index == 5);
    lights[BANK_7_LIGHT].setBrightness(selected_bank_index == 6);
    lights[BANK_8_LIGHT].setBrightness(selected_bank_index == 7);
  }

};
