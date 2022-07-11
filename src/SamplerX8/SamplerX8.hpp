struct SamplerX8 : VoxglitchSamplerModule
{
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};
  std::vector<SamplePlayer> sample_players;
  dsp::SchmittTrigger sample_triggers[NUMBER_OF_SAMPLES];
  float left_audio = 0;
  float right_audio = 0;
  StereoPanSubModule stereo_pan_submodule;
  dsp::SchmittTrigger mute_buttons_schmitt_triggers[NUMBER_OF_SAMPLES];
  unsigned int mute_buttons[NUMBER_OF_SAMPLES];
  unsigned int mute_lights[NUMBER_OF_SAMPLES];
  bool mute_states[NUMBER_OF_SAMPLES];

  enum ParamIds {
    // Enums for volume knobs
    VOLUME_KNOBS,
    VOLUME_KNOB_1,
    VOLUME_KNOB_2,
    VOLUME_KNOB_3,
    VOLUME_KNOB_4,
    VOLUME_KNOB_5,
    VOLUME_KNOB_6,
    VOLUME_KNOB_7,
    VOLUME_KNOB_8,
    PAN_KNOBS,
    PAN_KNOB_1,
    PAN_KNOB_2,
    PAN_KNOB_3,
    PAN_KNOB_4,
    PAN_KNOB_5,
    PAN_KNOB_6,
    PAN_KNOB_7,
    PAN_KNOB_8,
    MUTE_BUTTON_1,
    MUTE_BUTTON_2,
    MUTE_BUTTON_3,
    MUTE_BUTTON_4,
    MUTE_BUTTON_5,
    MUTE_BUTTON_6,
    MUTE_BUTTON_7,
    MUTE_BUTTON_8,
		NUM_PARAMS
	};
	enum InputIds {

    // I'll be referencing these enums by number later in the code, which is
    // why I'm explicitely assigning them values instead of allowing the
    // compiler to.  If I didn't, their values could shift if additional
    // enums are added before them in this declaration.

    TRIGGER_INPUT_1 = 0,
    TRIGGER_INPUT_2 = 1,
    TRIGGER_INPUT_3 = 2,
    TRIGGER_INPUT_4 = 3,
    TRIGGER_INPUT_5 = 4,
    TRIGGER_INPUT_6 = 5,
    TRIGGER_INPUT_7 = 6,
    TRIGGER_INPUT_8 = 7,

    POSITION_INPUT_1 = 8,
    POSITION_INPUT_2 = 9,
    POSITION_INPUT_3 = 10,
    POSITION_INPUT_4 = 11,
    POSITION_INPUT_5 = 12,
    POSITION_INPUT_6 = 13,
    POSITION_INPUT_7 = 14,
    POSITION_INPUT_8 = 15,

		NUM_INPUTS
	};
	enum OutputIds {

    // Output enums are organized so I can easily reference them in a for loop:
    //
    // for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++) {
    //   outputs[i]   // left output
    //   outputs[i+NUMBER_OF_SAMPLES] // right output

		AUDIO_OUTPUT_1_LEFT = 0,
    AUDIO_OUTPUT_2_LEFT = 1,
    AUDIO_OUTPUT_3_LEFT = 2,
    AUDIO_OUTPUT_4_LEFT = 3,
    AUDIO_OUTPUT_5_LEFT = 4,
    AUDIO_OUTPUT_6_LEFT = 5,
    AUDIO_OUTPUT_7_LEFT = 6,
    AUDIO_OUTPUT_8_LEFT = 7,

		AUDIO_OUTPUT_1_RIGHT = 8,
		AUDIO_OUTPUT_2_RIGHT = 9,
		AUDIO_OUTPUT_3_RIGHT = 10,
		AUDIO_OUTPUT_4_RIGHT = 11,
		AUDIO_OUTPUT_5_RIGHT = 12,
		AUDIO_OUTPUT_6_RIGHT = 13,
		AUDIO_OUTPUT_7_RIGHT = 14,
		AUDIO_OUTPUT_8_RIGHT = 15,

    AUDIO_MIX_OUTPUT_LEFT,
    AUDIO_MIX_OUTPUT_RIGHT,
		NUM_OUTPUTS
	};
	enum LightIds {
    MUTE_BUTTON_1_LIGHT,
    MUTE_BUTTON_2_LIGHT,
    MUTE_BUTTON_3_LIGHT,
    MUTE_BUTTON_4_LIGHT,
    MUTE_BUTTON_5_LIGHT,
    MUTE_BUTTON_6_LIGHT,
    MUTE_BUTTON_7_LIGHT,
    MUTE_BUTTON_8_LIGHT,
		NUM_LIGHTS
	};



	SamplerX8()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(VOLUME_KNOB_1, 0.0, 1.0, 1.0, "VolumeKnob1");
    configParam(VOLUME_KNOB_2, 0.0, 1.0, 1.0, "VolumeKnob2");
    configParam(VOLUME_KNOB_3, 0.0, 1.0, 1.0, "VolumeKnob3");
    configParam(VOLUME_KNOB_4, 0.0, 1.0, 1.0, "VolumeKnob4");
    configParam(VOLUME_KNOB_5, 0.0, 1.0, 1.0, "VolumeKnob5");
    configParam(VOLUME_KNOB_6, 0.0, 1.0, 1.0, "VolumeKnob6");
    configParam(VOLUME_KNOB_7, 0.0, 1.0, 1.0, "VolumeKnob7");
    configParam(VOLUME_KNOB_8, 0.0, 1.0, 1.0, "VolumeKnob8");

    configParam(PAN_KNOB_1, -1.0, 1.0, 0.0, "PanKnob1");
    configParam(PAN_KNOB_2, -1.0, 1.0, 0.0, "PanKnob2");
    configParam(PAN_KNOB_3, -1.0, 1.0, 0.0, "PanKnob3");
    configParam(PAN_KNOB_4, -1.0, 1.0, 0.0, "PanKnob4");
    configParam(PAN_KNOB_5, -1.0, 1.0, 0.0, "PanKnob5");
    configParam(PAN_KNOB_6, -1.0, 1.0, 0.0, "PanKnob6");
    configParam(PAN_KNOB_7, -1.0, 1.0, 0.0, "PanKnob7");
    configParam(PAN_KNOB_8, -1.0, 1.0, 0.0, "PanKnob8");

    configParam(MUTE_BUTTON_1, 0.f, 1.f, 1.f, "MuteButton1");
    configParam(MUTE_BUTTON_2, 0.f, 1.f, 1.f, "MuteButton2");
    configParam(MUTE_BUTTON_3, 0.f, 1.f, 1.f, "MuteButton3");
    configParam(MUTE_BUTTON_4, 0.f, 1.f, 1.f, "MuteButton4");
    configParam(MUTE_BUTTON_5, 0.f, 1.f, 1.f, "MuteButton5");
    configParam(MUTE_BUTTON_6, 0.f, 1.f, 1.f, "MuteButton6");
    configParam(MUTE_BUTTON_7, 0.f, 1.f, 1.f, "MuteButton7");
    configParam(MUTE_BUTTON_8, 0.f, 1.f, 1.f, "MuteButton8");

    mute_buttons[0] = MUTE_BUTTON_1;
    mute_buttons[1] = MUTE_BUTTON_2;
    mute_buttons[2] = MUTE_BUTTON_3;
    mute_buttons[3] = MUTE_BUTTON_4;
    mute_buttons[4] = MUTE_BUTTON_5;
    mute_buttons[5] = MUTE_BUTTON_6;
    mute_buttons[6] = MUTE_BUTTON_7;
    mute_buttons[7] = MUTE_BUTTON_8;

    mute_lights[0] = MUTE_BUTTON_1_LIGHT;
    mute_lights[1] = MUTE_BUTTON_2_LIGHT;
    mute_lights[2] = MUTE_BUTTON_3_LIGHT;
    mute_lights[3] = MUTE_BUTTON_4_LIGHT;
    mute_lights[4] = MUTE_BUTTON_5_LIGHT;
    mute_lights[5] = MUTE_BUTTON_6_LIGHT;
    mute_lights[6] = MUTE_BUTTON_7_LIGHT;
    mute_lights[7] = MUTE_BUTTON_8_LIGHT;

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      SamplePlayer sample_player;
      sample_players.push_back(sample_player);
      mute_states[i] = true;
    }
	}

	// Autosave module data.  VCV Rack decides when this should be called.
	json_t *dataToJson() override
	{
		json_t *root = json_object();

    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(root, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(sample_players[i].getPath().c_str()));
		}

    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(root, ("mute_states_" + std::to_string(i+1)).c_str(), json_integer((unsigned int) mute_states[i]));
		}

		return root;
	}

	// Load module data
	void dataFromJson(json_t *root) override
	{
    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(root, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				if(sample_players[i].loadSample(json_string_value(loaded_sample_path)))
        {
          loaded_filenames[i] = sample_players[i].getFilename();
        }
			}
		}

    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_mute_value = json_object_get(root, ("mute_states_" +  std::to_string(i+1)).c_str());
			if (loaded_mute_value) mute_states[i] = json_integer_value(loaded_mute_value);
		}
	}


	void process(const ProcessArgs &args) override
	{
    float summed_output_left = 0;
    float summed_output_right = 0;


    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      // Process trigger inputs to start sample playback
      if (sample_triggers[i].process(rescale(inputs[i].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
      {
        unsigned int position_input_index = i + 8;
        sample_players[i].trigger();
        if(inputs[position_input_index].isConnected()) sample_players[i].setPositionFromInput(inputs[position_input_index].getVoltage());
      }

      // Process mute button
      bool mute_button_is_triggered = mute_buttons_schmitt_triggers[i].process(params[mute_buttons[i]].getValue());
      if(mute_button_is_triggered) mute_states[i] ^= true;

      lights[mute_lights[i]].setBrightness(mute_states[i]);

      // Send audio to outputs
      std::tie(left_audio, right_audio) = sample_players[i].getStereoOutput();


      // Apply volume knobs
      left_audio = (left_audio * params[VOLUME_KNOBS + i + 1].getValue());
      right_audio = (right_audio * params[VOLUME_KNOBS + i + 1].getValue());

      // Apply panning knobs
      std::tie(left_audio, right_audio) = stereo_pan_submodule.process(left_audio, right_audio, params[PAN_KNOBS + i + 1].getValue());

      // Output audio for the current sample
      if(mute_states[i] == true)  // True means "play sample"
      {
        outputs[i].setVoltage(left_audio);
        outputs[i + NUMBER_OF_SAMPLES].setVoltage(right_audio);

        // Sum up the output for the mix L/R output
        summed_output_left += left_audio;
        summed_output_right += right_audio;
      }

      // Step samples
      sample_players[i].step(args.sampleRate);

    }

    // Output summed output
    outputs[AUDIO_MIX_OUTPUT_LEFT].setVoltage(summed_output_left);
    outputs[AUDIO_MIX_OUTPUT_RIGHT].setVoltage(summed_output_right);

  }
};
