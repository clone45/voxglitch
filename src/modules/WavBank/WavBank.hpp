struct WavBank : VoxglitchSamplerModule
{
	unsigned int selected_sample_slot = 0;

	float SAMPLE_START_POSITION = 0.0;
	float SAMPLE_END_POSITION = 1.0;

	unsigned int trig_input_response_mode = TRIGGER;
	std::string rootDir;
	std::string path;
	bool loading_samples = false;

	std::vector<SamplePlayer> sample_players;
	dsp::SchmittTrigger playTrigger;
	DeclickFilter declick_filter;

	bool playback = false;

	// End-of-sample trigger output
	dsp::PulseGenerator endOfSamplePulse;
	bool previousPlaybackState = false;
	double previousPlaybackPosition = 0.0;

	// Auto-increment feature
	bool auto_increment_on_completion = false;  // Disabled by default

	enum ParamIds
	{
		WAV_KNOB,
		WAV_ATTN_KNOB,
		LOOP_SWITCH,
		NUM_PARAMS
	};
	enum InputIds
	{
		TRIG_INPUT,
		WAV_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		WAV_LEFT_OUTPUT,
		WAV_RIGHT_OUTPUT,
		END_OF_SAMPLE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	WavBank()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(LOOP_SWITCH, 0.0f, 1.0f, 0.0f, "LoopSwitch");
		configInput(TRIG_INPUT, "Trigger");
		configInput(WAV_INPUT, "Wave Selection");
		configInput(PITCH_INPUT, "Pitch");
		configOutput(END_OF_SAMPLE_OUTPUT, "End of Sample Trigger");
		// configSwitch(SWITCH_TEST, 0.0f, 1.0f, 1.0f, "Something", {"Value", "Other Value"});
	}

	// Save
	json_t *dataToJson() override
	{
		json_t *json_root = json_object();
		json_object_set_new(json_root, "path", json_string(this->path.c_str()));
		json_object_set_new(json_root, "trig_input_response_mode", json_integer(trig_input_response_mode));
		json_object_set_new(json_root, "auto_increment_on_completion", json_boolean(auto_increment_on_completion));
		return json_root;
	}

	// Load
	void dataFromJson(json_t *json_root) override
	{
		json_t *loaded_path_json = json_object_get(json_root, ("path"));
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path.c_str());
		}

		// Load trigger input response mode
		json_t *trig_input_response_mode_json = json_object_get(json_root, "trig_input_response_mode");
		if (trig_input_response_mode_json)
			trig_input_response_mode = json_integer_value(trig_input_response_mode_json);

		// Load auto-increment setting
		json_t *auto_increment_json = json_object_get(json_root, "auto_increment_on_completion");
		if (auto_increment_json)
			auto_increment_on_completion = json_boolean_value(auto_increment_json);
	}

	void load_samples_from_path(std::string path)
	{
		loading_samples = true;

		// Clear out any old .wav files
		this->sample_players.clear();

		// Load all .wav files found in the folder specified by 'path'
		// this->rootDir = std::string(path);

		std::vector<std::string> dirList = system::getEntries(path.c_str());

		// Sort the vector.  This is in response to a user who's samples were being
		// loaded out of order.  I think it's a mac thing.
		sort(dirList.begin(), dirList.end());

		// TODO: Consider supporting MP3.
		for (auto path : dirList)
		{
			if (
				(rack::string::lowercase(system::getExtension(path)) == "wav") ||
				(rack::string::lowercase(system::getExtension(path)) == ".wav"))
			{
				SamplePlayer sample_player;

				sample_player.loadSample(path);
				this->sample_players.push_back(sample_player);
			}
		}

		loading_samples = false;
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
		// If the samples are being loaded, don't do anything
		if (loading_samples)
			return;

		unsigned int number_of_samples = sample_players.size();

		// Read the input/knob for sample selection
		unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, number_of_samples);
		wav_input_value = clamp(wav_input_value, 0, number_of_samples - 1);

		// Read the loop switch
		bool loop = params[LOOP_SWITCH].getValue();

		// Read the pitch input
		float pitch = inputs[PITCH_INPUT].getVoltage();

		//
		// If the sample has been changed.
		//
		if (wav_input_value != selected_sample_slot)
		{
			// Trigger the declick filter if the selected sample has changed
			declick_filter.trigger();

			// Reset sample position so playback does not start at previous sample position
			sample_players[selected_sample_slot].stop();

			// Set the selected sample
			selected_sample_slot = wav_input_value;

			playback = false;
		}

		// Check to see if the selected sample slot refers to an existing sample.
		// If not, return.  This could happen before any samples have been loaded.
		if (!(sample_players.size() > selected_sample_slot))
			return;

		SamplePlayer *selected_sample_player = &sample_players[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			if (trig_input_response_mode == TRIGGER)
			{
				// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
				// determines if a low-to-high transition happened at the TRIG_INPUT

				if (playTrigger.process(inputs[TRIG_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
				{
					playback = true;
					declick_filter.trigger();
					selected_sample_player->trigger(SAMPLE_START_POSITION, loop);
				}
			}
			else if (trig_input_response_mode == GATE)
			{
				// In gate mode, continue playing back the sample as long as the gate is high
				// 5.0 volts is required for a gate high signal, but this is extra lenient
				// as the standards say a 10.0 or higher signal should be accepted.
				if (inputs[TRIG_INPUT].getVoltage() >= 5.0)
				{
					if (playback == false)
					{
						playback = true;
						selected_sample_player->trigger(SAMPLE_START_POSITION, loop); // TODO: May have to come back to this
						declick_filter.trigger();
					}
				}
				else
				{
					playback = false;
					selected_sample_player->stop();
					declick_filter.trigger();
				}
			}
		}
		// If no cable is connected to the trigger input, then provide constant playback
		else
		{
			playback = true;
		}

		if (playback)
		{
			// Store previous playback position before stepping
			previousPlaybackPosition = selected_sample_player->playback_position;

			float left_audio;
			float right_audio;

			selected_sample_player->getStereoOutput(&left_audio, &right_audio, interpolation);

			declick_filter.process(&left_audio, &right_audio);

			outputs[WAV_LEFT_OUTPUT].setVoltage(left_audio * GAIN);
			outputs[WAV_RIGHT_OUTPUT].setVoltage(right_audio * GAIN);

			selected_sample_player->step(pitch, SAMPLE_START_POSITION, SAMPLE_END_POSITION, loop);

			// Detect end of sample (works in both loop and non-loop modes)
			if (selected_sample_player->sample.loaded)
			{
				unsigned int sample_size = selected_sample_player->sample.size() * SAMPLE_END_POSITION;

				if (loop)
				{
					// In loop mode, detect when playback position wraps around
					if (previousPlaybackPosition >= (sample_size - 1) &&
						selected_sample_player->playback_position < previousPlaybackPosition)
					{
						// Sample reached end and looped back
						endOfSamplePulse.trigger(0.01f);

						// Auto-increment if enabled AND no CV control
						// Don't auto-increment when CV is controlling sample selection
						if (auto_increment_on_completion && !inputs[WAV_INPUT].isConnected())
						{
							incrementSampleSelection();
						}
					}
				}
				else
				{
					// In non-loop mode, detect when sample stops playing
					if (!selected_sample_player->playing && previousPlaybackState)
					{
						// Sample just ended - trigger the pulse
						endOfSamplePulse.trigger(0.01f);

						// Auto-increment if enabled AND no CV control
						// Don't auto-increment when CV is controlling sample selection
						if (auto_increment_on_completion && !inputs[WAV_INPUT].isConnected())
						{
							incrementSampleSelection();
						}
					}
				}
			}

			previousPlaybackState = selected_sample_player->playing;
		}

		// Process the pulse generator and output the trigger
		bool endPulse = endOfSamplePulse.process(1.0 / args.sampleRate);
		outputs[END_OF_SAMPLE_OUTPUT].setVoltage(endPulse ? 10.0f : 0.0f);
	}

private:
	void incrementSampleSelection()
	{
		unsigned int number_of_samples = sample_players.size();
		if (number_of_samples == 0) return;

		// Calculate next sample index with wrap-around
		unsigned int next_sample = (selected_sample_slot + 1) % number_of_samples;

		// Update the WAV_KNOB parameter value
		// The knob ranges from 0.0 to 1.0, so we need to scale appropriately
		float knob_value = (float)next_sample / (float)(number_of_samples - 1);

		// Use setValue to programmatically change the parameter
		// This will update the knob position visually
		params[WAV_KNOB].setValue(knob_value);

		// Stop the current sample
		sample_players[selected_sample_slot].stop();

		// Update the selected sample slot
		selected_sample_slot = next_sample;

		// Get the loop setting
		bool loop = params[LOOP_SWITCH].getValue();

		// Start playback of the new sample
		sample_players[selected_sample_slot].trigger(SAMPLE_START_POSITION, loop);

		// Ensure playback flag is set
		playback = true;

		// Trigger declick filter for smooth transition
		declick_filter.trigger();
	}
};
