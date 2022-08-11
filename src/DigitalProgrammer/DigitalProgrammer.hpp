/*

DigitalProgrammer
By Bret Truchan
- Special thanks to Andras Szabo (Firo Lightfog) for their creative input.
- Thank you to Guenon from the VCV Community for sharing their feedback.
*/

struct DigitalProgrammer : Module
{
  dsp::SchmittTrigger bank_button_triggers[NUMBER_OF_BANKS];
  unsigned int selected_bank = 0;

  // Mouse over tracking
  unsigned int mouse_over_bank = 0;
  bool is_moused_over_bank = false;
  unsigned int moused_over_slider = 0;
  bool is_moused_over_slider = false;

  unsigned int bank_interaction_mode = SELECT_MODE;

  // Copy/paste tracking
  unsigned int copy_bank_id = 0;

  // Context menu options
  bool visualize_sums = false;
  bool colorful_sliders = false;

  PortWidget *output_ports[NUMBER_OF_SLIDERS];

  unsigned int snap_settings[NUMBER_OF_SLIDERS] = {0};
  unsigned int range_settings[NUMBER_OF_SLIDERS] = {0};

  DPSlider sliders[NUMBER_OF_BANKS][NUMBER_OF_SLIDERS];
  float add_input_voltages[NUMBER_OF_SLIDERS] = {0};

  std::string snap_division_names[NUMBER_OF_SNAP_DIVISIONS] = { "None", "32", "16", "8", "4" };
  std::string labels[NUMBER_OF_SLIDERS] = {"","","","","","","","","","","","","","","",""};

  dsp::SchmittTrigger bank_next_input_schmitt_trigger;
  dsp::SchmittTrigger bank_next_button_schmitt_trigger;
  dsp::SchmittTrigger bank_prev_input_schmitt_trigger;
  dsp::SchmittTrigger bank_prev_button_schmitt_trigger;
  dsp::SchmittTrigger bank_reset_schmitt_trigger;
  dsp::SchmittTrigger copy_mode_button_trigger;
  dsp::SchmittTrigger clear_mode_button_trigger;
  dsp::SchmittTrigger randomize_mode_button_trigger;

  std::string voltage_range_names[NUMBER_OF_VOLTAGE_RANGES] = {
    "0.0 to 10.0",
    "-10.0 to 10.0",
    "0.0 to 5.0",
    "-5.0 to 5.0",
    "0.0 to 3.0",
    "-3.0 to 3.0",
    "0.0 to 1.0",
    "-1.0 to 1.0"
  };

  enum ParamIds {
    ENUMS(BANK_BUTTONS, NUMBER_OF_BANKS),
    COPY_MODE_PARAM,
    CLEAR_MODE_PARAM,
    RANDOMIZE_MODE_PARAM,
    BANK_PREV_PARAM,
    BANK_NEXT_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    BANK_CV_INPUT,
    BANK_NEXT_INPUT,
    BANK_PREV_INPUT,
    BANK_RESET_INPUT,
    POLY_ADD_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(CV_OUTPUTS, NUMBER_OF_SLIDERS),
    POLY_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    ENUMS(BANK_LIGHTS, NUMBER_OF_BANKS),
    COPY_MODE_LIGHT,
    CLEAR_MODE_LIGHT,
    RANDOMIZE_MODE_LIGHT,
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  DigitalProgrammer()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configInput(POLY_ADD_INPUT, "Add To CV (poly)");
    configInput(BANK_CV_INPUT, "CV control over selected bank");
    configInput(BANK_NEXT_INPUT, "Step to the next bank");
    configInput(BANK_PREV_INPUT, "Step to the previous bank");
    configInput(BANK_RESET_INPUT, "Reset to the 1st bank");
  }


  /*
  ==================================================================================================================================================
    SAVE & LOAD
  ==================================================================================================================================================
  */

