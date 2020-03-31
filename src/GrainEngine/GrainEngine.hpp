struct GrainEngine : Module
{
	float spawn_rate_counter = 0;
	float step_amount = 0;
	float smooth_rate = 0;

	int step = 0;
	std::string root_dir;
	std::string path;

	GrainEngineEx grain_engine_core;
	Sample sample;
	dsp::SchmittTrigger purge_trigger;
	dsp::SchmittTrigger purge_button_trigger;
    dsp::SchmittTrigger spawn_trigger;

	float jitter_divisor = 1;

	// The filename of the loaded sample.  This is used to display the currently
	// loaded sample in the right-click context menu.
	std::string loaded_filename = "[ EMPTY ]";

	enum ParamIds {
		LENGTH_KNOB,
		LENGTH_ATTN_KNOB,
		SAMPLE_PLAYBACK_POSITION_KNOB,
		SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		TRIM_KNOB,
        AMP_SLOPE_KNOB,
        AMP_SLOPE_ATTN_KNOB,
		JITTER_KNOB,
        LEN_MULT_KNOB,
        PAN_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		JITTER_CV_INPUT,
		LENGTH_INPUT,
		SAMPLE_PLAYBACK_POSITION_INPUT,
		PITCH_INPUT,
        SPAWN_TRIGGER_INPUT,
        AMP_SLOPE_INPUT,
        PAN_INPUT,
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
	GrainEngine()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "GhostLengthKnob");
        configParam(LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GhostLengthAttnKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
		configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
		configParam(PITCH_KNOB, -0.3f, 1.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "PitchAttnKnob");
		configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
        configParam(LEN_MULT_KNOB, 1.0f, 128.0f, 1.0f, "LenMultKnob");
        configParam(AMP_SLOPE_KNOB, 0.0f, 1.0f, 0.0f, "AmpSlopeKnob");
        configParam(AMP_SLOPE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "AmpSlopeAttnKnob");
		configParam(JITTER_KNOB, 0.f, 1.0f, 0.0f, "JitterKnob");
        configParam(PAN_SWITCH, 0.0f, 1.0f, 0.0f, "PanSwitch");

		jitter_divisor = static_cast <float> (RAND_MAX / 1024.0);
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

		if(loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			sample.load(path, false);
			loaded_filename = sample.filename;
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
        float length_multiplier = params[LEN_MULT_KNOB].getValue();
		float playback_length = calculate_inputs(LENGTH_INPUT, LENGTH_KNOB, LENGTH_ATTN_KNOB, 128) * length_multiplier;
		float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, sample.total_sample_count);
        int amp_slope_selection = calculate_inputs(AMP_SLOPE_INPUT, AMP_SLOPE_KNOB, AMP_SLOPE_ATTN_KNOB, 9.0);

		// Ensure that the inputs are within range
		if(start_position >= (sample.total_sample_count - playback_length)) start_position = sample.total_sample_count - playback_length;

		// Shorten the playback length if it would result in playback passing the end of the sample data.
		if(playback_length > (sample.total_sample_count - start_position)) playback_length = sample.total_sample_count - start_position;

        //
        // Process Jitter input
        //

		if(inputs[JITTER_CV_INPUT].isConnected())
		{
            float spread = params[JITTER_KNOB].getValue() * 64.0 * inputs[JITTER_CV_INPUT].getVoltage();
            float r = (static_cast <float> (rand()) / (RAND_MAX / spread)) - spread;
            start_position = start_position + r;
		}
        else
        {
            float spread = params[JITTER_KNOB].getValue() * 64.0;
            float r = (static_cast <float> (rand()) / (RAND_MAX / spread)) - spread;
            start_position = start_position + r;
        }

        //
        // Process Pan input
        //

        float pan = 0;
        if(inputs[PAN_INPUT].isConnected())
        {
            if(params[PAN_SWITCH].getValue() == 1) // unipolar
            {
                // Incoming pan signal is unipolar.  Convert it to bipolar.
                pan = (inputs[PAN_INPUT].getVoltage() / 5.0) - 1;
            }
            else // bipolar
            {
                // Incoming pan signal is bipolar.  No conversion necessary.
                pan = (inputs[PAN_INPUT].getVoltage() / 10.0);
            }
        }

        if(spawn_trigger.process(inputs[SPAWN_TRIGGER_INPUT].getVoltage()))
        {
            grain_engine_core.add(start_position, playback_length, pan, &sample);
        }

		if (sample.loaded && grain_engine_core.isEmpty() == false)
		{
			// pre-calculate step amount and smooth rate. This is to reduce the amount of math needed
			// within each Ghost's getStereoOutput() and age() functions.

			if(inputs[PITCH_INPUT].isConnected())
			{
				step_amount = (sample.sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
			}
			else
			{
				step_amount = (sample.sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
			}

			smooth_rate = 128.0f / args.sampleRate;

			// Get the output from the graveyard and increase the age of each ghost
			std::pair<float, float> stereo_output = grain_engine_core.process(smooth_rate, step_amount, amp_slope_selection);
			float left_mix_output = stereo_output.first * params[TRIM_KNOB].getValue();
			float right_mix_output = stereo_output.second  * params[TRIM_KNOB].getValue();

			// Send audio to outputs
			outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_mix_output);
			outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_mix_output);
		}
	}
};
