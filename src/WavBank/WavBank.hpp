struct WavBank : Module
{
	unsigned int selected_sample_slot = 0;
	float samplePos = 0;
	float smooth_ramp = 1;
	float last_wave_output_voltage[2] = {0};
	std::string rootDir;
	std::string path;

	std::vector<Sample> samples;
	dsp::SchmittTrigger playTrigger;

	bool triggered = false;

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		LOOP_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		WAV_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WAV_LEFT_OUTPUT,
		WAV_RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
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
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "path", json_string(this->path.c_str()));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *loaded_path_json = json_object_get(rootJ, ("path"));
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path.c_str());
		}
	}

	void load_samples_from_path(const char *path)
	{
		// Clear out any old .wav files
		this->samples.clear();

		// Load all .wav files found in the folder specified by 'path'

		this->rootDir = std::string(path);
		std::list<std::string> dirList = system::getEntries(path);

		// TODO: Decide on a maximum memory consuption allowed and abort if
		// that amount of member would be exhausted by loading all of the files
		// in the folder.  Also consider supporting MP3.
		for (auto entry : dirList)
		{
			if (rack::string::lowercase(rack::string::filenameExtension(entry)) == "wav")
			{
				Sample new_sample;
				DEBUG("LOAD OK");
				new_sample.load(entry, false);
				this->samples.push_back(new_sample);
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
		unsigned int number_of_samples = samples.size();

		// Read the input/knob for sample selection
		unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, number_of_samples);
		wav_input_value = clamp(wav_input_value, 0, number_of_samples - 1);

		if(wav_input_value != selected_sample_slot)
		{
			// Reset the smooth ramp if the selected sample has changed
			smooth_ramp = 0;

			// Reset sample position so playback does not start at previous sample position
			// TODO: Think this over.  Is it more flexible to allow people to changes
			// samples without resetting the sample position?
			samplePos = 0;

			// Set the selected sample
			selected_sample_slot = wav_input_value;

			triggered = false;
		}

		// Check to see if the selected sample slot refers to an existing sample.
		// If not, return.  This could happen before any samples have been loaded.
		if(! (samples.size() > selected_sample_slot)) return;

		Sample *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			//
			// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
			// determines if a low-to-high transition happened at the TRIG_INPUT
			//
			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
			{
				samplePos = 0;
				smooth_ramp = 0;
				triggered = true;
			}
		}
		else
		{
			triggered = true;
		}

		// Loop
		if(params[LOOP_SWITCH].getValue() && (samplePos >= selected_sample->total_sample_count)) samplePos = 0;

		if (triggered && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->total_sample_count > 0) && (samplePos < selected_sample->total_sample_count))
		{
			float left_wav_output_voltage;
			float right_wav_output_voltage;

			if (samplePos >= 0)
			{
				left_wav_output_voltage = GAIN  * selected_sample->leftPlayBuffer[(int)samplePos];
				if(selected_sample->channels > 1)
				{
					right_wav_output_voltage = GAIN  * selected_sample->rightPlayBuffer[(int)samplePos];
				}
				else
				{
					right_wav_output_voltage = left_wav_output_voltage;
				}
			}
			else
			{
				// What is this for?  Does it play the sample in reverse?  I think so.
				left_wav_output_voltage = GAIN * selected_sample->leftPlayBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)];
				if(selected_sample->channels > 1) {
					right_wav_output_voltage = GAIN * selected_sample->rightPlayBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)];
				}
				else {
					right_wav_output_voltage = left_wav_output_voltage;
				}
			}

			if(SMOOTH_ENABLED && (smooth_ramp < 1))
			{
				float smooth_rate = (128.0f / args.sampleRate);  // A smooth rate of 128 seems to work best
				smooth_ramp += smooth_rate;
				left_wav_output_voltage = (last_wave_output_voltage[0] * (1 - smooth_ramp)) + (left_wav_output_voltage * smooth_ramp);
				if(selected_sample->channels > 1) {
					right_wav_output_voltage = (last_wave_output_voltage[1] * (1 - smooth_ramp)) + (right_wav_output_voltage * smooth_ramp);
				}
				else {
					right_wav_output_voltage = left_wav_output_voltage;
				}
			}

			outputs[WAV_LEFT_OUTPUT].setVoltage(left_wav_output_voltage);
			outputs[WAV_RIGHT_OUTPUT].setVoltage(right_wav_output_voltage);

			last_wave_output_voltage[0] = left_wav_output_voltage;
			last_wave_output_voltage[1] = right_wav_output_voltage;

			// Increment sample offset (pitch)
			if (inputs[PITCH_INPUT].isConnected())
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate) + ((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f);
			}
			else
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate);
			}
		}
		else
		{
			triggered = false; // Cancel current trigger
			outputs[WAV_LEFT_OUTPUT].setVoltage(0);
			outputs[WAV_RIGHT_OUTPUT].setVoltage(0);
		}
	}
};