  json_t *dataToJson() override
  {
    json_t *json_root = json_object();

    //
    //  Save all of the programmer data
    //

    json_t *banks_json_array = json_array();

    for(unsigned int bank_number = 0; bank_number < NUMBER_OF_BANKS; bank_number++)
    {
      json_t *sliders_json_array = json_array();

      for(unsigned int i = 0; i < NUMBER_OF_SLIDERS; i++)
      {
        json_array_append_new(sliders_json_array, json_real(sliders[bank_number][i].getValue()));
      }
      json_array_append_new(banks_json_array, sliders_json_array);
    }

    json_object_set(json_root, "banks", banks_json_array);
    json_decref(banks_json_array);

    //
    // Save the labels
    //
    json_t *labels_json_array = json_array();
    for(unsigned int i = 0; i < NUMBER_OF_SLIDERS; i++)
    {
      json_array_append_new(labels_json_array, json_string(labels[i].c_str()));
    }
    json_object_set(json_root, "labels", labels_json_array);
    json_decref(labels_json_array);

    // Save the selected bank
    json_object_set_new(json_root, "selected_bank", json_integer(this->selected_bank));

    // Save colorful slider mode
    json_object_set_new(json_root, "colorful_sliders", json_integer(colorful_sliders));

    // Save visualize sums
    json_object_set_new(json_root, "visualize_sums", json_integer(visualize_sums));

    //
    // Save sequencer voltage range index selections
    //
    json_t *slider_voltage_range_json_array = json_array();
    for(unsigned int slider_number = 0; slider_number < NUMBER_OF_SLIDERS; slider_number++)
    {
      json_array_append_new(slider_voltage_range_json_array, json_integer(this->range_settings[slider_number]));
    }
    json_object_set(json_root, "voltage_ranges", slider_voltage_range_json_array);
    json_decref(slider_voltage_range_json_array);

    return json_root;
  }

  // Autoload settings
  void dataFromJson(json_t *json_root) override
  {

    //
    //  Load all of the programmer data
    //

    json_t *banks_arrays_data = json_object_get(json_root, "banks");

    if(banks_arrays_data)
    {
      size_t bank_number;
      json_t *json_slider_array;

      json_array_foreach(banks_arrays_data, bank_number, json_slider_array)
      {
        for(unsigned int i=0; i<NUMBER_OF_SLIDERS; i++)
        {
          this->sliders[bank_number][i].setValue(json_real_value(json_array_get(json_slider_array, i)));
        }
      }
    }

    //
    // Load the labels
    //
    json_t *labels_json = json_object_get(json_root, "labels");

    if(labels_json)
    {
      size_t i;
      json_t *label_json;

      json_array_foreach(labels_json, i, label_json)
      {
        labels[i] = json_string_value(label_json);
      }
    }

    // load selected bank
    json_t* selected_bank_json = json_object_get(json_root, "selected_bank");
    if (selected_bank_json) this->selected_bank = json_integer_value(selected_bank_json);

    // Load colorful sliders option
    json_t* colorful_slider_json = json_object_get(json_root, "colorful_sliders");
    if (colorful_slider_json) colorful_sliders = json_integer_value(colorful_slider_json);

    json_t* visualize_sums_json = json_object_get(json_root, "visualize_sums");
    if (visualize_sums_json) visualize_sums = json_integer_value(visualize_sums_json);


    //
    // Load voltage ranges
    //
    json_t *voltage_ranges_json_array = json_object_get(json_root, "voltage_ranges");

    if(voltage_ranges_json_array)
    {
      size_t slider_number;
      json_t *voltage_range_json;

      json_array_foreach(voltage_ranges_json_array, slider_number, voltage_range_json)
      {
        this->range_settings[slider_number] = json_integer_value(voltage_range_json);
      }
    }
  }

  /*
  ==================================================================================================================================================
    Helper Functions
  ==================================================================================================================================================
  */

  void incrementBank()
  {
    if(selected_bank < (NUMBER_OF_BANKS - 1))
    {
      selected_bank++;
    }
    else
    {
      selected_bank = 0;
    }
  }

  void decrementBank()
  {
    if(selected_bank > 0)
    {
      selected_bank--;
    }
    else
    {
      selected_bank = NUMBER_OF_BANKS - 1;
    }
  }

  void resetBank()
  {
    selected_bank = 0;
  }

