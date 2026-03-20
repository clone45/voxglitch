//
// OneShot
//
// Sample playback module with proportional envelope system.
// Attack and decay times are expressed as percentages of the
// pitch-adjusted playback duration.
//

struct OneShot : VoxglitchSamplerModule
{
	unsigned int selected_sample_slot = 0;
	double playback_position = 0.0;
	bool playing = false;
	float sample_time = 0;
	float smooth_ramp = 1.0f;
	float last_output_voltage_l = 0.0f;
	float last_output_voltage_r = 0.0f;
	std::string rootDir;
	std::string path;

	dsp::SchmittTrigger trigger_input_trigger;
	dsp::SchmittTrigger trigger_button_trigger;
	dsp::SchmittTrigger retrigger_input_trigger;
	dsp::SchmittTrigger retrigger_button_trigger;
	dsp::SchmittTrigger reset_input_trigger;
	dsp::SchmittTrigger reset_button_trigger;

	dsp::PulseGenerator eoc_pulse;

	unsigned int number_of_samples = 0;
	std::vector<SampleMC> samples;

	// Delay timer (in samples)
	float delay_timer = 0.0f;
	bool armed = false;

	enum ParamIds
	{
		WAV_SELECTOR_PARAM,
		WAV_ATTENUATOR_PARAM,
		TRANSPOSE_PARAM,
		TRANSPOSE_ATTENUATOR_PARAM,
		ATTACK_PARAM,
		ATTACK_ATTENUATOR_PARAM,
		ATTACK_WAVEFORM_PARAM,
		DECAY_PARAM,
		DECAY_ATTENUATOR_PARAM,
		DECAY_WAVEFORM_PARAM,
		LEVEL_PARAM,
		DELAY_PARAM,
		BAL_PARAM,
		TRIGGER_BUTTON,
		RETRIGGER_BUTTON,
		RESET_BUTTON,
		LOOP_SWITCH,
		NUM_PARAMS
	};

	enum InputIds
	{
		WAV_CV_INPUT,
		TRANSPOSE_CV_INPUT,
		ATTACK_CV_INPUT,
		DECAY_CV_INPUT,
		TRIGGER_INPUT,
		RETRIGGER_INPUT,
		LEVEL_CV_INPUT,
		LOOP_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};

	enum OutputIds
	{
		OUTPUT_L,
		OUTPUT_R,
		ENV_OUTPUT,
		EOC_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	OneShot()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		// WAV Selector Section
		configParam(WAV_SELECTOR_PARAM, 0.0f, 1.0f, 0.0f, "Sample Select");
		configParam(WAV_ATTENUATOR_PARAM, 0.0f, 1.0f, 1.0f, "Sample Select CV Attenuator");
		configParam(TRANSPOSE_PARAM, -24.0f, 24.0f, 0.0f, "Transpose (semitones)");
		getParamQuantity(TRANSPOSE_PARAM)->snapEnabled = true;
		configParam(TRANSPOSE_ATTENUATOR_PARAM, 0.0f, 1.0f, 0.0f, "Transpose CV Attenuator");

		// Envelope Section
		configParam(ATTACK_PARAM, 0.0f, 0.5f, 0.0f, "Attack (% of duration)");
		configParam(ATTACK_ATTENUATOR_PARAM, 0.0f, 1.0f, 0.0f, "Attack CV Attenuator");
		configParam(ATTACK_WAVEFORM_PARAM, -1.0f, 1.0f, 0.0f, "Attack Curve Shape");
		configParam(DECAY_PARAM, 0.0f, 0.5f, 0.0f, "Decay (% of duration)");
		configParam(DECAY_ATTENUATOR_PARAM, 0.0f, 1.0f, 0.0f, "Decay CV Attenuator");
		configParam(DECAY_WAVEFORM_PARAM, -1.0f, 1.0f, 0.0f, "Decay Curve Shape");

		// Output Section
		configParam(LEVEL_PARAM, 0.0f, 1.0f, 1.0f, "Output Level");
		configParam(DELAY_PARAM, 0.0f, 1000.0f, 0.0f, "Trigger Delay (ms)");
		configParam(BAL_PARAM, -1.0f, 1.0f, 0.0f, "Stereo Balance");

		// Buttons and Switches
		configParam(TRIGGER_BUTTON, 0.0f, 1.0f, 0.0f, "Trigger");
		configParam(RETRIGGER_BUTTON, 0.0f, 1.0f, 0.0f, "Retrigger");
		configParam(RESET_BUTTON, 0.0f, 1.0f, 0.0f, "Reset");
		configParam(LOOP_SWITCH, 0.0f, 1.0f, 0.0f, "Loop");

