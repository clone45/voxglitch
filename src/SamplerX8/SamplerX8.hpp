struct SamplerX8 : Module
{
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};
  std::vector<SamplePlayer> sample_players;
  dsp::SchmittTrigger sample_triggers[NUMBER_OF_SAMPLES];
  float left_audio = 0;
  float right_audio = 0;

  enum ParamIds {
    // Enums for volumn knobs
    VOLUME_KNOBS,
    VOLUME_KNOB_1,
    VOLUME_KNOB_2,
    VOLUME_KNOB_3,
    VOLUME_KNOB_4,
    VOLUME_KNOB_5,
    VOLUME_KNOB_6,
    VOLUME_KNOB_7,
    VOLUME_KNOB_8,
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

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      SamplePlayer sample_player;
      sample_players.push_back(sample_player);
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
				sample_players[i].loadSample(json_string_value(loaded_sample_path));
				loaded_filenames[i] = sample_players[i].getFilename();
			}
		}
	}

	void process(const ProcessArgs &args) override
	{
    float summed_output_left = 0;
    float summed_output_right = 0;

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      // Process trigger inputs to start sample playback
      if (sample_triggers[i].process(rescale(inputs[i].getVoltage(), 0.0f, 10.0f, 0.f, 1.f))) sample_players[i].trigger();

      // Send audio to outputs
      std::tie(left_audio, right_audio) = sample_players[i].getStereoOutput();
      outputs[i].setVoltage(left_audio);
      outputs[i + NUMBER_OF_SAMPLES].setVoltage(right_audio);

      summed_output_left += (left_audio * params[VOLUME_KNOBS + i + 1].getValue());
      summed_output_right += (right_audio * params[VOLUME_KNOBS + i + 1].getValue());

      // Step samples
      sample_players[i].step(args.sampleRate);
    }

    // Output summed output
    outputs[AUDIO_MIX_OUTPUT_LEFT].setVoltage(summed_output_left);
    outputs[AUDIO_MIX_OUTPUT_RIGHT].setVoltage(summed_output_right);
  }
};
