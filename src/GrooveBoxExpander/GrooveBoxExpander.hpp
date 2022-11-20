struct GrooveBoxExpander : VoxglitchModule
{
  dsp::SchmittTrigger mute_cv_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger solo_button_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger solo_cv_triggers[NUMBER_OF_TRACKS];
  dsp::PulseGenerator triggerOutputPulseGenerators[NUMBER_OF_TRACKS];
  dsp::PulseGenerator triggerLightPulseGenerators[NUMBER_OF_TRACKS];

  GrooveboxToExpanderMessage groovebox_to_expander_message_a;
  GrooveboxToExpanderMessage groovebox_to_expander_message_b;

  bool mutes[NUMBER_OF_TRACKS];
  bool solos[NUMBER_OF_TRACKS];
  bool send_update_to_groovebox = false;
  bool track_triggers[NUMBER_OF_TRACKS];
  bool expander_connected = false;
  bool shift_key = false;

  enum ParamIds {
    ENUMS(MUTE_BUTTONS, NUMBER_OF_TRACKS),
    ENUMS(SOLO_BUTTONS, NUMBER_OF_TRACKS),
    ENUMS(VOLUME_KNOBS, NUMBER_OF_TRACKS),
    ENUMS(PAN_KNOBS, NUMBER_OF_TRACKS),
    ENUMS(PITCH_KNOBS, NUMBER_OF_TRACKS),
    NUM_PARAMS
  };
  enum InputIds {
    ENUMS(MUTE_INPUTS, NUMBER_OF_TRACKS),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(TRIGGER_OUTPUTS, NUMBER_OF_TRACKS),
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(MUTE_BUTTON_LIGHTS, NUMBER_OF_TRACKS),
    ENUMS(SOLO_BUTTON_LIGHTS, NUMBER_OF_TRACKS),
    ENUMS(GATE_OUTPUT_LIGHTS, NUMBER_OF_TRACKS),
    CONNECTED_LIGHT,
    NUM_LIGHTS
  };

  json_t *dataToJson() override
	{
    json_t *json_root = json_object();

    //
    // Save mutes
    //

    json_t *mutes_json_array = json_array();
    for(int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      json_array_append_new(mutes_json_array, json_integer(this->mutes[i]));
    }
    json_object_set(json_root, "mutes", mutes_json_array);
    json_decref(mutes_json_array);

    //
    // Save solos
    //

    json_t *solos_json_array = json_array();
    for(int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      json_array_append_new(solos_json_array, json_integer(this->solos[i]));
    }
    json_object_set(json_root, "solos", solos_json_array);
    json_decref(solos_json_array);

    return json_root;
  }

  void dataFromJson(json_t *json_root) override
	{
    // Load mutes
    json_t *mutes_json_object = json_object_get(json_root, "mutes");
    if(mutes_json_object)
    {
      size_t i;
      json_t *mutes_json = NULL;
      json_array_foreach(mutes_json_object, i, mutes_json)
      {
        this->mutes[i] = json_integer_value(mutes_json);
      }
    }

    // Load solos
    json_t *solos_json_object = json_object_get(json_root, "solos");
    if(solos_json_object)
    {
      size_t i;
      json_t *solos_json = NULL;
      json_array_foreach(solos_json_object, i, solos_json)
      {
        this->solos[i] = json_integer_value(solos_json);
      }
    }

    send_update_to_groovebox = true;
  }

  // Constructor
  GrooveBoxExpander()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      mutes[i] = false;
      solos[i] = false;
      // old_track_volumes[i] = 1.0;
      configParam(VOLUME_KNOBS + i, 0.0, 2.0, 1.0, "Volume");
      configParam(PAN_KNOBS + i, -1.0, 1.0, 0.0, "Pan");
      configParam(PITCH_KNOBS + i, -1.0, 1.0, 0.0, "Pitch");

      configOnOff(MUTE_BUTTONS + i, 0.0, "Mute Track " + std::to_string(i + 1));
      configOnOff(SOLO_BUTTONS + i, 0.0, "Solo Track "+ std::to_string(i + 1));
    }

    rightExpander.producerMessage = &groovebox_to_expander_message_a;
    rightExpander.consumerMessage = &groovebox_to_expander_message_b;
  }

  // Destructor
  ~GrooveBoxExpander()
  {

  }

	void process(const ProcessArgs &args) override
  {
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      // Read mute buttons and inputs
      bool mute_button_value = params[MUTE_BUTTONS + i].getValue();
      bool mute_button_triggered = rescale(inputs[MUTE_INPUTS + i].getVoltage(), 0.0f, 10.0f, 0.f, 1.f);
      mutes[i] = (mute_button_value || mute_button_triggered);

      // Read solo buttons
      solos[i] = params[SOLO_BUTTONS + i].getValue();
    }

    expander_connected = (rightExpander.module && rightExpander.module->model == modelGrooveBox);

    //
    // Send information to groovebox
    //
    if (expander_connected)
    {
      writeToGroovebox();
      readFromGroovebox();
    }

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      bool trigger_output_pulse = triggerOutputPulseGenerators[i].process(args.sampleTime);
      outputs[TRIGGER_OUTPUTS + i].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));

      bool light_output_pulse = triggerLightPulseGenerators[i].process(args.sampleTime);
      lights[GATE_OUTPUT_LIGHTS + i].setBrightness(light_output_pulse ? 1.0f : 0.0f);
    }

    lights[CONNECTED_LIGHT].setBrightness(expander_connected);
	}

  void exclusiveSolo(unsigned int track_index)
  {
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      bool value = (i == track_index);
      solos[i] = value;
      params[SOLO_BUTTONS + i].setValue(value);
    }
  }

  void unmuteAll()
  {
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      mutes[i] = false;
      params[MUTE_BUTTONS + i].setValue(false);
    }
  }

  void writeToGroovebox()
  {
    // Prepare message for sending to Grain Engine MK2
    // When writing to the groovebox, we're using the __GrooveBox's__ producer and consumer pair
    // Always write to the producer and read from the consumer
    ExpanderToGrooveboxMessage *message_to_groove_box = (ExpanderToGrooveboxMessage *) rightExpander.module->leftExpander.producerMessage;

    // Wait until the groovebox received the last message
    if(message_to_groove_box && message_to_groove_box->message_received == true)
    {
      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        message_to_groove_box->mutes[i] = mutes[i];
        message_to_groove_box->solos[i] = solos[i];
        message_to_groove_box->track_volumes[i] = params[VOLUME_KNOBS + i].getValue();
        message_to_groove_box->track_pans[i] = params[PAN_KNOBS + i].getValue();
        message_to_groove_box->track_pitches[i] = params[PITCH_KNOBS + i].getValue();
      }

      // Tell GrooveBox that the message is ready for receiving
      message_to_groove_box->message_received = false;
    }
  }

  void readFromGroovebox()
  {
    // Receive message from expander
    GrooveboxToExpanderMessage *groovebox_message = (GrooveboxToExpanderMessage *) this->rightExpander.consumerMessage;

    // Retrieve the data from the expander
    if(groovebox_message && groovebox_message->message_received == false)
    {
      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        if(groovebox_message->track_triggers[i])
        {
          // Trigger output
          triggerOutputPulseGenerators[i].trigger(0.01f);
          triggerLightPulseGenerators[i].trigger(0.05f);
        }
      }

      groovebox_message->message_received = true;
    }
    rightExpander.messageFlipRequested = true;
  }
};
