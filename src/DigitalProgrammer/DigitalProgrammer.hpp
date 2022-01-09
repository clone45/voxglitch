/*

DigitalProgrammer
By Bret Truchan
Special thanks to Andras Szabo (Firo Lightfog) for their creative input.

*/

struct DigitalProgrammer : Module
{
  dsp::SchmittTrigger bank_button_triggers[NUMBER_OF_BANKS];
  unsigned int selected_bank = 0;
  int mouse_over_bank = -1;
  DPSlider sliders[NUMBER_OF_BANKS][NUMBER_OF_SLIDERS];

  dsp::SchmittTrigger bank_next_schmitt_trigger;
  dsp::SchmittTrigger bank_prev_schmitt_trigger;
  dsp::SchmittTrigger bank_reset_schmitt_trigger;

  enum ParamIds {
    ENUMS(BANK_BUTTONS, NUMBER_OF_BANKS),
    NUM_PARAMS
  };
  enum InputIds {
    BANK_CV_INPUT,
    BANK_NEXT_INPUT,
    BANK_PREV_INPUT,
    BANK_RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(CV_OUTPUTS, NUMBER_OF_SLIDERS),
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


  void increment_bank()
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

  void decrement_bank()
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

  void reset_bank()
  {
    selected_bank = 0;
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

    if(bank_next_schmitt_trigger.process(inputs[BANK_NEXT_INPUT].getVoltage())) increment_bank();
    if(bank_prev_schmitt_trigger.process(inputs[BANK_PREV_INPUT].getVoltage())) decrement_bank();
    if(bank_reset_schmitt_trigger.process(inputs[BANK_RESET_INPUT].getVoltage())) reset_bank();

    // Output values
    for(int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      outputs[column].setVoltage(sliders[selected_bank][column].getValue());
    }
  }

};
