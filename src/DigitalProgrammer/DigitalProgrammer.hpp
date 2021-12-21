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
  dsp::SchmittTrigger bank_button_triggers[NUMBER_OF_BANKS];
  // bool bank_button_is_triggered[NUMBER_OF_BANKS];

  int selected_bank = 0;

  DPSlider sliders[NUMBER_OF_BANKS][NUMBER_OF_SLIDERS];

  enum ParamIds {
    ENUMS(BANK_BUTTONS, NUMBER_OF_BANKS),
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
    ENUMS(BANK_LIGHTS, NUMBER_OF_BANKS),
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  DigitalProgrammer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);


    for(int i=0; i<NUMBER_OF_BANKS; i++)
    {
      configSwitch(BANK_BUTTONS + i, 0.f, 1.f, 0.f, string::f("%d mute", i + 1));
			//configInput(IN_INPUTS + i, string::f("Row %d", i + 1));
		//	configOutput(OUT_OUTPUTS + i, string::f("Row %d", i + 1));
    }

    for(int i = 0; i < NUMBER_OF_BANKS; i++)
    {
      params[BANK_BUTTONS + i].setValue(0.0);
    }
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
    /*
    // Handle bank seletion
    //
    // See if someone pressed one of the green sequence selection buttons
    //
    for(unsigned int trigger_index=0; trigger_index < NUMBER_OF_BANKS; trigger_index++)
    {
      bank_button_is_triggered[trigger_index] = bank_button_triggers[trigger_index].process(params[trigger_index].getValue());
    }

    // If any of the green sequence buttons were pressed, set the index "selected_bank_index"
    // which will be used to look up the selected voltage and gate sequencers from
    // the voltage_sequencers[] and gate_sequencers[] arrays
    for(unsigned int trigger_index=0; trigger_index < NUMBER_OF_BANKS; trigger_index++)
    {
      if(bank_button_is_triggered[trigger_index]) selected_bank_index = trigger_index;
    }
    */

    // bool button_pressed = false;

    for(int i = 0; i < NUMBER_OF_BANKS; i++)
    {
      // reference: sequencer_1_button_is_triggered = sequencer_1_button_trigger.process(params[SEQUENCER_1_BUTTON].getValue());

      if(bank_button_triggers[i].process(params[BANK_BUTTONS + i].getValue()))
      {
        selected_bank = i;
      }
    }


    // DEBUG(std::to_string(params[0].getValue()).c_str());

    // Output values
    for(int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      outputs[column].setVoltage(sliders[selected_bank][column].getValue());
    }

    // Set brightness of lamps
    for(int light_index=0; light_index < NUMBER_OF_BANKS; light_index++)
    {
      lights[BANK_LIGHTS + light_index].setBrightness(selected_bank == light_index);
    }
  }

};
