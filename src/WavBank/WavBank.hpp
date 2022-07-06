struct WavBank : VoxglitchSamplerModule
{
	unsigned int selected_sample_slot = 0;
	double samplePos = 0;
	float smooth_ramp = 1;
	float last_wave_output_voltage[2] = {0};
  unsigned int trig_input_response_mode = TRIGGER;
	std::string rootDir;
	std::string path;

	std::vector<Sample> samples;
	dsp::SchmittTrigger playTrigger;

  bool playback = false;

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

  // Save
	json_t *dataToJson() override
	{
		json_t *json_root = json_object();
		json_object_set_new(json_root, "path", json_string(this->path.c_str()));
    json_object_set_new(json_root, "trig_input_response_mode", json_integer(trig_input_response_mode));
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
    json_t* trig_input_response_mode_json = json_object_get(json_root, "trig_input_response_mode");
    if (trig_input_response_mode_json) trig_input_response_mode = json_integer_value(trig_input_response_mode_json);
	}

	void load_samples_from_path(std::string path)
	{
		// Clear out any old .wav files
		this->samples.clear();

		// Load all .wav files found in the folder specified by 'path'
		// this->rootDir = std::string(path);

		std::vector<std::string> dirList = system::getEntries(path.c_str());

		// TODO: Decide on a maximum memory consuption allowed and abort if
		// that amount of member would be exhausted by loading all of the files
		// in the folder.  Also consider supporting MP3.
		for (auto entry : dirList)
		{
			if (
        (rack::string::lowercase(system::getExtension(entry)) == "wav") ||
        (rack::string::lowercase(system::getExtension(entry)) == ".wav")
      )
			{
				Sample new_sample;
				new_sample.load(entry);
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

    //
    // If the sample has been changed.
    //
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

      playback = false;
		}

		// Check to see if the selected sample slot refers to an existing sample.
		// If not, return.  This could happen before any samples have been loaded.
		if(! (samples.size() > selected_sample_slot)) return;

		Sample *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
      if(trig_input_response_mode == TRIGGER)
      {
        // playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
  			// determines if a low-to-high transition happened at the TRIG_INPUT

  			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
  			{
  				samplePos = 0;
  				smooth_ramp = 0;
  				playback = true;
  			}
      }
      else if(trig_input_response_mode == GATE)
      {
        // In gate mode, continue playing back the sample as long as the gate is high
        // 5.0 volts is required for a gate high signal, but this is extra lenient
        // as the standards say a 10.0 or higher signal should be accepted.
        if(inputs[TRIG_INPUT].getVoltage() >= 5.0)
        {
          if(playback == false)
          {
            playback = true;
            samplePos = 0;
    				smooth_ramp = 0;
          }
        }
        else
        {
          playback = false;
          samplePos = 0;
  				smooth_ramp = 0;
        }
      }
		}
    // If no cable is connected to the trigger input, then provide constant playback
		else
		{
			playback = true;
		}

		// Loop
		if(params[LOOP_SWITCH].getValue() && (samplePos >= selected_sample->size())) samplePos = 0;

		if (playback && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->size() > 0) && (samplePos < selected_sample->size()))
		{
			float left_wav_output_voltage;
			float right_wav_output_voltage;

			if (samplePos >= 0)
			{
        selected_sample->read((int)samplePos, &left_wav_output_voltage, &right_wav_output_voltage);
			}
			else
			{
        selected_sample->read(floor(selected_sample->size() - 1 + samplePos), &left_wav_output_voltage, &right_wav_output_voltage);
			}

      left_wav_output_voltage *= GAIN;
      right_wav_output_voltage *= GAIN;

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
      // This block of code gets run if the sample is loading, or there's no
      // sample data, or most importantly, if the sample playback had ended
      // and loop == false

			playback = false; // Cancel current trigger
			outputs[WAV_LEFT_OUTPUT].setVoltage(0);
			outputs[WAV_RIGHT_OUTPUT].setVoltage(0);
		}
	}
};