		// Configure inputs
		configInput(WAV_CV_INPUT, "Sample Select CV");
		configInput(TRANSPOSE_CV_INPUT, "Transpose CV");
		configInput(ATTACK_CV_INPUT, "Attack CV");
		configInput(DECAY_CV_INPUT, "Decay CV");
		configInput(TRIGGER_INPUT, "Trigger");
		configInput(RETRIGGER_INPUT, "Retrigger");
		configInput(LEVEL_CV_INPUT, "Level CV");
		configInput(LOOP_INPUT, "Loop Gate");
		configInput(RESET_INPUT, "Reset");

		// Configure outputs
		configOutput(OUTPUT_L, "Left Audio");
		configOutput(OUTPUT_R, "Right Audio");
		configOutput(ENV_OUTPUT, "Envelope CV");
		configOutput(EOC_OUTPUT, "End of Cycle Trigger");
	}

	// Save
	json_t *dataToJson() override
	{
		json_t *json_root = json_object();
		json_object_set_new(json_root, "path", json_string(this->path.c_str()));
		return json_root;
	}

	// Load
	void dataFromJson(json_t *json_root) override
	{
		json_t *loaded_path_json = json_object_get(json_root, "path");
		if (loaded_path_json)
		{
			this->path = json_string_value(loaded_path_json);
			this->load_samples_from_path(this->path);
		}
	}

	//
	// Envelope curve shaping
	// t is normalized phase position 0.0 - 1.0
	// shape is the waveform knob value -1.0 to +1.0
	//
	float shapedValue(float t, float shape)
	{
		float linear = t;
		float curved;

		if (shape > 0.0f)
		{
			// Exponential side
			curved = pow(t, 1.0f + shape * 3.0f);
		}
		else
		{
			// Logarithmic side
			curved = 1.0f - pow(1.0f - t, 1.0f - shape * 3.0f);
		}

		float blend = (shape >= 0.0f) ? shape : -shape;
		return rack::crossfade(linear, curved, blend);
	}

	//
	// Calculate envelope value based on current playback position
	//
	float calculateEnvelope(double position, double total_length, float attack_fraction, float decay_fraction, float attack_shape, float decay_shape)
	{
		if (total_length <= 0.0)
		{
			return 1.0f;
		}

		double attack_length = attack_fraction * total_length;
		double decay_length = decay_fraction * total_length;

		// Clamp if attack + decay exceed total length
		if (attack_length + decay_length > total_length)
		{
			double scale = total_length / (attack_length + decay_length);
			attack_length *= scale;
			decay_length *= scale;
		}

		double sustain_start = attack_length;
		double decay_start = total_length - decay_length;

		float envelope = 1.0f;

		if (position < attack_length && attack_length > 0.0)
		{
			// Attack phase
			float t = (float)(position / attack_length);
			envelope = shapedValue(t, attack_shape);
		}
		else if (position >= decay_start && decay_length > 0.0)
		{
			// Decay phase
			float t = (float)((position - decay_start) / decay_length);
			envelope = shapedValue(1.0f - t, decay_shape);
		}
		// else: sustain phase, envelope = 1.0

		return clamp(envelope, 0.0f, 1.0f);
	}

	//
	// Get the transpose amount in semitones (knob + CV)
	//
	float getTransposeSemitones()
	{
		float semitones = params[TRANSPOSE_PARAM].getValue();
		if (inputs[TRANSPOSE_CV_INPUT].isConnected())
		{
			float cv = inputs[TRANSPOSE_CV_INPUT].getVoltage() / 10.0f;
			float attn = params[TRANSPOSE_ATTENUATOR_PARAM].getValue();
			semitones += cv * attn * 48.0f - 24.0f;
		}
		return clamp(semitones, -24.0f, 24.0f);
	}

	//
	// Get the attack fraction (knob + CV)
	//
	float getAttackFraction()
	{
		float attack = params[ATTACK_PARAM].getValue();
		if (inputs[ATTACK_CV_INPUT].isConnected())
		{
			float cv = inputs[ATTACK_CV_INPUT].getVoltage() / 10.0f;
			float attn = params[ATTACK_ATTENUATOR_PARAM].getValue();
			attack = clamp(attack + cv * attn * 0.5f, 0.0f, 0.5f);
		}
		return attack;
	}

