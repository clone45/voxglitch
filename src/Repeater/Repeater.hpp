struct Repeater : Module
{
	unsigned int selected_sample_slot = 0;
	float samplePos = 0;
	int step = 0;
	bool isPlaying = false;
	SmoothSubModule smooth;
	int retrigger;
	std::string root_dir;

	// When this flag is flase, the display area on the front panel will
	// display something like, "Right-click to Load.."
	bool any_sample_has_been_loaded = false;

	// When this flag is flase, the display area will yell at the user to
	// provide a clock signal.
	bool trig_input_is_connected = false;

	Sample samples[NUMBER_OF_SAMPLES];
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

	dsp::SchmittTrigger playTrigger;
	dsp::PulseGenerator triggerOutputPulse;

	enum ParamIds {
		CLOCK_DIVISION_KNOB,
		CLOCK_DIVISION_ATTN_KNOB,
		POSITION_KNOB,
		POSITION_ATTN_KNOB,
		SAMPLE_SELECT_KNOB,
		SAMPLE_SELECT_ATTN_KNOB,
		PITCH_KNOB,
		PITCH_ATTN_KNOB,
		SMOOTH_SWITCH,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_INPUT,
		POSITION_INPUT,
		CLOCK_DIVISION_INPUT,
		SAMPLE_SELECT_INPUT,
		PITCH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		WAV_OUTPUT,
		TRG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	//
	// Constructor
	//
	Repeater()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PITCH_KNOB, -1.0f, 1.0f, 0.0f, "PitchKnob");
		configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PitchAttnKnob");
		configParam(CLOCK_DIVISION_KNOB, 0.0f, 1.0f, 0.0f, "ClockDivisionKnob");
		configParam(CLOCK_DIVISION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "ClockDivisionAttnKnob");
		configParam(POSITION_KNOB, 0.0f, 1.0f, 0.0f, "PositionKnob");
		configParam(POSITION_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "PositionAttnKnob");
		configParam(SAMPLE_SELECT_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
		configParam(SAMPLE_SELECT_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");
		configParam(SMOOTH_SWITCH, 0.f, 1.f, 1.f, "Smooth");

		std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");
	}

	// Autosave module data.  VCV Rack decides when this should be called.
	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(rootJ, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}

		json_object_set_new(rootJ, "retrigger", json_integer(retrigger));
		return rootJ;
	}

	// Load module data
	void dataFromJson(json_t *rootJ) override
	{
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				samples[i].load(json_string_value(loaded_sample_path));
				loaded_filenames[i] = samples[i].filename;
				this->any_sample_has_been_loaded = true;
			}

			json_t* retrigger_json = json_object_get(rootJ, "retrigger");
			if (retrigger_json) retrigger = json_integer_value(retrigger_json);
		}
	}

	// TODO: Eventually share this code instead of duplicating it
	float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
	{
		float input_value = inputs[input_index].getVoltage() / 10.0;
		float knob_value = params[knob_index].getValue();
		float attenuator_value = params[attenuator_index].getValue();

		return(((input_value * scale) * attenuator_value) + (knob_value * scale));
	}

	//
	// Main module processing loop.  This runs at whatever samplerate is selected within VCV Rack.
	//

	void process(const ProcessArgs &args) override
	{
		bool trigger_output_pulse = false;

		unsigned int sample_select_input_value = calculate_inputs(SAMPLE_SELECT_INPUT, SAMPLE_SELECT_KNOB, SAMPLE_SELECT_ATTN_KNOB, NUMBER_OF_SAMPLES_FLOAT);
		sample_select_input_value = clamp(sample_select_input_value, 0, NUMBER_OF_SAMPLES - 1);

		if(sample_select_input_value != selected_sample_slot)
		{
			// Start smoothing if the selected sample has changed
			smooth.trigger();

			// Set the selected sample
			selected_sample_slot = sample_select_input_value;
		}

		// This is just for convenience
		Sample *selected_sample = &samples[selected_sample_slot];

		if (inputs[TRIG_INPUT].isConnected())
		{
			trig_input_is_connected = true;

			//
			// playTrigger is a SchmittTrigger object.  Here, the SchmittTrigger
			// determines if a low-to-high transition happened at the TRIG_INPUT
			//
			if (playTrigger.process(inputs[TRIG_INPUT].getVoltage()))
			{
				float clock_division = calculate_inputs(CLOCK_DIVISION_INPUT, CLOCK_DIVISION_KNOB, CLOCK_DIVISION_ATTN_KNOB, 10);
				clock_division = clamp(clock_division, 0.0, 10.0);

				step += 1;
				if(step > clock_division) step = 0;

				if(step == 0)
				{
					isPlaying = true;
					samplePos = calculate_inputs(POSITION_INPUT, POSITION_KNOB, POSITION_ATTN_KNOB, selected_sample->total_sample_count);
					smooth.trigger();
					triggerOutputPulse.trigger(0.01f);
				}
			}
		}
		else
		{
			trig_input_is_connected = false;
		}

		// If the sample has completed playing and the retrigger option is true,
		// then restart sample playback.

		if(retrigger && abs(floor(samplePos)) >= selected_sample->total_sample_count)
		{
			samplePos = 0;
			smooth.trigger();
		}

		if (isPlaying && (! selected_sample->loading) && (selected_sample->loaded) && (selected_sample->total_sample_count > 0) && ((abs(floor(samplePos)) < selected_sample->total_sample_count)))
		{
			float wav_output_voltage;

			if (samplePos >= 0)
			{
				wav_output_voltage = GAIN  * selected_sample->leftPlayBuffer[floor(samplePos)];
			}
			else
			{
				// What is this for?  Does it play the sample in reverse?  I think so.
				wav_output_voltage = GAIN * selected_sample->leftPlayBuffer[floor(selected_sample->total_sample_count - 1 + samplePos)];
			}

			// Apply smoothing
			if(params[SMOOTH_SWITCH].getValue()) wav_output_voltage = smooth.process(wav_output_voltage, (128.0f / args.sampleRate));

			// Output voltage
			outputs[WAV_OUTPUT].setVoltage(wav_output_voltage);

			// Increment sample offset (pitch)
			if (inputs[PITCH_INPUT].isConnected())
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
			}
			else
			{
				samplePos = samplePos + (selected_sample->sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
			}
		}
		else
		{
			isPlaying = false;
			outputs[WAV_OUTPUT].setVoltage(0);
		}

		trigger_output_pulse = triggerOutputPulse.process(1.0 / args.sampleRate);
		outputs[TRG_OUTPUT].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
	}
};
