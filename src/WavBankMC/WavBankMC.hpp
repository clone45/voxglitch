//
// WavBankMC
//
// TODO:
// * tooltips for all components

struct WavBankMC : VoxglitchSamplerModule
{
	unsigned int selected_sample_slot = 0;
	double playback_positions[NUMBER_OF_CHANNELS];
  bool playback[NUMBER_OF_CHANNELS];
  float previous_wav_knob_value = 0.0;

  float sample_time = 0;
	float smooth_ramp[NUMBER_OF_CHANNELS] = {1};
	float last_output_voltage[NUMBER_OF_CHANNELS] = {0};
	std::string rootDir;
	std::string path;

  dsp::SchmittTrigger next_wav_cv_trigger;
  dsp::SchmittTrigger prev_wav_cv_trigger;
  dsp::SchmittTrigger trg_cv_trigger;

	dsp::SchmittTrigger next_wav_button_trigger;
	dsp::SchmittTrigger prev_wav_button_trigger;
  dsp::SchmittTrigger trg_button_trigger;

  unsigned int number_of_samples = 0;
  bool smoothing = true;
	std::vector<SampleMC> samples;
  unsigned int sample_change_mode = RESTART_PLAYBACK;

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		LOOP_SWITCH,
    NEXT_WAV_BUTTON_PARAM,
    PREV_WAV_BUTTON_PARAM,
    TRIG_INPUT_BUTTON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		WAV_INPUT,
		VOLUME_INPUT,
		PITCH_INPUT,
    NEXT_WAV_TRIGGER_INPUT,
    PREV_WAV_TRIGGER_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    LEFT_WAV_OUTPUT,
    RIGHT_WAV_OUTPUT,
		POLY_WAV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
    NEXT_WAV_LIGHT,
    PREV_WAV_LIGHT,
    TRIG_LIGHT,
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
		configParam(NEXT_WAV_BUTTON_PARAM, 0.f, 1.f, 0.f, "NextWavButtonParam");
    configParam(PREV_WAV_BUTTON_PARAM, 0.f, 1.f, 0.f, "PrevWavButtonParam");
    configParam(TRIG_INPUT_BUTTON_PARAM, 0.f, 1.f, 0.f, "TrigInputButtonParam");

    reset_all_playback_positions();
    set_all_playback_flags(false);

