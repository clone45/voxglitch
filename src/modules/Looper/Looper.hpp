// Refresh icon curtesy of "Trendy" from the Noun Project

struct Looper : VoxglitchSamplerModule
{
	std::string loaded_filename = "[ EMPTY ]";
  SamplePlayer sample_player;
  dsp::SchmittTrigger resetTrigger;
  float left_audio = 0;
  float right_audio = 0;
  std::string root_dir;
  
  // Voltage range settings
  unsigned int voltage_range_index = 3; // Default to -5.0 to 5.0
  
  std::string voltage_range_names[8] = {
    "0.0 to 10.0",
    "-10.0 to 10.0", 
    "0.0 to 5.0",
    "-5.0 to 5.0",
    "0.0 to 3.0",
    "-3.0 to 3.0",
    "0.0 to 1.0",
    "-1.0 to 1.0"
  };
  
  double voltage_ranges[8][2] = {
    {0.0, 10.0},
    {-10.0, 10.0},
    {0.0, 5.0},
    {-5.0, 5.0},
    {0.0, 3.0},
    {-3.0, 3.0},
    {0.0, 1.0},
    {-1.0, 1.0}
  };

  enum ParamIds {
    VOLUME_SLIDER,
		NUM_PARAMS
	};
	enum InputIds {
    RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
		NUM_OUTPUTS
	};

	Looper()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
    configParam(VOLUME_SLIDER, 0.0f, 1.0f, 1.0f, "VolumeSlider");
	}

	// Autosave module data.  VCV Rack decides when this should be called.
	json_t *dataToJson() override
	{
		json_t *root = json_object();
		json_object_set_new(root, "loaded_sample_path", json_string(sample_player.getPath().c_str()));
		json_object_set_new(root, "voltage_range_index", json_integer(voltage_range_index));

    // Call VoxglitchSamplerModule::saveSamplerData to save sampler data
    saveSamplerData(root);

		return root;
	}

	// Load module data
	void dataFromJson(json_t *root) override
	{
		json_t *loaded_sample_path = json_object_get(root, ("loaded_sample_path"));
		if (loaded_sample_path)
		{
			if(sample_player.loadSample(json_string_value(loaded_sample_path))) sample_player.trigger(0.0, true); // starting position=0.0, loop=true
			loaded_filename = sample_player.getFilename();
		}
		
		json_t *voltage_range_index_json = json_object_get(root, "voltage_range_index");
		if (voltage_range_index_json)
		{
			voltage_range_index = json_integer_value(voltage_range_index_json);
		}

    // Call VoxglitchSamplerModule::loadSamplerData to load sampler specific data
    loadSamplerData(root);
	}

	void process(const ProcessArgs &args) override
	{
    if(resetTrigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
    {
      sample_player.trigger(); // starting position=0.0, loop=true
    }

    sample_player.getStereoOutput(&left_audio, &right_audio, interpolation);

    // pitch = 0.0, sample_start = 0.0, sample_end = 1.0, loop = false
    sample_player.step(0.0, 0.0, 1.0, true);

    float volume = params[VOLUME_SLIDER].getValue();
    
    // Scale audio from -1 to +1 range to selected voltage range
    double min_voltage = voltage_ranges[voltage_range_index][0];
    double max_voltage = voltage_ranges[voltage_range_index][1];
    
    // Rescale from -1..+1 to min_voltage..max_voltage
    float left_scaled = rescale(left_audio * volume, -1.0f, 1.0f, min_voltage, max_voltage);
    float right_scaled = rescale(right_audio * volume, -1.0f, 1.0f, min_voltage, max_voltage);

    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_scaled);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_scaled);
  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override
  {
    sample_player.updateSampleRate();
  }
};