  void copyBank(unsigned int source_bank_id, unsigned int destination_bank_id)
  {
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      sliders[destination_bank_id][column].setValue(sliders[source_bank_id][column].getValue());
    }
  }

  void clearBank(unsigned int bank_id)
  {
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      sliders[bank_id][column].setValue(0);
    }
  }

  void randomizeBank(unsigned int bank_id)
  {
    for(unsigned int slider = 0; slider < NUMBER_OF_SLIDERS; slider ++)
    {
      this->sliders[bank_id][slider].setValue((std::rand() % 100) / 100.0);
    }
  }

  void onRandomize() override
  {
    for(unsigned int bank = 0; bank < NUMBER_OF_BANKS; bank++)
    {
      for(unsigned int slider = 0; slider < NUMBER_OF_SLIDERS; slider ++)
      {
        this->sliders[bank][slider].setValue((std::rand() % 100) / 100.0);
      }
    }
  }

  void onReset(const ResetEvent &e) override
  {
    for(unsigned int bank = 0; bank < NUMBER_OF_BANKS; bank++)
    {
      for(unsigned int slider = 0; slider < NUMBER_OF_SLIDERS; slider ++)
      {
        this->sliders[bank][slider].setValue(0);
      }
    }
    Module::onReset(e);
  }

  /*
  ==================================================================================================================================================
    Process
  ==================================================================================================================================================
  */

  void process(const ProcessArgs &args) override
  {

    //
    // Process bank controls
    //

    if(inputs[BANK_CV_INPUT].isConnected())
    {
      unsigned int bank_cv_value = (inputs[BANK_CV_INPUT].getVoltage() / 10.0) * NUMBER_OF_BANKS;
      bank_cv_value = clamp(bank_cv_value, 0, NUMBER_OF_BANKS - 1);
      this->selected_bank = bank_cv_value;
    }

    if(
      bank_next_input_schmitt_trigger.process(inputs[BANK_NEXT_INPUT].getVoltage()) ||
      bank_next_button_schmitt_trigger.process(params[BANK_NEXT_PARAM].getValue())
    ) incrementBank();

    if(
      bank_prev_input_schmitt_trigger.process(inputs[BANK_PREV_INPUT].getVoltage()) ||
      bank_prev_button_schmitt_trigger.process(params[BANK_PREV_PARAM].getValue())
    ) decrementBank();

    if(bank_reset_schmitt_trigger.process(inputs[BANK_RESET_INPUT].getVoltage())) resetBank();

    // Set input channels for poly add input
    inputs[POLY_ADD_INPUT].setChannels(NUMBER_OF_SLIDERS);

    //
    // Process bank mode buttons
    //

    if(copy_mode_button_trigger.process(params[COPY_MODE_PARAM].getValue()))
    {
      bank_interaction_mode = COPY_MODE;
      copy_bank_id = selected_bank;
      params[CLEAR_MODE_PARAM].setValue(false);
      params[RANDOMIZE_MODE_PARAM].setValue(false);
    }
    else if(clear_mode_button_trigger.process(params[CLEAR_MODE_PARAM].getValue()))
    {
      bank_interaction_mode = CLEAR_MODE;
      params[COPY_MODE_PARAM].setValue(false);
      params[RANDOMIZE_MODE_PARAM].setValue(false);
    }
    else if(randomize_mode_button_trigger.process(params[RANDOMIZE_MODE_PARAM].getValue()))
    {
      bank_interaction_mode = RANDOMIZE_MODE;
      params[COPY_MODE_PARAM].setValue(false);
      params[CLEAR_MODE_PARAM].setValue(false);
    }

    // If all buttons are disabled, set mode to select mode
    if(! (params[CLEAR_MODE_PARAM].getValue() || params[COPY_MODE_PARAM].getValue() || params[RANDOMIZE_MODE_PARAM].getValue())) bank_interaction_mode = SELECT_MODE;

    //
    // Output values
    //

    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      // Get voltage for the specific slider
      float scaled_output = sliders[selected_bank][column].getOutput(range_settings[column]);

      // Add any value from the poly input
      float add_input_voltage = inputs[POLY_ADD_INPUT].getVoltage(column);
      this->add_input_voltages[column] = add_input_voltage;
      scaled_output += add_input_voltage;

      // Output voltage
      outputs[column].setVoltage(scaled_output);
      outputs[POLY_OUTPUT].setVoltage(scaled_output, column); // range from 0 to 10v
    }

    // output voltages and manage lights
    outputs[POLY_OUTPUT].setChannels(NUMBER_OF_SLIDERS);
  }

};
