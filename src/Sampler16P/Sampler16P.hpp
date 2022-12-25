struct Sampler16P : VoxglitchSamplerModule
{
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};
  std::vector<SamplePlayer> sample_players;
  dsp::SchmittTrigger sample_triggers[NUMBER_OF_SAMPLES];

  StereoPan stereo_pan;

  enum ParamIds
  {
		NUM_PARAMS
	};

	enum InputIds
  {
    TRIGGER_INPUTS,
		NUM_INPUTS
	};

	enum OutputIds
  {
    AUDIO_MIX_OUTPUT_LEFT,
    AUDIO_MIX_OUTPUT_RIGHT,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};


	Sampler16P()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      SamplePlayer sample_player;
      sample_players.push_back(sample_player);
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

    // Call VoxglitchSamplerModule::loadSamplerData to load sampler specific data
    loadSamplerData(root);
	}


	void process(const ProcessArgs &args) override
	{
    float summed_output_left = 0;
    float summed_output_right = 0;

    inputs[TRIGGER_INPUTS].setChannels(NUMBER_OF_SAMPLES);

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      // Process trigger inputs to start sample playback
      if (sample_triggers[i].process(rescale(inputs[TRIGGER_INPUTS].getVoltage(i), 0.0f, 10.0f, 0.f, 1.f)))
      {
        sample_players[i].trigger();
      }

      //
      // Send audio to outputs
      float left_audio, right_audio;
      sample_players[i].getStereoOutput(&left_audio, &right_audio, interpolation);

      // Sum up the output for the mix L/R output
      summed_output_left += left_audio;
      summed_output_right += right_audio;

      // Step samples
      sample_players[i].step();
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
