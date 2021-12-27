//
// TODO: fix pitch input to be 1v/octave
//

struct WavBankMC : Module
{
	unsigned int selected_sample_slot = 0;
  // double samplePos = 0;
	double playback_positions[NUMBER_OF_CHANNELS];
  bool playback[NUMBER_OF_CHANNELS];

	float smooth_ramp = 1;
	float last_wave_output_voltage[2] = {0};
  unsigned int trig_input_response_mode = TRIGGER;
	std::string rootDir;
	std::string path;

	std::vector<SampleMC> samples;
	dsp::SchmittTrigger playTrigger;

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		LOOP_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		WAV_INPUT,
		VOLUME_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_WAV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	WavBankMC()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(LOOP_SWITCH, 0.0f, 1.0f, 0.0f, "LoopSwitch");

    reset_all_playback_positions();
    set_all_playback_flags(false);
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

  void reset_all_playback_positions()
  {
    for(unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++)
    {
      playback_positions[i] = 0.0;
    }
  }

  void set_all_playback_flags(bool value)
  {
    for(unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++)
    {
      playback[i] = value;
    }
  }

	void load_samples_from_path(const char *path)
	{
		// Clear out any old .wav files
    // Reminder: this->samples is a vector, and vectors have a .clear() menthod.
		this->samples.clear();

		// Load all .wav files found in the folder specified by 'path'
		this->rootDir = std::string(path);

		std::vector<std::string> dirList = system::getEntries(path);

		// TODO: Decide on a maximum memory consuption allowed and abort if
		// that amount of member would be exhausted by loading all of the files
		// in the folder.
		for (auto entry : dirList)
		{
			if (
        // Something happened in Rack 2 where the extension started to include
        // the ".", so I decided to check for both versions, just in case.
        (rack::string::lowercase(system::getExtension(entry)) == "wav") ||
        (rack::string::lowercase(system::getExtension(entry)) == ".wav")
      )
			{
        // Create new multi-channel sample.  This structure is defined in Common/sample_mc.hpp
				SampleMC new_sample;

        // Load the sample data from the disk
				new_sample.load(entry);

        // Reminder: .push_back is a method of vectors that pushes the object
        // to the end of a list.
				this->samples.push_back(new_sample);
			}
		}

    DEBUG("done loading files");

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
      reset_all_playback_positions();
			// samplePos = 0;


			// Set the selected sample
			selected_sample_slot = wav_input_value;

      set_all_playback_flags(false);
		}

		// Check to see if the selected sample slot refers to an existing sample.
		// If not, return.  This edge case could happen before any samples have been loaded.
		if(! (samples.size() > selected_sample_slot)) return;

		SampleMC *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
      if(trig_input_response_mode == TRIGGER)
      {
        // playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
  			// determines if a low-to-high transition happened at the TRIG_INPUT

  			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
  			{
  				reset_all_playback_positions();
          set_all_playback_flags(true);
  				smooth_ramp = 0;
  			}
      }
		}
    // If no cable is connected to the trigger input, then provide constant playback
		else
		{
			set_all_playback_flags(true);
		}

		// If the loop mode is true, check each channel position.  If any are past
    // the end of the sample, then reset them to 0.  (This reset might need
    // some work to be more accurate by indexing a little from the start position
    // based on the difference from the sample size and playback position.)

		if(params[LOOP_SWITCH].getValue() == true)
    {
      for (unsigned int channel = 0; channel < selected_sample->number_of_channels; channel++)
      {
        // Note: All channels in a .wav file have the same length
        if (playback_positions[channel] >= selected_sample->size()) playback_positions[channel] = 0;
      }
    }

    for (unsigned int channel = 0; channel < selected_sample->number_of_channels; channel++)
    {
      if (playback[channel] && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->size() > 0) && (playback_positions[channel] < selected_sample->size()))
  		{
          float output_voltage = selected_sample->read(channel, (int)playback_positions[channel]);
          float channel_volume = inputs[VOLUME_INPUT].getVoltage(channel);
          output_voltage *= channel_volume;

    			outputs[POLY_WAV_OUTPUT].setVoltage(output_voltage, channel);

          // Increment sample offset (pitch)
    			if (inputs[PITCH_INPUT].isConnected())
    			{
            // If there's a polyphonic cable at the pitch input with a channel
            // that matches the sample's channel, then use that cable to control
            // the pitch of the channel playback.
            // unsigned int pitch_input_channel = std::min((unsigned int) inputs[PITCH_INPUT].getChannels() - 1, channel);
    				playback_positions[channel] += (selected_sample->sample_rate / args.sampleRate) + ((inputs[PITCH_INPUT].getVoltage(channel) / 10.0f) - 0.5f);
    			}
    			else
    			{
    				playback_positions[channel] += (selected_sample->sample_rate / args.sampleRate);
    			}
      }
      // This block of code gets run if the sample is loading, or there's no
      // sample data, or most importantly, if the sample playback had ended
      // and loop == false
      else
      {
        playback[channel] = false; // Cancel current trigger
        outputs[POLY_WAV_OUTPUT].setVoltage(0, channel);
      }
    } // end of foreach channel

    outputs[POLY_WAV_OUTPUT].setChannels(selected_sample->number_of_channels);

  } // end of process loop
};