	//
	// Get the decay fraction (knob + CV)
	//
	float getDecayFraction()
	{
		float decay = params[DECAY_PARAM].getValue();
		if (inputs[DECAY_CV_INPUT].isConnected())
		{
			float cv = inputs[DECAY_CV_INPUT].getVoltage() / 10.0f;
			float attn = params[DECAY_ATTENUATOR_PARAM].getValue();
			decay = clamp(decay + cv * attn * 0.5f, 0.0f, 0.5f);
		}
		return decay;
	}

	//
	// Get the output level (knob + CV)
	//
	float getLevel()
	{
		float level = params[LEVEL_PARAM].getValue();
		if (inputs[LEVEL_CV_INPUT].isConnected())
		{
			float cv = inputs[LEVEL_CV_INPUT].getVoltage() / 10.0f;
			level = clamp(level + cv * (1.0f - level), 0.0f, 1.0f);
		}
		return level;
	}

	//
	// Get the selected sample index (knob + CV)
	//
	unsigned int getSelectedSampleIndex()
	{
		if (number_of_samples == 0) return 0;

		float knob = params[WAV_SELECTOR_PARAM].getValue();
		float index = knob * (number_of_samples - 1);

		if (inputs[WAV_CV_INPUT].isConnected())
		{
			float cv = inputs[WAV_CV_INPUT].getVoltage() / 10.0f;
			float attn = params[WAV_ATTENUATOR_PARAM].getValue();
			index += cv * attn * (number_of_samples - 1);
		}

		return (unsigned int)clamp((int)roundf(index), 0, (int)(number_of_samples - 1));
	}

	void sort_samples_by_filename(std::vector<SampleMC> &samples)
	{
		std::sort(samples.begin(), samples.end(), [](const SampleMC &a, const SampleMC &b)
		{
			return a.filename < b.filename;
		});
	}

	void load_samples_from_path(std::string path)
	{
		this->samples.clear();
		this->rootDir = path;

		std::vector<std::string> dirList = system::getEntries(path.c_str());
		sort(dirList.begin(), dirList.end());

		for (auto entry : dirList)
		{
			std::string ext = rack::string::lowercase(system::getExtension(entry));
			if (ext == "wav" || ext == ".wav" || ext == "mp3" || ext == ".mp3")
			{
				SampleMC new_sample;
				new_sample.load(entry);
				this->samples.push_back(new_sample);
			}
		}

		sort_samples_by_filename(this->samples);
	}

	void startPlayback()
	{
		playback_position = 0.0;
		playing = true;
		smooth_ramp = 0.0f;
	}

	void stopPlayback()
	{
		playing = false;
		playback_position = 0.0;
		armed = false;
		delay_timer = 0.0f;
	}

