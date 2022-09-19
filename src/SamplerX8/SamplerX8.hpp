struct SamplerX8 : VoxglitchSamplerModule
{
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};
  std::vector<SamplePlayer> sample_players;
  dsp::SchmittTrigger sample_triggers[NUMBER_OF_SAMPLES];

  StereoPan stereo_pan;
  // dsp::SchmittTrigger mute_buttons_schmitt_triggers[NUMBER_OF_SAMPLES];
  // bool mute_states[NUMBER_OF_SAMPLES];

  enum ParamIds
  {
    ENUMS(VOLUME_KNOBS, NUMBER_OF_SAMPLES),
    ENUMS(PAN_KNOBS, NUMBER_OF_SAMPLES),
    ENUMS(MUTE_BUTTONS, NUMBER_OF_SAMPLES),
		NUM_PARAMS
	};

	enum InputIds
  {
    ENUMS(TRIGGER_INPUTS, NUMBER_OF_SAMPLES),
    ENUMS(POSITION_INPUTS, NUMBER_OF_SAMPLES),
		NUM_INPUTS
	};

	enum OutputIds
  {
    ENUMS(AUDIO_LEFT_OUTPUTS, NUMBER_OF_SAMPLES),
    ENUMS(AUDIO_RIGHT_OUTPUTS, NUMBER_OF_SAMPLES),
    AUDIO_MIX_OUTPUT_LEFT,
    AUDIO_MIX_OUTPUT_RIGHT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};


	SamplerX8()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for(unsigned int i=0; i < NUMBER_OF_SAMPLES; i++)
    {
      configParam(VOLUME_KNOBS + i, 0.0, 1.0, 1.0, "volume");
      configParam(PAN_KNOBS + i, -1.0, 1.0, 0.0, "pan");
      configSwitch(MUTE_BUTTONS + i, false, true, true, "on/off");
      
      configOutput(AUDIO_LEFT_OUTPUTS + i, "left");
      configOutput(AUDIO_RIGHT_OUTPUTS + i, "right");
    }

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      SamplePlayer sample_player;
      sample_players.push_back(sample_player);
      // mute_states[i] = true;
    }
	}

  //
	// Save module data
  //
	json_t *dataToJson() override
	{
		json_t *root = json_object();

    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(root, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(sample_players[i].getPath().c_str()));
		}

    /*
    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(root, ("mute_states_" + std::to_string(i+1)).c_str(), json_integer((unsigned int) mute_states[i]));
		}
    */

    saveSamplerData(root);

		return root;
	}

  //
	// Load module data
  //
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
			if (loaded_mute_value) 
      {
        params[MUTE_BUTTONS + i].setValue(json_integer_value(loaded_mute_value));
      }
		}

    // Call VoxglitchSamplerModule::loadSamplerData to load sampler specific data
    loadSamplerData(root);
	}


	void process(const ProcessArgs &args) override
	{
    float summed_output_left = 0;
    float summed_output_right = 0;

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      // Process trigger inputs to start sample playback
      if (sample_triggers[i].process(rescale(inputs[TRIGGER_INPUTS + i].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
      {
        if(inputs[POSITION_INPUTS + i].isConnected())
        {
          sample_players[i].trigger(rescale(inputs[POSITION_INPUTS + i].getVoltage(), 0.0f, 10.0f, 0.f, 1.f));
        }
        else
        {
          sample_players[i].trigger();
        }
      }

      // Process mute button
      // bool mute_button_is_triggered = mute_buttons_schmitt_triggers[i].process(params[MUTE_BUTTONS + i].getValue());
      // if(mute_button_is_triggered) mute_states[i] ^= true;
      // mute_states[i] = params[MUTE_BUTTONS + i].getValue();

      //
      // Send audio to outputs
      float left_audio, right_audio;
      sample_players[i].getStereoOutput(&left_audio, &right_audio, interpolation);

      // Apply volume knobs
      left_audio = (left_audio * params[VOLUME_KNOBS + i].getValue());
      right_audio = (right_audio * params[VOLUME_KNOBS + i].getValue());

      // Apply panning knobs
      stereo_pan.process(&left_audio, &right_audio, params[PAN_KNOBS + i].getValue());

      // Output audio for the current sample
      if(params[MUTE_BUTTONS + i].getValue() == true)  // True means "play sample"
      {
        // Output individual outputs
        outputs[AUDIO_LEFT_OUTPUTS + i].setVoltage(left_audio);
        outputs[AUDIO_RIGHT_OUTPUTS + i].setVoltage(right_audio);

        // Sum up the output for the mix L/R output
        summed_output_left += left_audio;
        summed_output_right += right_audio;
      }

      // Step samples
      // step(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false)
      sample_players[i].step(0.0, inputs[POSITION_INPUTS + i].getVoltage(), 1.0, false);

    }

    // Output summed output
    outputs[AUDIO_MIX_OUTPUT_LEFT].setVoltage(summed_output_left);
    outputs[AUDIO_MIX_OUTPUT_RIGHT].setVoltage(summed_output_right);

  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override
  {
    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      sample_players[i].updateSampleRate();
    }
  }
};
