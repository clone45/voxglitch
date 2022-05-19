struct GrooveBoxExpander : Module
{
  dsp::SchmittTrigger mute_button_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger mute_cv_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger solo_button_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger solo_cv_triggers[NUMBER_OF_TRACKS];
  dsp::PulseGenerator triggerOutputPulseGenerators[NUMBER_OF_TRACKS];

  GrooveBoxMessage *producer_message = new GrooveBoxMessage;
  GrooveBoxMessage *consumer_message = new GrooveBoxMessage;

  bool mutes[NUMBER_OF_TRACKS];
  bool solos[NUMBER_OF_TRACKS];
  bool send_update_to_groovebox = false;
  float old_track_volumes[NUMBER_OF_TRACKS];
  bool track_triggers[NUMBER_OF_TRACKS];

  enum ParamIds {
    ENUMS(MUTE_BUTTONS, NUMBER_OF_TRACKS),
    ENUMS(SOLO_BUTTONS, NUMBER_OF_TRACKS),
    ENUMS(VOLUME_KNOBS, NUMBER_OF_TRACKS),
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(MUTE_INPUTS, NUMBER_OF_TRACKS),
    ENUMS(SOLO_INPUTS, NUMBER_OF_TRACKS),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(TRIGGER_OUTPUTS, NUMBER_OF_TRACKS),
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
      old_track_volumes[i] = 1.0;
      configParam(VOLUME_KNOBS + i, 0.0, 2.0, 1.0, "Track Volume");
    }

    producer_message->message_received = true;
  }

  // Destructor
  ~GrooveBoxExpander()
  {
    delete producer_message;
    delete consumer_message;
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

      //
      // See if track volumes have changed
      //

      float new_track_volume = params[VOLUME_KNOBS + i].getValue();
      if(old_track_volumes[i] != new_track_volume)
      {
        old_track_volumes[i] = new_track_volume;
        send_update_to_groovebox = true;
      }

      // Set mute and solo lights
      lights[MUTE_BUTTON_LIGHTS + i].setBrightness(mutes[i]);
      lights[SOLO_BUTTON_LIGHTS + i].setBrightness(solos[i]);
    }

    //
    // Send information to groovebox
    //
    if (send_update_to_groovebox)
    {
      writeToGroovebox();
      send_update_to_groovebox = false;
    }

    readFromGroovebox();

    // This is required after the flip
    rightExpander.producerMessage = producer_message;
    rightExpander.consumerMessage = consumer_message;

    // Output gates
    bool trigger_output_pulse = false;

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      trigger_output_pulse = triggerOutputPulseGenerators[i].process(args.sampleTime);
      outputs[TRIGGER_OUTPUTS + i].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }
	}

  void writeToGroovebox()
  {
    if (rightExpander.module && rightExpander.module->model == modelGrooveBox)
    {
      // Prepare message for sending to Grain Engine MK2
      GrooveBoxExpanderMessage *message_to_groove_box = (GrooveBoxExpanderMessage *) rightExpander.module->leftExpander.producerMessage;

      if(message_to_groove_box && message_to_groove_box->message_received == true)
      {
        for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
        {
          message_to_groove_box->mutes[i] = mutes[i];
          message_to_groove_box->solos[i] = solos[i];
          message_to_groove_box->track_volumes[i] = params[VOLUME_KNOBS + i].getValue();
        }

        // Tell GrooveBox that the message is ready for receiving
        message_to_groove_box->message_received = false;
      }
    }
  }

  void readFromGroovebox()
  {
    if (rightExpander.module && rightExpander.module->model == modelGrooveBox)
    {
      // Receive message from expander
      GrooveBoxMessage *groovebox_message = (GrooveBoxMessage *) this->rightExpander.producerMessage;

      // Retrieve the data from the expander
      if(groovebox_message && groovebox_message->message_received == true)
      {
        for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
        {
          if(groovebox_message->track_triggers[i])
          {
            // Trigger output
            triggerOutputPulseGenerators[i].trigger(0.01f);
          }
        }

        groovebox_message->message_received = false;
      }

      rightExpander.messageFlipRequested = true;
    }

  }
};