	void process(const ProcessArgs &args) override
	{
		number_of_samples = samples.size();
		sample_time = args.sampleTime;
		float smooth_rate = 128.0f / args.sampleRate;

		//
		// Handle trigger, retrigger, and reset inputs
		//

		bool trigger_fired = trigger_input_trigger.process(inputs[TRIGGER_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger)
			|| trigger_button_trigger.process(params[TRIGGER_BUTTON].getValue());

		bool retrigger_fired = retrigger_input_trigger.process(inputs[RETRIGGER_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger)
			|| retrigger_button_trigger.process(params[RETRIGGER_BUTTON].getValue());

		bool reset_fired = reset_input_trigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger)
			|| reset_button_trigger.process(params[RESET_BUTTON].getValue());

		// Reset: stop playback immediately
		if (reset_fired)
		{
			stopPlayback();
		}

		// Retrigger: restart playback immediately (no delay)
		if (retrigger_fired)
		{
			armed = false;
			delay_timer = 0.0f;
			startPlayback();
		}

		// Trigger: start delay timer, then play
		if (trigger_fired)
		{
			float delay_ms = params[DELAY_PARAM].getValue();
			if (delay_ms > 0.0f)
			{
				delay_timer = delay_ms * 0.001f * args.sampleRate;
				armed = true;
			}
			else
			{
				startPlayback();
			}
		}

		// Process delay timer
		if (armed)
		{
			delay_timer -= 1.0f;
			if (delay_timer <= 0.0f)
			{
				armed = false;
				delay_timer = 0.0f;
				startPlayback();
			}
		}

		//
		// Update selected sample (does not interrupt current playback)
		//
		if (number_of_samples > 0)
		{
			selected_sample_slot = getSelectedSampleIndex();
		}

		// Bail out if no samples loaded
		if (selected_sample_slot >= samples.size())
		{
			outputs[OUTPUT_L].setVoltage(0.0f);
			outputs[OUTPUT_R].setVoltage(0.0f);
			outputs[ENV_OUTPUT].setVoltage(0.0f);
			outputs[EOC_OUTPUT].setVoltage(eoc_pulse.process(args.sampleTime) ? 10.0f : 0.0f);
			return;
		}

		SampleMC *selected_sample = &samples[selected_sample_slot];

		// Check if loop is enabled
		bool loop_enabled = (params[LOOP_SWITCH].getValue() > 0.5f);
		if (inputs[LOOP_INPUT].isConnected())
		{
			loop_enabled = (inputs[LOOP_INPUT].getVoltage() >= 1.0f);
		}

		// Calculate pitch/speed
		float semitones = getTransposeSemitones();
		float speed_ratio = pow(2.0f, semitones / 12.0f);
		double base_increment = (double)selected_sample->sample_rate / (double)args.sampleRate;
		double increment = base_increment * (double)speed_ratio;

		// Calculate adjusted duration in samples (at module sample rate)
		double original_duration = (double)selected_sample->size();
		double adjusted_duration = original_duration / (double)speed_ratio;

		// Get envelope parameters
		float attack_fraction = getAttackFraction();
		float decay_fraction = getDecayFraction();
		float attack_shape = params[ATTACK_WAVEFORM_PARAM].getValue();
		float decay_shape = params[DECAY_WAVEFORM_PARAM].getValue();

		// Get output level and balance
		float level = getLevel();
		float bal = params[BAL_PARAM].getValue();
		float left_gain = (bal <= 0.0f) ? 1.0f : 1.0f - bal;
		float right_gain = (bal >= 0.0f) ? 1.0f : 1.0f + bal;

		if (playing && !selected_sample->loading && selected_sample->loaded && selected_sample->size() > 0)
		{
			// Check if playback has reached the end
			if (playback_position >= (double)selected_sample->size())
			{
				eoc_pulse.trigger(1e-3f);

				if (loop_enabled)
				{
					double overshoot = playback_position - (double)selected_sample->size();
					playback_position = fmod(overshoot, (double)selected_sample->size());
					smooth_ramp = 0.0f;
				}
				else
				{
					playing = false;
					playback_position = 0.0;
					outputs[OUTPUT_L].setVoltage(0.0f);
					outputs[OUTPUT_R].setVoltage(0.0f);
					outputs[ENV_OUTPUT].setVoltage(0.0f);
					outputs[EOC_OUTPUT].setVoltage(eoc_pulse.process(args.sampleTime) ? 10.0f : 0.0f);
					return;
				}
			}

			// Calculate envelope based on position within adjusted duration
			// The playback_position is in original sample frames, so we need
			// to convert to the proportional position within adjusted duration
			double proportional_position = playback_position / speed_ratio;
			float envelope = calculateEnvelope(proportional_position, adjusted_duration, attack_fraction, decay_fraction, attack_shape, decay_shape);

			// Read sample data
			float output_l = selected_sample->read(0, (int)playback_position);
			float output_r;

			if (selected_sample->number_of_channels > 1)
			{
				output_r = selected_sample->read(1, (int)playback_position);
			}
			else
			{
				output_r = output_l;
			}

			// Apply envelope
			output_l *= envelope;
			output_r *= envelope;

			// Apply level
			output_l *= level;
			output_r *= level;

			// Smoothing
			if (smooth_ramp < 1.0f)
			{
				smooth_ramp += smooth_rate;
				output_l = (last_output_voltage_l * (1.0f - smooth_ramp)) + (output_l * smooth_ramp);
				output_r = (last_output_voltage_r * (1.0f - smooth_ramp)) + (output_r * smooth_ramp);
			}
			last_output_voltage_l = output_l;
			last_output_voltage_r = output_r;

			// Apply balance and scale to VCV Rack voltage range
			outputs[OUTPUT_L].setVoltage(output_l * left_gain * GAIN);
			outputs[OUTPUT_R].setVoltage(output_r * right_gain * GAIN);
			outputs[ENV_OUTPUT].setVoltage(envelope * 10.0f);

			// Advance playback position
			playback_position += increment;
		}
		else
		{
			// Not playing
			outputs[OUTPUT_L].setVoltage(0.0f);
			outputs[OUTPUT_R].setVoltage(0.0f);
			outputs[ENV_OUTPUT].setVoltage(0.0f);
		}

		// Process EOC trigger output
		outputs[EOC_OUTPUT].setVoltage(eoc_pulse.process(args.sampleTime) ? 10.0f : 0.0f);
	}
};
