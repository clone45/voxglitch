struct Goblins : Module
{
	unsigned int selected_sample_slot = 0;
	float spawn_rate_counter = 0;
	float step_amount = 0;
	int step = 0;
	std::string root_dir;
	std::string path;

	std::vector<Goblin> countryside;
	Sample samples[NUMBER_OF_SAMPLES];
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

	dsp::SchmittTrigger purge_trigger;
	dsp::SchmittTrigger purge_button_trigger;

	enum ParamIds {
		SAMPLE_PLAYBACK_POSITION_KNOB,
		SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
		PLAYBACK_LENGTH_KNOB,
		PLAYBACK_LENGTH_ATTN_KNOB,
		SPAWN_RATE_KNOB,
		SPAWN_RATE_ATTN_KNOB,
		COUNTRYSIDE_CAPACITY_KNOB,
		COUNTRYSIDE_CAPACITY_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		SAMPLE_SELECT_KNOB,
		SAMPLE_SELECT_ATTN_KNOB,
		PURGE_BUTTON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PURGE_TRIGGER_INPUT,
		PITCH_INPUT,
		PLAYBACK_LENGTH_INPUT,
		COUNTRYSIDE_CAPACITY_INPUT,
		SPAWN_RATE_INPUT,
		SAMPLE_PLAYBACK_POSITION_INPUT,
		SAMPLE_SELECT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT_LEFT,
		AUDIO_OUTPUT_RIGHT,
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PURGE_LIGHT,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Goblins()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
		configParam(PLAYBACK_LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "LengthKnob");
		configParam(PLAYBACK_LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "LengthAttnKnob");
		configParam(SPAWN_RATE_KNOB, 0.00f, 1.0f, 0.2f, "SpawnRateKnob");
		configParam(SPAWN_RATE_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SpawnRateAttnKnob");
		configParam(COUNTRYSIDE_CAPACITY_KNOB, 0.0f, 1.0f, 1.0f, "CountrysideCapacityKnob");
		configParam(COUNTRYSIDE_CAPACITY_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "CountrysideCapacityAttnKnob");
		configParam(PITCH_KNOB, -1.0f, 1.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "PitchAttnKnob");
		configParam(SAMPLE_SELECT_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(SAMPLE_SELECT_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(PURGE_BUTTON_PARAM, 0.f, 1.f, 0.f, "PurgeButtonParam");

		std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(rootJ, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				samples[i].load(json_string_value(loaded_sample_path), false);
				loaded_filenames[i] = samples[i].filename;
			}
		}
	}

	float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
	{
		float input_value = inputs[input_index].getVoltage() / 10.0;
		float knob_value = params[knob_index].getValue();
		float attenuator_value = params[attenuator_index].getValue();

		return(((input_value * scale) * attenuator_value) + (knob_value * scale));
	}

	void process(const ProcessArgs &args) override
	{
		//
		//  Set selected sample based on inputs.
		//  This must happen before we calculate start_position

		selected_sample_slot = (unsigned int) calculate_inputs(SAMPLE_SELECT_INPUT, SAMPLE_SELECT_KNOB, SAMPLE_SELECT_ATTN_KNOB, NUMBER_OF_SAMPLES_FLOAT);
		selected_sample_slot = clamp(selected_sample_slot, 0, NUMBER_OF_SAMPLES - 1);
		Sample *selected_sample = &samples[selected_sample_slot];

		//
		//  Calculate additional inputs

		float spawn_rate = calculate_inputs(SPAWN_RATE_INPUT, SPAWN_RATE_KNOB, SPAWN_RATE_ATTN_KNOB, MAX_SPAWN_RATE);
		float playback_length = calculate_inputs(PLAYBACK_LENGTH_INPUT, PLAYBACK_LENGTH_KNOB, PLAYBACK_LENGTH_ATTN_KNOB, (args.sampleRate / 8));
		float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, selected_sample->total_sample_count);

		// Ensure that the inputs are within range
		spawn_rate = clamp(spawn_rate, 0.0f, MAX_SPAWN_RATE);

		// If the purge input it trittered, empty the goblins from the country side
		bool purge_is_triggered = purge_trigger.process(inputs[PURGE_TRIGGER_INPUT].getVoltage()) || purge_button_trigger.process(params[PURGE_BUTTON_PARAM].getValue());
		if (purge_is_triggered) countryside.clear();
		lights[PURGE_LIGHT].setSmoothBrightness(purge_is_triggered, args.sampleTime);

		// Frolic Goblins!
		// Ravish the countryside!
		// Beer! Bread! Lamb!
		if((spawn_rate_counter >= spawn_rate) && (selected_sample->loaded))
		{
			Goblin goblin;
			goblin.start_position = start_position;
			goblin.sample_ptr = selected_sample;
			countryside.push_back(goblin);

			spawn_rate_counter = 0;
		}

		unsigned int countryside_capacity = calculate_inputs(COUNTRYSIDE_CAPACITY_INPUT, COUNTRYSIDE_CAPACITY_KNOB, COUNTRYSIDE_CAPACITY_ATTN_KNOB, MAX_NUMBER_OF_GOBLINS);
		countryside_capacity = clamp(countryside_capacity, 0, MAX_NUMBER_OF_GOBLINS);

		// If there are too many goblins, kill off the oldest until the population is under control.
		while(countryside.size() > countryside_capacity)
		{
            countryside.erase(countryside.begin());
		}

		if ((! selected_sample->loading) && (selected_sample->total_sample_count > 0))
		{
			float left_mix_output = 0;
			float right_mix_output = 0;

			if(countryside.empty() == false)
			{
				// TODO: selected_sample doesn't really have a meaning in this
				// part of the code.  The step_amount calculation will need to
				// be moved into the Goblin's age() code.

				// Increment sample offset (pitch)
				if (inputs[PITCH_INPUT].isConnected())
				{
					step_amount = (selected_sample->sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
				}
				else
				{
					step_amount = (selected_sample->sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
				}

				// Iterate over all the goblins in the countryside and collect their output.
				for(Goblin& goblin : countryside)
				{
					// mix_output += goblin.getOutput();
					std::pair<float, float> stereo_output = goblin.getStereoOutput();
					left_mix_output  += stereo_output.first;
					right_mix_output += stereo_output.second;

					goblin.age(step_amount, playback_length);
				}

				outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_mix_output);
				outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_mix_output);
			}

			spawn_rate_counter = spawn_rate_counter + 1.0f;
		}
		else
		{
			outputs[AUDIO_OUTPUT_LEFT].setVoltage(0);
			outputs[AUDIO_OUTPUT_RIGHT].setVoltage(0);
		}
	}
};
