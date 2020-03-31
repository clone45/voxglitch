struct Autobreak : Module
{
	unsigned int selected_sample_slot = 0;

	// Actual index into the sample's array for playback
	float actual_playback_position = 0;

	// A location in a theoretical sample that's 8 bars at the selected BPM.
	// This value is stepped and repositioned when jumping around in a breakbeat.
	// This value is then used to figure out the actual_playback_position based
	// on the sample's length.

	float theoretical_playback_position = 0;

	// incrementing_bpm_counter counts from 0 to the number of samples required
	// to play a theoretical sample for 8 bars at a specific bpm.
	unsigned int incrementing_bpm_counter = 0;

	// Pattern lock is a flag which, when true, stops the pattern from changing
	// due to changes in the pattern knob or the CV input.  This flag is set
	// to true when the user is actively editing the current pattern.

    double time_counter = 0.0;
	double bpm = 160;
    double timer_before = 0;
    bool clock_triggered = false;

    StereoSmoothSubModule loop_smooth;

    std::string root_dir;
	std::string path;

	Sample samples[NUMBER_OF_SAMPLES];
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

	dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger clockTrigger;

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
        CLOCK_INPUT,
		RESET_INPUT,
        SEQUENCE_INPUT,
		WAV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		AUDIO_OUTPUT_LEFT,
		AUDIO_OUTPUT_RIGHT,
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Autobreak()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");

		std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");
	}

	// Autosave settings
	json_t *dataToJson() override
	{
		//
		// Save selected samples
		//

		json_t *json_root = json_object();
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(json_root, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}

		return json_root;
	}

	// Autoload settings
	void dataFromJson(json_t *json_root) override
	{
		//
		// Load samples
		//

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(json_root, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
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
		unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, NUMBER_OF_SAMPLES);
		wav_input_value = clamp(wav_input_value, 0, NUMBER_OF_SAMPLES - 1);

		if(wav_input_value != selected_sample_slot)
		{
			// Reset the smooth ramp if the selected sample has changed
			loop_smooth.trigger();

			// Set the selected sample
			selected_sample_slot = wav_input_value;
		}

		Sample *selected_sample = &samples[selected_sample_slot];

        //
        // Handle BPM detection
        //

        time_counter += 1.0 / args.sampleRate;

        if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
        {
            if(timer_before != 0)
            {
                double elapsed_time = time_counter - timer_before;
                if(elapsed_time > 0) bpm = 30.0 / elapsed_time;
            }

            timer_before = time_counter;
            clock_triggered = true;
        }

        //
        // Handle reset input
        //

		if (inputs[RESET_INPUT].isConnected())
		{
			if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
			{
                // Reset counters
				actual_playback_position = 0;
				theoretical_playback_position = 0;
				incrementing_bpm_counter = 0;

                // Smooth back into playback
                loop_smooth.trigger();
			}
		}

		if (selected_sample->loaded && (selected_sample->total_sample_count > 0))
		{
			// 60.0 is for conversion from minutes to seconds
			// 8.0 is for 8 beats (2 bars) of loops, which is a typical drum loop length
			float samples_to_play_per_loop = ((60.0 / bpm) * args.sampleRate) * 8.0;

			actual_playback_position = clamp(actual_playback_position, 0.0, selected_sample->total_sample_count - 1);

			float left_output  = GAIN  * selected_sample->leftPlayBuffer[(int)actual_playback_position];
			float right_output = GAIN  * selected_sample->rightPlayBuffer[(int)actual_playback_position];

            // Handle smoothing
            float smooth_rate = (128.0f / args.sampleRate);
            std::tie(left_output, right_output) = loop_smooth.process(left_output, right_output, smooth_rate);

            // Output audio
			outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_output);
			outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_output);

			// Step the theoretical playback position
			theoretical_playback_position = theoretical_playback_position + 1;

			// Optionally jump to new breakbeat position
            if(clock_triggered)
            {
                float sequence_input_value = inputs[SEQUENCE_INPUT].getVoltage() / 10.0;
                int breakbeat_location = (sequence_input_value * 16) - 1;
        		breakbeat_location = clamp(breakbeat_location, -1, 15);

                if(breakbeat_location != -1)
				{
					theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
				}

                clock_triggered = false;
            }

			// Loop the theoretical_playback_position
			if(theoretical_playback_position >= samples_to_play_per_loop)
			{
				theoretical_playback_position = 0;
                loop_smooth.trigger();
			}

			// Map the theoretical playback position to the actual sample playback position
			actual_playback_position = ((float) theoretical_playback_position / samples_to_play_per_loop) * selected_sample->total_sample_count;

			//
			// Increment the bpm counter, which goes from 0 to the number of samples
			// needed to play an entire loop of a theoretical sample at the bpm specified.
			incrementing_bpm_counter = incrementing_bpm_counter + 1;
			if(incrementing_bpm_counter > samples_to_play_per_loop) incrementing_bpm_counter = 0;
		}
	}
};
