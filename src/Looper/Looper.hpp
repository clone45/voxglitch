// Refresh icon curtesy of "Trendy" from the Noun Project

struct Looper : Module
{
	std::string loaded_filename = "[ EMPTY ]";
  SamplePlayer sample_player;
  dsp::SchmittTrigger resetTrigger;
  float left_audio = 0;
  float right_audio = 0;
  std::string root_dir;

  enum ParamIds {
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
	}

	// Autosave module data.  VCV Rack decides when this should be called.
	json_t *dataToJson() override
	{
		json_t *root = json_object();
		json_object_set_new(root, "loaded_sample_path", json_string(sample_player.getPath().c_str()));
		return root;
	}

	// Load module data
	void dataFromJson(json_t *root) override
	{
		json_t *loaded_sample_path = json_object_get(root, ("loaded_sample_path"));
		if (loaded_sample_path)
		{
			sample_player.loadSample(json_string_value(loaded_sample_path));
			loaded_filename = sample_player.getFilename();
      sample_player.trigger();
		}
	}

	void process(const ProcessArgs &args) override
	{
    if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      sample_player.reset();
    }

    std::tie(left_audio, right_audio) = sample_player.getStereoOutput();
    sample_player.step(args.sampleRate);

    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_audio);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_audio);
  }
};
