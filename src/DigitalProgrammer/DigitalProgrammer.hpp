/*

DigitalProgrammer
By Bret Truchan
Special thanks to Andras Szabo (Firo Lightfog) for their creative input.


TODO: Give inputs and outputs friendly names
TODO: set default output range
TODO: saving and loading

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

  bool copy_paste_mode = false;
  unsigned int copy_bank_id = 0;

  DPSlider sliders[NUMBER_OF_BANKS][NUMBER_OF_SLIDERS];

  dsp::SchmittTrigger bank_next_schmitt_trigger;
  dsp::SchmittTrigger bank_prev_schmitt_trigger;
  dsp::SchmittTrigger bank_reset_schmitt_trigger;
  dsp::SchmittTrigger copy_mode_button_trigger;

  enum ParamIds {
    ENUMS(BANK_BUTTONS, NUMBER_OF_BANKS),
    COPY_MODE_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    BANK_CV_INPUT,
    BANK_NEXT_INPUT,
    BANK_PREV_INPUT,
    BANK_RESET_INPUT,
    POLY_ADD_INPUT,
    // COPY_MODE_INPUT,
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
    return json_root;
  }

  // Autoload settings
  void dataFromJson(json_t *json_root) override
  {
  }


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
    for(int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      sliders[destination_bank_id][column].setValue(sliders[source_bank_id][column].getValue());
    }
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
    if(inputs[BANK_CV_INPUT].isConnected())
    {
      unsigned int bank_cv_value = (inputs[BANK_CV_INPUT].getVoltage() / 10.0) * NUMBER_OF_BANKS;
      bank_cv_value = clamp(bank_cv_value, 0, NUMBER_OF_BANKS - 1);
      this->selected_bank = bank_cv_value;
    }

    if(bank_next_schmitt_trigger.process(inputs[BANK_NEXT_INPUT].getVoltage())) incrementBank();
    if(bank_prev_schmitt_trigger.process(inputs[BANK_PREV_INPUT].getVoltage())) decrementBank();
    if(bank_reset_schmitt_trigger.process(inputs[BANK_RESET_INPUT].getVoltage())) resetBank();

    inputs[POLY_ADD_INPUT].setChannels(NUMBER_OF_SLIDERS);

    // Process copy/paste button
    if(copy_mode_button_trigger.process(params[COPY_MODE_PARAM].getValue()))
    {
      copy_paste_mode = ! copy_paste_mode; // toggle off/on
      this->copy_bank_id = this->selected_bank;
    }

    // Output values
    for(int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      // Get voltage for the specific slider
      float output_voltage = sliders[selected_bank][column].getValue();

      // Add any value from the poly input
      output_voltage += inputs[POLY_ADD_INPUT].getVoltage(column);

      // Output voltage
      outputs[column].setVoltage(output_voltage);
      outputs[POLY_OUTPUT].setVoltage(output_voltage, column);
    }

    outputs[POLY_OUTPUT].setChannels(NUMBER_OF_SLIDERS);
    lights[COPY_MODE_LIGHT].setBrightness(copy_paste_mode == true);
  }

};
