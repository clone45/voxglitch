struct Netrunner : VoxglitchSamplerModule
{
	unsigned int selected_sample_slot = 0;

	float SAMPLE_START_POSITION = 0.0;
	float SAMPLE_END_POSITION = 1.0;

	unsigned int trig_input_response_mode = TRIGGER;
	std::string path;
	bool loading_samples = false;

	std::vector<SamplePlayer> sample_players;
	std::vector<AsyncSampleLoader> async_loaders;
	std::vector<std::string> pending_urls;

	dsp::SchmittTrigger playTrigger;
	DeclickFilter declick_filter;

	bool playback = false;

	dsp::PulseGenerator endOfSamplePulse;
	bool previousPlaybackState = false;
	double previousPlaybackPosition = 0.0;

	bool auto_increment_on_completion = false;

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

	Netrunner()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(LOOP_SWITCH, 0.0f, 1.0f, 0.0f, "LoopSwitch");
		configInput(TRIG_INPUT, "Trigger");
		configInput(WAV_INPUT, "Wave Selection");
		configInput(PITCH_INPUT, "Pitch");
		configOutput(END_OF_SAMPLE_OUTPUT, "End of Sample Trigger");
	}

	json_t *dataToJson() override
	{
		json_t *json_root = json_object();
		json_object_set_new(json_root, "path", json_string(this->path.c_str()));
		json_object_set_new(json_root, "trig_input_response_mode", json_integer(trig_input_response_mode));
		json_object_set_new(json_root, "auto_increment_on_completion", json_boolean(auto_increment_on_completion));
		return json_root;
	}

	void dataFromJson(json_t *json_root) override
	{
		json_t *loaded_path_json = json_object_get(json_root, ("path"));
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			if (!this->path.empty())
			{
				load_samples_from_config(this->path);
			}
		}

		json_t *trig_input_response_mode_json = json_object_get(json_root, "trig_input_response_mode");
		if (trig_input_response_mode_json)
			trig_input_response_mode = json_integer_value(trig_input_response_mode_json);

		json_t *auto_increment_json = json_object_get(json_root, "auto_increment_on_completion");
		if (auto_increment_json)
			auto_increment_on_completion = json_boolean_value(auto_increment_json);
	}

	void load_samples_from_config(std::string config_path)
	{
		loading_samples = true;

		sample_players.clear();
		async_loaders.clear();
		pending_urls.clear();

		FILE* file = fopen(config_path.c_str(), "r");
		if (!file)
		{
			WARN("Netrunner: Failed to open config file: %s", config_path.c_str());
			loading_samples = false;
			return;
		}

		json_error_t error;
		json_t* root = json_loadf(file, 0, &error);
		fclose(file);

		if (!root)
		{
			WARN("Netrunner: Failed to parse JSON: %s", error.text);
			loading_samples = false;
			return;
		}

		json_t* samples_array = json_object_get(root, "samples");
		if (!samples_array || !json_is_array(samples_array))
		{
			WARN("Netrunner: Config missing 'samples' array");
			json_decref(root);
			loading_samples = false;
			return;
		}

		size_t num_samples = json_array_size(samples_array);
		INFO("Netrunner: Loading %zu samples from config", num_samples);

		for (size_t i = 0; i < num_samples; i++)
		{
			json_t* sample_obj = json_array_get(samples_array, i);
			if (!sample_obj || !json_is_object(sample_obj))
				continue;

			json_t* url_json = json_object_get(sample_obj, "url");
			if (!url_json || !json_is_string(url_json))
				continue;

			std::string url = json_string_value(url_json);

			std::string name = "";
			json_t* name_json = json_object_get(sample_obj, "name");
			if (name_json && json_is_string(name_json))
			{
				name = json_string_value(name_json);
			}

			SamplePlayer sample_player;
			sample_player.sample.display_name = name.empty() ? url : name;
			sample_players.push_back(sample_player);

			AsyncSampleLoader loader;
			async_loaders.push_back(std::move(loader));

			pending_urls.push_back(url);

			INFO("Netrunner: Queued sample %zu: %s -> %s", i, name.c_str(), url.c_str());
		}

		json_decref(root);

		for (size_t i = 0; i < async_loaders.size(); i++)
		{
			async_loaders[i].startLoad(pending_urls[i], 120.0f);
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
		if (loading_samples)
			return;

		for (size_t i = 0; i < async_loaders.size(); i++)
		{
			if (async_loaders[i].isReady())
			{
				std::unique_ptr<Sample> loaded_sample = async_loaders[i].getLoadedSample();
				if (loaded_sample && i < sample_players.size())
				{
					sample_players[i].sample = *loaded_sample;
					INFO("Netrunner: Sample %zu loaded successfully", i);
				}
			}
		}

		unsigned int number_of_samples = sample_players.size();
		if (number_of_samples == 0)
			return;

		unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, number_of_samples);
		wav_input_value = clamp(wav_input_value, 0, number_of_samples - 1);

		bool loop = params[LOOP_SWITCH].getValue();

		float pitch = inputs[PITCH_INPUT].getVoltage();

		if (wav_input_value != selected_sample_slot)
		{
			declick_filter.trigger();

			sample_players[selected_sample_slot].stop();

			selected_sample_slot = wav_input_value;

			playback = false;
		}

		if (!(sample_players.size() > selected_sample_slot))
			return;

		SamplePlayer *selected_sample_player = &sample_players[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			if (trig_input_response_mode == TRIGGER)
			{
				if (playTrigger.process(inputs[TRIG_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
				{
					playback = true;
					declick_filter.trigger();
					selected_sample_player->trigger(SAMPLE_START_POSITION, loop);
				}
			}
			else if (trig_input_response_mode == GATE)
			{
				if (inputs[TRIG_INPUT].getVoltage() >= 5.0)
				{
					if (playback == false)
					{
						playback = true;
						selected_sample_player->trigger(SAMPLE_START_POSITION, loop);
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
		else
		{
			playback = true;
		}

		if (playback)
		{
			previousPlaybackPosition = selected_sample_player->playback_position;

			float left_audio;
			float right_audio;

			selected_sample_player->getStereoOutput(&left_audio, &right_audio, interpolation);

			declick_filter.process(&left_audio, &right_audio);

			outputs[WAV_LEFT_OUTPUT].setVoltage(left_audio * GAIN);
			outputs[WAV_RIGHT_OUTPUT].setVoltage(right_audio * GAIN);

			selected_sample_player->step(pitch, SAMPLE_START_POSITION, SAMPLE_END_POSITION, loop);

			if (selected_sample_player->sample.loaded)
			{
				unsigned int sample_size = selected_sample_player->sample.size() * SAMPLE_END_POSITION;

				if (loop)
				{
					if (previousPlaybackPosition >= (sample_size - 1) &&
						selected_sample_player->playback_position < previousPlaybackPosition)
					{
						endOfSamplePulse.trigger(0.01f);

						if (auto_increment_on_completion && !inputs[WAV_INPUT].isConnected())
						{
							incrementSampleSelection();
						}
					}
				}
				else
				{
					if (!selected_sample_player->playing && previousPlaybackState)
					{
						endOfSamplePulse.trigger(0.01f);

						if (auto_increment_on_completion && !inputs[WAV_INPUT].isConnected())
						{
							incrementSampleSelection();
						}
					}
				}
			}

			previousPlaybackState = selected_sample_player->playing;
		}

		bool endPulse = endOfSamplePulse.process(1.0 / args.sampleRate);
		outputs[END_OF_SAMPLE_OUTPUT].setVoltage(endPulse ? 10.0f : 0.0f);
	}

private:
	void incrementSampleSelection()
	{
		unsigned int number_of_samples = sample_players.size();
		if (number_of_samples == 0) return;

		unsigned int next_sample = (selected_sample_slot + 1) % number_of_samples;

		float knob_value = (float)next_sample / (float)(number_of_samples - 1);

		params[WAV_KNOB].setValue(knob_value);

		sample_players[selected_sample_slot].stop();

		selected_sample_slot = next_sample;

		bool loop = params[LOOP_SWITCH].getValue();

		sample_players[selected_sample_slot].trigger(SAMPLE_START_POSITION, loop);

		playback = true;

		declick_filter.trigger();
	}
};