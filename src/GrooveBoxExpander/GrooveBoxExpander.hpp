struct GrooveBoxExpander : Module
{
  dsp::SchmittTrigger mute_button_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger mute_cv_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger solo_button_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger solo_cv_triggers[NUMBER_OF_TRACKS];

  bool mutes[NUMBER_OF_TRACKS];
  bool solos[NUMBER_OF_TRACKS];
  bool send_update_to_groovebox = false;

  enum ParamIds {
    ENUMS(MUTE_BUTTONS, NUMBER_OF_TRACKS),
    ENUMS(SOLO_BUTTONS, NUMBER_OF_TRACKS),
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(MUTE_INPUTS, NUMBER_OF_TRACKS),
    ENUMS(SOLO_INPUTS, NUMBER_OF_TRACKS),
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(MUTE_BUTTON_LIGHTS, NUMBER_OF_TRACKS),
    ENUMS(SOLO_BUTTON_LIGHTS, NUMBER_OF_TRACKS),
    NUM_LIGHTS
  };

  // Constructor
  GrooveBoxExpander()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      mutes[i] = false;
      solos[i] = false;
    }
  }

  // Destructor
  ~GrooveBoxExpander()
  {
  }

	void process(const ProcessArgs &args) override
  {
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      //
      // Read mute buttons abd inputs
      //

      bool mute_button_pressed = mute_button_triggers[i].process(params[MUTE_BUTTONS + i].getValue());
      bool mute_button_triggered = mute_cv_triggers[i].process(rescale(inputs[MUTE_INPUTS + i].getVoltage(), 0.0f, 10.0f, 0.f, 1.f));

      if(mute_button_pressed || mute_button_triggered) {
         mutes[i] = !mutes[i];
         send_update_to_groovebox = true;
      }

      //
      // Read solo buttons and inputs
      //

      bool solo_button_pressed = solo_button_triggers[i].process(params[SOLO_BUTTONS + i].getValue());
      bool solo_button_triggered = solo_cv_triggers[i].process(rescale(inputs[SOLO_INPUTS + i].getVoltage(), 0.0f, 10.0f, 0.f, 1.f));

      if(solo_button_pressed || solo_button_triggered) {
         solos[i] = !solos[i];
         send_update_to_groovebox = true;
      }

      // Set mute and solo lights
      lights[MUTE_BUTTON_LIGHTS + i].setBrightness(mutes[i]);
      lights[SOLO_BUTTON_LIGHTS + i].setBrightness(solos[i]);
    }

    //
    // Send informatio to groovebox
    //
    if (send_update_to_groovebox && rightExpander.module && rightExpander.module->model == modelGrooveBox)
    {
      // Prepare message for sending to Grain Engine MK2
      GrooveBoxExpanderMessage *message_to_groove_box = (GrooveBoxExpanderMessage *) rightExpander.module->leftExpander.producerMessage;

      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        message_to_groove_box->mutes[i] = mutes[i];
        message_to_groove_box->solos[i] = solos[i];
      }

      // Tell GrooveBox that the message is ready for receiving
      message_to_groove_box->message_received = false;

      // Message has been sent, so flip flag send_update_to_groovebox to false
      send_update_to_groovebox = false;
    }
	}
};
