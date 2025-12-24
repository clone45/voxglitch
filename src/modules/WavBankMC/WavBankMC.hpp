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

  // EOC trigger state
  dsp::PulseGenerator eoc_pulse;
  int last_division[NUMBER_OF_CHANNELS] = {-1};  // Track which division we're in per channel

	enum ParamIds {
		WAV_KNOB,
		WAV_ATTN_KNOB,
		LOOP_SWITCH,
		NEXT_WAV_BUTTON_PARAM,
		PREV_WAV_BUTTON_PARAM,
		TRIG_INPUT_BUTTON_PARAM,
		START_KNOB,
		START_ATTN_KNOB,
		END_KNOB,
		END_ATTN_KNOB,
		ATTACK_KNOB,
		ATTACK_ATTN_KNOB,
		DECAY_KNOB,
		DECAY_ATTN_KNOB,
		PITCH_ATTN_KNOB,
		VOL_ATTN_KNOB,
		DIVISION_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		WAV_INPUT,
		VOLUME_INPUT,
		PITCH_INPUT,
		NEXT_WAV_TRIGGER_INPUT,
		PREV_WAV_TRIGGER_INPUT,
		START_INPUT,
		END_INPUT,
		ATTACK_INPUT,
		DECAY_INPUT,
		LOOP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		LEFT_WAV_OUTPUT,
		RIGHT_WAV_OUTPUT,
		POLY_WAV_OUTPUT,
		EOC_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
    NEXT_WAV_BUTTON_LIGHT,
    PREV_WAV_BUTTON_LIGHT,
    TRIG_INPUT_BUTTON_LIGHT,
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
		configParam(START_KNOB, 0.0f, 1.0f, 0.0f, "Start Position");
		configParam(START_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "Start CV Attenuator");
		configParam(END_KNOB, 0.0f, 1.0f, 1.0f, "End Position");
		configParam(END_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "End CV Attenuator");
		configParam(ATTACK_KNOB, 0.0f, 1.0f, 0.0f, "Attack Time");
		configParam(ATTACK_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "Attack CV Attenuator");
		configParam(DECAY_KNOB, 0.0f, 1.0f, 1.0f, "Decay Time");
		configParam(DECAY_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "Decay CV Attenuator");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "Pitch CV Attenuator");
		configParam(VOL_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "Volume CV Attenuator");
		configParam(DIVISION_KNOB, 1.0f, 16.0f, 1.0f, "EOC Division");
		getParamQuantity(DIVISION_KNOB)->snapEnabled = true;

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

  // Calculate the effective start position in samples based on knob + CV
  double getStartPosition(unsigned int sample_size)
  {
    float start_value = params[START_KNOB].getValue();
    if (inputs[START_INPUT].isConnected())
    {
      float cv = inputs[START_INPUT].getVoltage() / 10.0f;
      float attn = params[START_ATTN_KNOB].getValue();
      start_value = clamp(start_value + (cv * attn), 0.0f, 1.0f);
    }
    return start_value * sample_size;
  }

  // Calculate the effective end position in samples based on knob + CV
  double getEndPosition(unsigned int sample_size)
  {
    float end_value = params[END_KNOB].getValue();
    if (inputs[END_INPUT].isConnected())
    {
      float cv = inputs[END_INPUT].getVoltage() / 10.0f;
      float attn = params[END_ATTN_KNOB].getValue();
      end_value = clamp(end_value + (cv * attn), 0.0f, 1.0f);
    }
    return end_value * sample_size;
  }

  // Get attack time as a normalized value (0.0 = instant, 1.0 = max time)
  float getAttackValue()
  {
    float attack_value = params[ATTACK_KNOB].getValue();
    if (inputs[ATTACK_INPUT].isConnected())
    {
      float cv = inputs[ATTACK_INPUT].getVoltage() / 10.0f;
      float attn = params[ATTACK_ATTN_KNOB].getValue();
      attack_value = clamp(attack_value + (cv * attn), 0.0f, 1.0f);
    }
    return attack_value;
  }

  // Get decay time as a normalized value (0.0 = instant, 1.0 = infinite/no decay)
  float getDecayValue()
  {
    float decay_value = params[DECAY_KNOB].getValue();
    if (inputs[DECAY_INPUT].isConnected())
    {
      float cv = inputs[DECAY_INPUT].getVoltage() / 10.0f;
      float attn = params[DECAY_ATTN_KNOB].getValue();
      decay_value = clamp(decay_value + (cv * attn), 0.0f, 1.0f);
    }
    return decay_value;
  }

  // Calculate envelope multiplier based on position within start/end range
  // Returns 1.0 when no envelope is needed, otherwise 0.0-1.0
  float calculateEnvelope(double position, double start_pos, double end_pos, float attack, float decay)
  {
    // Skip envelope calculation if attack=0 and decay=1 (defaults)
    if (attack <= 0.0f && decay >= 1.0f)
    {
      return 1.0f;
    }

    double region_length = end_pos - start_pos;
    if (region_length <= 0) return 1.0f;

    double position_in_region = position - start_pos;
    float envelope = 1.0f;

    // Attack phase: fade in at the beginning
    if (attack > 0.0f)
    {
      // Attack length is a percentage of the region (scaled exponentially for better feel)
      double attack_length = region_length * attack * attack * 0.5; // max 50% of region
      if (position_in_region < attack_length && attack_length > 0)
      {
        envelope *= (float)(position_in_region / attack_length);
      }
    }

    // Decay phase: fade out at the end
    if (decay < 1.0f)
    {
      // Decay length is a percentage of the region (scaled exponentially)
      double decay_length = region_length * (1.0f - decay) * (1.0f - decay) * 0.5; // max 50% of region
      double distance_from_end = end_pos - position;
      if (distance_from_end < decay_length && decay_length > 0)
      {
        envelope *= (float)(distance_from_end / decay_length);
      }
    }

    return clamp(envelope, 0.0f, 1.0f);
  }

  void reset_all_playback_positions()
  {
    for(unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++)
    {
      playback_positions[i] = 0.0;
    }
  }

  void reset_all_playback_positions_to_start(double start_position)
  {
    for(unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++)
    {
      playback_positions[i] = start_position;
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
    lights[TRIG_INPUT_BUTTON_LIGHT].setSmoothBrightness(trg_is_triggered, this->sample_time);
    return(trg_is_triggered);
  }

  void process_wav_navigation_buttons()
  {
    if(this->samples.size() == 0) return;

    // If next_wav button is pressed, step to the next sample
    bool next_wav_is_triggered = next_wav_cv_trigger.process(inputs[NEXT_WAV_TRIGGER_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || next_wav_button_trigger.process(params[NEXT_WAV_BUTTON_PARAM].getValue());
    if(next_wav_is_triggered) increment_selected_sample();
    lights[NEXT_WAV_BUTTON_LIGHT].setSmoothBrightness(next_wav_is_triggered, this->sample_time);

    // Now do prev_wav_is_triggered
    bool prev_wav_is_triggered = prev_wav_cv_trigger.process(inputs[PREV_WAV_TRIGGER_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger) || prev_wav_button_trigger.process(params[PREV_WAV_BUTTON_PARAM].getValue());
    if(prev_wav_is_triggered) decrement_selected_sample();
    lights[PREV_WAV_BUTTON_LIGHT].setSmoothBrightness(prev_wav_is_triggered, this->sample_time);

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
			// Something happened in Rack 2 where the extension started to include
			// the ".", so I decided to check for both versions, just in case.
			std::string ext = rack::string::lowercase(system::getExtension(entry));
			if (ext == "wav" || ext == ".wav" || ext == "mp3" || ext == ".mp3")
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

    // Calculate effective start and end positions based on knobs + CV
    double start_position = getStartPosition(selected_sample->size());
    double end_position = getEndPosition(selected_sample->size());

    // Ensure start is before end (swap if needed)
    if (start_position > end_position)
    {
      double temp = start_position;
      start_position = end_position;
      end_position = temp;
    }

    // Get attack and decay values for envelope
    float attack_value = getAttackValue();
    float decay_value = getDecayValue();

    // If either the TRG CV input is triggered or the corresponding button is
    // pressed, then restart all playback positions to start position and begin playback.
    if(process_trg_input())
    {
      reset_all_playback_positions_to_start(start_position);
      set_all_playback_flags(true);
      smooth_all_channels();
      // Reset division tracking for all channels
      for (unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++)
      {
        last_division[i] = -1;
      }
    }

    if(! inputs[TRIG_INPUT].isConnected())
    {
      set_all_playback_flags(true);
    }


		// Check if loop is enabled (either via switch or CV input)
    bool loop_enabled = (params[LOOP_SWITCH].getValue() == true);
    if (inputs[LOOP_INPUT].isConnected())
    {
      loop_enabled = loop_enabled || (inputs[LOOP_INPUT].getVoltage() >= 1.0f);
    }

    // Calculate division size for EOC trigger output based on division knob
    // Division knob ranges from 1-16: at 1, only trigger at end; at 2, trigger at 50% and 100%, etc.
    int num_divisions = (int)params[DIVISION_KNOB].getValue();
    double region_length = end_position - start_position;
    double division_size = (num_divisions > 0) ? (region_length / (double)num_divisions) : region_length;

    // If loop mode is enabled, check each channel position.  If any are past
    // the end position, then reset them to the start position.

		if(loop_enabled)
    {
      for (unsigned int channel = 0; channel < selected_sample->number_of_channels; channel++)
      {
        // Note: All channels in a .wav file have the same length
        if (playback_positions[channel] >= end_position)
        {
          // Calculate overshoot and wrap around to start
          double overshoot = playback_positions[channel] - end_position;
          double loop_length = end_position - start_position;
          if (loop_length > 0)
          {
            playback_positions[channel] = start_position + fmod(overshoot, loop_length);
          }
          else
          {
            playback_positions[channel] = start_position;
          }
          start_smoothing(channel);

          // Reset division tracking for this channel
          last_division[channel] = -1;
        }
      }
    }

    for (unsigned int channel = 0; channel < selected_sample->number_of_channels; channel++)
    {
      if (playback[channel] && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->size() > 0) && (playback_positions[channel] < end_position))
  		{
          float output_voltage = selected_sample->read(channel, (int)playback_positions[channel]);

          //
          // Volume control
          //
          float channel_volume = 1.0;
          if(inputs[VOLUME_INPUT].isConnected())
          {
            float vol_attn = params[VOL_ATTN_KNOB].getValue();
            float vol_cv;
            // If the current channel has a volume input provided by the CV
            // input at that channel, then use it.
            if(channel < (unsigned int) inputs[VOLUME_INPUT].getChannels())
            {
              vol_cv = inputs[VOLUME_INPUT].getVoltage(channel);
            }
            // Otherwise use the volume of the first channel
            else
            {
              vol_cv = inputs[VOLUME_INPUT].getVoltage(0);
            }
            // Apply attenuverter (0 = no CV effect, 1 = full CV effect)
            channel_volume = 1.0f - (vol_attn * (1.0f - (vol_cv / 10.0f)));
          }
          output_voltage *= channel_volume;

          //
          // Attack/Decay envelope
          //
          float envelope = calculateEnvelope(playback_positions[channel], start_position, end_position, attack_value, decay_value);
          output_voltage *= envelope;

          //
          // Smoothing
          //
          if(this->smoothing && (smooth_ramp[channel] < 1))
    			{
    				smooth_ramp[channel] += smooth_rate;
    				output_voltage = (last_output_voltage[channel] * (1 - smooth_ramp[channel])) + (output_voltage * smooth_ramp[channel]);
    			}
          last_output_voltage[channel] = output_voltage;

          // Output the audio (scale from -1.0/+1.0 to -5V/+5V for VCV Rack)
          float output_scaled = output_voltage * 5.0f;
    			outputs[POLY_WAV_OUTPUT].setVoltage(output_scaled, channel);

          if(channel == 0) outputs[LEFT_WAV_OUTPUT].setVoltage(output_scaled);
          if(channel == 1) outputs[RIGHT_WAV_OUTPUT].setVoltage(output_scaled);

          // Copy the left output to the right output if there's only 1 channel
          if(selected_sample->number_of_channels == 1) outputs[RIGHT_WAV_OUTPUT].setVoltage(output_scaled);

          // Increment sample offset (pitch)
    			if (inputs[PITCH_INPUT].isConnected())
    			{
            float pitch_attn = params[PITCH_ATTN_KNOB].getValue();
            float channel_pitch;

            if(channel < (unsigned int) inputs[PITCH_INPUT].getChannels())
            {
              channel_pitch = inputs[PITCH_INPUT].getVoltage(channel);
            }
            else
            {
              channel_pitch = inputs[PITCH_INPUT].getVoltage(0);
            }
            // Apply attenuverter (0 = no pitch CV effect, 1 = full pitch CV effect)
            // Pitch CV is bipolar around 5V center, so we scale the deviation from center
            float pitch_offset = ((channel_pitch / 10.0f) - 0.5f) * pitch_attn;
    				playback_positions[channel] += (selected_sample->sample_rate / args.sampleRate) + pitch_offset;
    			}
    			else
    			{
    				playback_positions[channel] += (selected_sample->sample_rate / args.sampleRate);
    			}

          // EOC division trigger: check if we've crossed into a new division
          // Triggers EOC at each division point (e.g., at division=2: 50% and 100%)
          if (division_size > 0 && channel == 0)  // Only check first channel to avoid multiple triggers
          {
            int current_division = (int)((playback_positions[channel] - start_position) / division_size);
            current_division = clamp(current_division, 0, num_divisions - 1);
            if (current_division != last_division[channel] && last_division[channel] >= 0)
            {
              eoc_pulse.trigger(1e-3f);
            }
            last_division[channel] = current_division;
          }
      }
      // This block of code gets run if the sample is loading, or there's no
      // sample data, or most importantly, if the sample playback had ended
      // and loop == false
      else
      {
        // Trigger EOC when playback ends (non-loop mode) - this catches the final division
        if (playback[channel] && channel == 0)
        {
          eoc_pulse.trigger(1e-3f);
        }
        playback[channel] = false; // Cancel current trigger
        outputs[POLY_WAV_OUTPUT].setVoltage(0, channel);
        last_division[channel] = -1;  // Reset division tracking
      }
    } // end of foreach channel

    outputs[POLY_WAV_OUTPUT].setChannels(selected_sample->number_of_channels);

    // Process EOC trigger output
    outputs[EOC_OUTPUT].setVoltage(eoc_pulse.process(args.sampleTime) ? 10.0f : 0.0f);

  } // end of process loop


};