    // This is a variable that helps us detect if there's been any knob
    // movement, which is important if the user is trying to dial in a sample
    // using the selection know while no CV input is present
    previous_wav_knob_value = params[WAV_KNOB].getValue();
	}

  // Save
	json_t *dataToJson() override
	{

		json_t *json_root = json_object();
		json_object_set_new(json_root, "path", json_string(this->path.c_str()));
    json_object_set_new(json_root, "sample_change_mode", json_integer(sample_change_mode));
    json_object_set_new(json_root, "smoothing", json_integer(smoothing));
		return json_root;
	}

  // Load
	void dataFromJson(json_t *json_root) override
	{

		json_t *loaded_path_json = json_object_get(json_root, ("path"));
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path);
		}

    // Load sample change mode
    json_t* sample_change_mode_json = json_object_get(json_root, "sample_change_mode");
    if (sample_change_mode_json) sample_change_mode = json_integer_value(sample_change_mode_json);

    // Load smoothing
    json_t* smoothing_json = json_object_get(json_root, "smoothing");
    if (smoothing_json) smoothing = json_integer_value(smoothing_json);

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

  void start_smoothing(unsigned int channel)
  {
    smooth_ramp[channel] = 0;
  }

  void smooth_all_channels()
  {
    for(unsigned int channel = 0; channel < NUMBER_OF_CHANNELS; channel++)
    {
      start_smoothing(channel);
    }
  }

  void increment_selected_sample()
  {
    change_selected_sample((selected_sample_slot + 1) % this->samples.size());
  }

  void decrement_selected_sample()
  {
    if(selected_sample_slot > 0)
    {
      change_selected_sample(selected_sample_slot - 1);
    }
    else
    {
      change_selected_sample(this->samples.size() - 1);
    }
  }

  void change_selected_sample(unsigned int new_sample_slot)
  {
    if(this->samples.size() != 0)
    {
      // Reset the smooth ramp if the selected sample has changed
      smooth_all_channels();

      if(this->sample_change_mode == RESTART_PLAYBACK)
      {
        reset_all_playback_positions();
        set_all_playback_flags(false);
      }

      // Set the selected sample
      selected_sample_slot = new_sample_slot;
    }
  }

  void process_wav_cv_input()
  {
    unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, number_of_samples);

		wav_input_value = clamp(wav_input_value, 0, number_of_samples - 1);
    previous_wav_knob_value = params[WAV_KNOB].getValue();

    // If the sample has been changed, then change selected sample slot
		if(wav_input_value != selected_sample_slot) change_selected_sample(wav_input_value);
  }

  void manual_wav_selection()
  {
    if(previous_wav_knob_value != params[WAV_KNOB].getValue())
    {
      process_wav_knob_selection();
    }
    else
    {
      process_wav_navigation_buttons();
    }
  }

  void process_wav_knob_selection()
  {
    unsigned int wav_input_value = params[WAV_KNOB].getValue() * number_of_samples;

    if(this->samples.size() == 0) return;

		wav_input_value = clamp(wav_input_value, 0, number_of_samples - 1);
    previous_wav_knob_value = params[WAV_KNOB].getValue();

    if(wav_input_value != selected_sample_slot)
		{
      change_selected_sample(wav_input_value);
		}
  }

  bool process_trg_input()
  {
    bool trg_is_triggered = trg_cv_trigger.process(inputs[TRIG_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || trg_button_trigger.process(params[TRIG_INPUT_BUTTON_PARAM].getValue());
    lights[TRIG_LIGHT].setSmoothBrightness(trg_is_triggered, this->sample_time);
    return(trg_is_triggered);
  }

  void process_wav_navigation_buttons()
  {
    if(this->samples.size() == 0) return;

    // If next_wav button is pressed, step to the next sample
    bool next_wav_is_triggered = next_wav_cv_trigger.process(inputs[NEXT_WAV_TRIGGER_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || next_wav_button_trigger.process(params[NEXT_WAV_BUTTON_PARAM].getValue());
    if(next_wav_is_triggered) increment_selected_sample();
    lights[NEXT_WAV_LIGHT].setSmoothBrightness(next_wav_is_triggered, this->sample_time);

    // Now do prev_wav_is_triggered
    bool prev_wav_is_triggered = prev_wav_cv_trigger.process(inputs[PREV_WAV_TRIGGER_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || prev_wav_button_trigger.process(params[PREV_WAV_BUTTON_PARAM].getValue());
    if(prev_wav_is_triggered) decrement_selected_sample();
    lights[PREV_WAV_LIGHT].setSmoothBrightness(prev_wav_is_triggered, this->sample_time);

    // If either wav navigation button is detected, then set the WAV knob to
    // match the currently selected sample
    if(next_wav_is_triggered || prev_wav_is_triggered)
    {
      set_wav_knob_position();
      // params[WAV_KNOB].setValue((float) selected_sample_slot / (float) number_of_samples);
    }
  }

  void sort_samples_by_filename(std::vector<SampleMC>& samples)
  {
      std::sort(samples.begin(), samples.end(), [](const SampleMC& a, const SampleMC& b) {
          return a.filename < b.filename;
      });
  }

	void load_samples_from_path(std::string path)
	{
		// Clear out any old .wav files
    // Reminder: this->samples is a vector, and vectors have a .clear() menthod.
		this->samples.clear();

		// Load all .wav files found in the folder specified by 'path'
		this->rootDir = path;

		std::vector<std::string> dirList = system::getEntries(path.c_str());

    // Sort the vector.  This is in response to a user who's samples were being
    // loaded out of order.  I think it's a mac thing.
    sort(dirList.begin(), dirList.end());

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

    // Sort the samples vector based on a member attribute, e.g., filename
    sort_samples_by_filename(this->samples);

	}

  // Helper functions used by WavBankMCReadout
  bool wav_input_not_connected()
  {
    return(! inputs[WAV_INPUT].isConnected());
  }

  void set_wav_knob_position()
  {
    params[WAV_KNOB].setValue((float) selected_sample_slot / (float) number_of_samples);
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

		number_of_samples = samples.size();
    sample_time = args.sampleTime;
    float smooth_rate = (128.0f / args.sampleRate);

		// If there's a cable in the wav cv input, it has precendence over all
    // other methods of selecting the .wav file.  If there isn't, then the
    // selected wav file can be incremented, decremented, or reset using the
    // corresponding buttons or inputs.

    if(inputs[WAV_INPUT].isConnected())
    {
      process_wav_cv_input();
    }
    else
    {
      manual_wav_selection();
    }

		// Check to see if the selected sample slot refers to an existing sample.
		// If not, return.  This edge case could happen before any samples have been loaded.
		if(selected_sample_slot >= samples.size()) return;

		SampleMC *selected_sample = &samples[selected_sample_slot];

    // If either the TRG CV input is triggered or the corresponding button is
    // pressed, then restart all playback positions and start playback.
    if(process_trg_input())
    {
      reset_all_playback_positions();
      set_all_playback_flags(true);
      smooth_all_channels();
    }

    if(! inputs[TRIG_INPUT].isConnected())
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
        if (playback_positions[channel] >= selected_sample->size())
        {
          playback_positions[channel] = (playback_positions[channel] - selected_sample->size());
          start_smoothing(channel);
        }
      }
    }

    for (unsigned int channel = 0; channel < selected_sample->number_of_channels; channel++)
    {
      if (playback[channel] && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->size() > 0) && (playback_positions[channel] < selected_sample->size()))
  		{
          float output_voltage = selected_sample->read(channel, (int)playback_positions[channel]);

          //
          // Volume control
          //
          float channel_volume = 1.0;
          if(inputs[VOLUME_INPUT].isConnected())
          {
            // If the current channel has a volume input provided by the CV
            // input at that channel, then use it.
            if(channel < (unsigned int) inputs[VOLUME_INPUT].getChannels())
            {
              channel_volume = inputs[VOLUME_INPUT].getVoltage(channel);
            }
            // Otherwise use the volume of the first channel
            else
            {
              channel_volume = inputs[VOLUME_INPUT].getVoltage(0);
            }
          }
          output_voltage *= channel_volume;

          //
          // Smoothing
          //
          if(this->smoothing && (smooth_ramp[channel] < 1))
    			{
    				smooth_ramp[channel] += smooth_rate;
    				output_voltage = (last_output_voltage[channel] * (1 - smooth_ramp[channel])) + (output_voltage * smooth_ramp[channel]);
    			}
          last_output_voltage[channel] = output_voltage;

          // Output the audio
    			outputs[POLY_WAV_OUTPUT].setVoltage(output_voltage, channel);

          if(channel == 0) outputs[LEFT_WAV_OUTPUT].setVoltage(output_voltage);
          if(channel == 1) outputs[RIGHT_WAV_OUTPUT].setVoltage(output_voltage);

          // Copy the left output to the right output if there's only 1 channel
          if(selected_sample->number_of_channels == 1) outputs[RIGHT_WAV_OUTPUT].setVoltage(output_voltage);

          // Increment sample offset (pitch)
    			if (inputs[PITCH_INPUT].isConnected())
    			{
            float channel_pitch = 0.5;

            if(channel < (unsigned int) inputs[PITCH_INPUT].getChannels())
            {
              channel_pitch = inputs[PITCH_INPUT].getVoltage(channel);
            }
            else
            {
              channel_pitch = inputs[PITCH_INPUT].getVoltage(0);
            }
            // If there's a polyphonic cable at the pitch input with a channel
            // that matches the sample's channel, then use that cable to control
            // the pitch of the channel playback.
    				playback_positions[channel] += (selected_sample->sample_rate / args.sampleRate) + ((channel_pitch / 10.0f) - 0.5f);
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
