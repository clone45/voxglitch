struct Scatter : Module
{
  std::string root_dir;
	std::string path;
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

  std::vector<SamplePlayer> sample_players;
  dsp::SchmittTrigger sample_triggers[NUMBER_OF_SAMPLES];
  int sample_trigger_inputs[NUMBER_OF_SAMPLES];
  int sample_outputs[NUMBER_OF_SAMPLES][2];

  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    TRIGGER_INPUT_1,
    TRIGGER_INPUT_2,
    TRIGGER_INPUT_3,
    TRIGGER_INPUT_4,
    TRIGGER_INPUT_5,
    TRIGGER_INPUT_6,
    TRIGGER_INPUT_7,
    TRIGGER_INPUT_8,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT_1_LEFT,
		AUDIO_OUTPUT_1_RIGHT,
    AUDIO_OUTPUT_2_LEFT,
		AUDIO_OUTPUT_2_RIGHT,
    AUDIO_OUTPUT_3_LEFT,
		AUDIO_OUTPUT_3_RIGHT,
    AUDIO_OUTPUT_4_LEFT,
		AUDIO_OUTPUT_4_RIGHT,
    AUDIO_OUTPUT_5_LEFT,
		AUDIO_OUTPUT_5_RIGHT,
    AUDIO_OUTPUT_6_LEFT,
		AUDIO_OUTPUT_6_RIGHT,
    AUDIO_OUTPUT_7_LEFT,
		AUDIO_OUTPUT_7_RIGHT,
    AUDIO_OUTPUT_8_LEFT,
		AUDIO_OUTPUT_8_RIGHT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Scatter()
	{
    sample_trigger_inputs[0] = TRIGGER_INPUT_1;
    sample_trigger_inputs[1] = TRIGGER_INPUT_2;
    sample_trigger_inputs[2] = TRIGGER_INPUT_3;
    sample_trigger_inputs[3] = TRIGGER_INPUT_4;
    sample_trigger_inputs[4] = TRIGGER_INPUT_5;
    sample_trigger_inputs[5] = TRIGGER_INPUT_6;
    sample_trigger_inputs[6] = TRIGGER_INPUT_7;
    sample_trigger_inputs[7] = TRIGGER_INPUT_8;

    sample_outputs[0][0] = AUDIO_OUTPUT_1_LEFT;
    sample_outputs[0][1] = AUDIO_OUTPUT_1_RIGHT;
    sample_outputs[1][0] = AUDIO_OUTPUT_2_LEFT;
    sample_outputs[1][1] = AUDIO_OUTPUT_2_RIGHT;
    sample_outputs[2][0] = AUDIO_OUTPUT_3_LEFT;
    sample_outputs[2][1] = AUDIO_OUTPUT_3_RIGHT;
    sample_outputs[3][0] = AUDIO_OUTPUT_4_LEFT;
    sample_outputs[3][1] = AUDIO_OUTPUT_4_RIGHT;
    sample_outputs[4][0] = AUDIO_OUTPUT_5_LEFT;
    sample_outputs[4][1] = AUDIO_OUTPUT_5_RIGHT;
    sample_outputs[5][0] = AUDIO_OUTPUT_6_LEFT;
    sample_outputs[5][1] = AUDIO_OUTPUT_6_RIGHT;
    sample_outputs[6][0] = AUDIO_OUTPUT_7_LEFT;
    sample_outputs[6][1] = AUDIO_OUTPUT_7_RIGHT;
    sample_outputs[7][0] = AUDIO_OUTPUT_8_LEFT;
    sample_outputs[7][1] = AUDIO_OUTPUT_8_RIGHT;

    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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

		return root;
	}

	// Load module data
	void dataFromJson(json_t *root) override
	{

	}

	void process(const ProcessArgs &args) override
	{
    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {

      // Process trigger inputs to start sample playback
      if (sample_triggers[i].process(rescale(inputs[sample_trigger_inputs[i]].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
      {
        sample_players[i].trigger();
      }

      // Send audio to outputs
      float left;
      float right;
      std::tie(left,right) = sample_players[i].getStereoOutput();

      outputs[sample_outputs[i][0]].setVoltage(left);
      outputs[sample_outputs[i][1]].setVoltage(right);

      // Step samples
      sample_players[i].step(args.sampleRate);
    }
  }
};
