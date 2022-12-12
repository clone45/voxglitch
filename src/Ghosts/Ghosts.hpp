//
// Voxglitch Ghosts
//

struct Ghosts : VoxglitchSamplerModule
{
	float spawn_rate_counter = 0.0;
	double step_amount = 0.0;
	double smooth_rate = 128.0f / APP->engine->getSampleRate();
	double sample_rate_division = 0.0;
	float sr_div_8 = APP->engine->getSampleRate() / 8.0;
	float sample_rate = APP->engine->getSampleRate();
	float jitter_spread = 0.0;
	float minimum_ghosts_per_second = 0.0;
	float maximum_ghosts_per_second = 0.0;
	unsigned int removal_mode = 0;

	int step = 0;
	std::string root_dir;
	std::string path;

	GhostsEx graveyard;
	Sample sample;
	dsp::SchmittTrigger purge_trigger;
	dsp::SchmittTrigger purge_button_trigger;
	Random random;

	float maximum_playback_length = 0; // 1/2 of a second
	float minimum_playback_length = 0; // very fast

	unsigned int counter = 0;
	unsigned int mode = 0;

	// The filename of the loaded sample.  This is used to display the currently
	// loaded sample in the right-click context menu.
	std::string loaded_filename = "[ EMPTY ]";

	enum ParamIds
	{
		GHOST_PLAYBACK_LENGTH_KNOB,
		GHOST_PLAYBACK_LENGTH_ATTN_KNOB,
		GRAVEYARD_CAPACITY_KNOB,
		GRAVEYARD_CAPACITY_ATTN_KNOB,
		GHOST_SPAWN_RATE_KNOB,
		GHOST_SPAWN_RATE_ATTN_KNOB,
		SAMPLE_PLAYBACK_POSITION_KNOB,
		SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB, // unused, but keeping here to avoid shifting the enums
		PURGE_BUTTON_PARAM,
		TRIM_KNOB,
		JITTER_SWITCH,
		MODES_KNOB,
		NUM_PARAMS
	};
	enum InputIds
	{
		PURGE_TRIGGER_INPUT,
		JITTER_CV_INPUT,
		GHOST_PLAYBACK_LENGTH_INPUT,
		GRAVEYARD_CAPACITY_INPUT,
		GHOST_SPAWN_RATE_INPUT,
		SAMPLE_PLAYBACK_POSITION_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		AUDIO_OUTPUT_LEFT,
		AUDIO_OUTPUT_RIGHT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		PURGE_LIGHT,
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Ghosts()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(GHOST_PLAYBACK_LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "GhostLengthKnob");
		configParam(GHOST_PLAYBACK_LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GhostLengthAttnKnob");
		configParam(GRAVEYARD_CAPACITY_KNOB, 0.0f, 1.0f, 0.2f, "GraveyardCapacityKnob");
		configParam(GRAVEYARD_CAPACITY_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GraveyardCapacityAttnKnob");
		configParam(GHOST_SPAWN_RATE_KNOB, 0.0f, 10.0f, 5.0f, "GhostSpawnRateKnob"); // max 24000
		configParam(GHOST_SPAWN_RATE_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "GhostSpawnRateAttnKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "Playback Position");
		configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "Playback Input Attenuation");
		configParam(PITCH_KNOB, -2.0f, 2.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "PitchAttnKnob");
		configParam(PURGE_BUTTON_PARAM, 0.f, 1.f, 0.f, "PurgeButtonParam");
		configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
		configParam(JITTER_SWITCH, 0.f, 1.f, 1.f, "Jitter");
		configParam(MODES_KNOB, 0.f, 3.f, 0.f, "MODES");
		paramQuantities[MODES_KNOB]->snapEnabled = true;
		// jitter_divisor = static_cast <double> (RAND_MAX / 1024.0);

		maximum_playback_length = sample_rate / modes[mode][0]; // 1/2 of a second
		minimum_playback_length = sample_rate / modes[mode][1]; // very fast
		maximum_ghosts_per_second = modes[mode][2];		
		minimum_ghosts_per_second = modes[mode][3];
		jitter_spread = modes[mode][4];
		removal_mode = modes[mode][5];
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "path", json_string(sample.path.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *loaded_path_json = json_object_get(rootJ, ("path"));

		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			sample.load(path);
			loaded_filename = sample.filename;
		}
	}

	float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
	{
		float input_value = inputs[input_index].getVoltage() / 10.0;
		float knob_value = params[knob_index].getValue();
		float attenuator_value = params[attenuator_index].getValue();

		return (((input_value * scale) * attenuator_value) + (knob_value * scale));
	}

	void process(const ProcessArgs &args) override
	{
		mode = params[MODES_KNOB].getValue();

		// float maximum_ghosts_per_second = modes[mode][2];
		// float minimum_ghosts_per_second = modes[mode][3];
		// float maximum_playback_length = sample_rate / modes[mode][0]; // 1/2 of a second
		// float minimum_playback_length = sample_rate / modes[mode][1]; // very fast
		// float removal_mode = modes[mode][5];

		// Compute spawn rate.  The spawn rate input should be 0.0 to 10.0 volts.
		float spawn_rate = ((inputs[GHOST_SPAWN_RATE_INPUT].getVoltage() * params[GHOST_SPAWN_RATE_ATTN_KNOB].getValue())) + params[GHOST_SPAWN_RATE_KNOB].getValue();
		spawn_rate = clamp(spawn_rate, 0.0, 10.0); // ensure 0.0 to 10.0 volts
		spawn_rate = rescale(spawn_rate, 0.0, 10.0, minimum_ghosts_per_second, maximum_ghosts_per_second);

		float playback_length = ((inputs[GHOST_PLAYBACK_LENGTH_INPUT].getVoltage() * params[GHOST_PLAYBACK_LENGTH_ATTN_KNOB].getValue())) + params[GHOST_PLAYBACK_LENGTH_KNOB].getValue();
		playback_length = clamp(playback_length, 0.0, 10.0); // ensure 0.0 to 10.0 volts
		playback_length = rescale(playback_length, 0.0, 10.0, minimum_playback_length, maximum_playback_length);

		// calculate_inputs(GHOST_SPAWN_RATE_INPUT, GHOST_SPAWN_RATE_KNOB, GHOST_SPAWN_RATE_ATTN_KNOB, 4) + 1; // range from 1 to
		// float playback_length = calculate_inputs(GHOST_PLAYBACK_LENGTH_INPUT, GHOST_PLAYBACK_LENGTH_KNOB, GHOST_PLAYBACK_LENGTH_ATTN_KNOB, sr_div_8);
		float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, sample.size());

		if (start_position >= (sample.size() - playback_length))
			start_position = sample.size() - playback_length;

		// Shorten the playback length if it would result in playback passing the end of the sample data.
		if (playback_length > (sample.size() - start_position))
			playback_length = sample.size() - start_position;

		//
		// This next conditional is a little tricky, so let me break it down...
		//   If there's a cable in the Jitter CV Input, then apply jitter if the signal on that cable is greater than 0
		//   .. otherwise, use position of the jitter switch to determine if jitter should be applied.
		//
		// Additional Notes
		// * The jitter switch is ignored if a cable is connected to the jitter CV input.

		if (inputs[JITTER_CV_INPUT].isConnected() ? (inputs[JITTER_CV_INPUT].getVoltage() > 0) : params[JITTER_SWITCH].getValue())
		{
			// double r = (static_cast<double>(rand()) / (RAND_MAX / jitter_spread)) - (jitter_spread / 2.0);
			double r = rescale(random.gen(), 0.0, 1.0, 0.0, jitter_spread) - (jitter_spread / 2.0);
			start_position = start_position + r;
		}

		// Handle purge trigger input
		bool purge_is_triggered = purge_trigger.process(inputs[PURGE_TRIGGER_INPUT].getVoltage()) || purge_button_trigger.process(params[PURGE_BUTTON_PARAM].getValue());
		if (purge_is_triggered)
		{
			graveyard.markAllForRemoval();
		}
		lights[PURGE_LIGHT].setSmoothBrightness(purge_is_triggered, args.sampleTime);

		// Add more ghosts!
		if (spawn_rate_counter >= 1)
		{
			//
			// I ain't afraid of no ghosts! ♫ ♪
			//
			graveyard.add(start_position, playback_length, &sample);
			spawn_rate_counter = 0;
		}

		// Start killing off older ghosts.  This doesn't remove them immediately.
		// Instead, it marks them for removal.  Once marked for removal, the audio
		// smoothing process will be giving time to complete before the ghost has
		// been completely removed.

		int graveyard_capacity = calculate_inputs(GRAVEYARD_CAPACITY_INPUT, GRAVEYARD_CAPACITY_KNOB, GRAVEYARD_CAPACITY_ATTN_KNOB, MAX_GRAVEYARD_CAPACITY);

		if (graveyard.size() > graveyard_capacity)
		{
			if (removal_mode == 0)
			{
				// Mark the oldest 'nth' ghosts for removal
				graveyard.markOldestForRemoval(graveyard.size() - graveyard_capacity);
			}
			if (removal_mode == 1)
			{
				graveyard.markRandomForRemoval(graveyard.size() - graveyard_capacity);
			}
		}

		if (sample.loaded)
		{

			//
			// Iterate over all the ghosts in the graveyard and listen to what
			// they have to say...  What's that?  You want nachos?  Can you
			// even eat nachos?
			//

			if (graveyard.isEmpty() == false)
			{
				// pre-calculate step amount and smooth rate. This is to reduce the amount of math needed
				// within each Ghost's getStereoOutput() and age() functions.

				step_amount = sample_rate_division * rack::dsp::approxExp2_taylor5(inputs[PITCH_INPUT].getVoltage() + params[PITCH_KNOB].getValue());

				// Get the output from the graveyard and increase the age of each ghost
				float left_output, right_output = 0;
				graveyard.process(smooth_rate, step_amount, &left_output, &right_output);

				// Send audio to outputs
				outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_output * params[TRIM_KNOB].getValue());
				outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_output * params[TRIM_KNOB].getValue());
			}

			// TODO: spawn_rate_counter should probably take into consideration the selected sample rate.
			spawn_rate_counter = spawn_rate_counter + (spawn_rate / sample_rate);
		}
	}

	void onSampleRateChange(const SampleRateChangeEvent &e) override
	{
		this->sample_rate = APP->engine->getSampleRate();
		this->smooth_rate = 128.0f / sample_rate;
		this->sr_div_8 = sample_rate / 8.0;
		if (sample.loaded)
			sample_rate_division = sample.sample_rate / sample_rate;

		this->maximum_playback_length = sample_rate / modes[mode][0]; // 1/2 of a second
		this->minimum_playback_length = sample_rate / modes[mode][1]; // very fast			
	}
};
