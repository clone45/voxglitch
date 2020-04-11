struct Scatter : Module
{
  std::string root_dir;
	std::string path;

  Sample samples[NUMBER_OF_SAMPLES];
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

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
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");
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
    // Send audio to outputs
    outputs[AUDIO_OUTPUT_1_LEFT].setVoltage(0);
    outputs[AUDIO_OUTPUT_1_RIGHT].setVoltage(0);
  }
};
