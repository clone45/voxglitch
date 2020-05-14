struct GrainEngineMK2 : Module
{
  // Various internal variables
  float pitch = 0;
  float smooth_rate = 0;
  unsigned int spawn_throttling_countdown = 0;
  float max_grains = 0;
  unsigned int selected_waveform = 0;
	std::string loaded_filename = "[ EMPTY ]";
	std::string root_dir;

  // Structs
  Sample sample;
  Common common;
  // SimpleTableOsc internal_modulation_oscillator;
  GrainEngineMK2Core grain_engine_mk2_core;

  // Triggers
  dsp::SchmittTrigger spawn_trigger;

  enum ParamIds {
    WINDOW_KNOB,
    WINDOW_ATTN_KNOB,
    SAMPLE_PLAYBACK_POSITION_KNOB,
    SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
    PITCH_KNOB,
    PITCH_ATTN_KNOB,
    TRIM_KNOB,
    JITTER_KNOB,
    PAN_SWITCH,
    GRAINS_KNOB,
    GRAINS_ATTN_KNOB,
    SPAWN_THROTTLING_KNOB,
    SPAWN_KNOB,
    SPAWN_ATTN_KNOB,
    NUM_PARAMS
  };
  enum InputIds {
    JITTER_CV_INPUT,
    WINDOW_INPUT,
    SAMPLE_PLAYBACK_POSITION_INPUT,
    PITCH_INPUT,
    SPAWN_TRIGGER_INPUT,
    CONTOUR_INPUT,
    PAN_INPUT,
    GRAINS_INPUT,
    SPAWN_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    INTERNAL_MODULATION_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    PURGE_LIGHT,
    SPAWN_INDICATOR_LIGHT,
    EXT_CLK_INDICATOR_LIGHT,
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  GrainEngineMK2()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(WINDOW_KNOB, 0.0f, 1.0f, 1.0f, "WindowKnob");
    configParam(WINDOW_ATTN_KNOB, 0.0f, 1.0f, 0.00f, "WindowAttnKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
    configParam(PITCH_KNOB, -1.3f, 2.0f, 0.0f, "PitchKnob");
    configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 0.20f, "PitchAttnKnob");
    configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
    configParam(JITTER_KNOB, 0.f, 1.0f, 0.0f, "JitterKnob");
    configParam(PAN_SWITCH, 0.0f, 1.0f, 0.0f, "PanSwitch");
    configParam(GRAINS_KNOB, 0.0f, 1.0f, 0.05f, "GrainsKnob");
    configParam(GRAINS_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "GrainsAttnKnob");
    configParam(SPAWN_KNOB, 0.0f, 1.0f, 0.7f, "SpawnKnob");
    configParam(SPAWN_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SpawnAttnKnob");

    grain_engine_mk2_core.common = &common;
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();
		return root;
  }

  void dataFromJson(json_t *root) override
  {
  }

  float calculate_inputs(int input_index, int knob_index, int attenuator_index, float low_range, float high_range)
  {
    float output;
    float attenuator_value = params[attenuator_index].getValue();
    float knob_value = rescale(params[knob_index].getValue(), 0.0, 1.0, low_range, high_range);

    if(inputs[input_index].isConnected())
    {
      float input_value = clamp(rescale(inputs[input_index].getVoltage(), -10.0, 10.0, low_range, high_range), low_range, high_range);
      output = clamp((input_value * attenuator_value) + knob_value, low_range, high_range);
    }
    else
    {
      output = clamp(knob_value, low_range, high_range);
    }

    return(output);
  }

  float calculate_inputs(int input_index, int knob_index, int attenuator_index, float high_range)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0;
    float knob_value = params[knob_index].getValue();
    float attenuator_value = params[attenuator_index].getValue();
    float output;

    if(inputs[input_index].isConnected())
    {
      input_value = clamp(input_value, 0.0, 1.0);
      output = clamp(((input_value * high_range) * attenuator_value) + (knob_value * high_range), 0.0, high_range);
    }
    else
    {
      output = clamp((knob_value * high_range), 0.0, high_range);
    }
    return(output);
  }

  float calculate_inputs(int input_index, int knob_index, int attenuator_index)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0;
    float knob_value = params[knob_index].getValue();
    float attenuator_value = params[attenuator_index].getValue();

    input_value = clamp(input_value, 0.0, 1.0);
    return((input_value * attenuator_value) + knob_value);
  }

/*
  float process_internal_LFO_position_modulation(float modulation_amplitude)
  {
    // add range knobs for these?
    float frequency = calculate_inputs(INTERNAL_MODULATION_FREQUENCY_INPUT, INTERNAL_MODULATION_FREQUENCY_KNOB, INTERNAL_MODULATION_FREQUENCY_ATTN_KNOB, 0.0, 6.0);
    // frequency = rescale(frequency, 0.0, 1.0, 0.0, 6.0);
    // frequency = frequency * frequency;  // give a nice slope
    // common.DEBUG_FLOAT(frequency);

    internal_modulation_oscillator.setFrequency(frequency);

    return(internal_modulation_oscillator.next() * modulation_amplitude);
  }
*/

  void process(const ProcessArgs &args) override
  {
    // Process Max Grains knob
    this->max_grains = calculate_inputs(GRAINS_INPUT, GRAINS_KNOB, GRAINS_ATTN_KNOB, MAX_GRAINS);

    unsigned int contour_index = 0;

    // Process window (width of the grains) inputs
    float window_knob_value = calculate_inputs(WINDOW_INPUT, WINDOW_KNOB, WINDOW_ATTN_KNOB, 1.0, 6400.0);

    // unsigned int window_length = args.sampleRate / window_knob_value;
    unsigned int window_length = window_knob_value;

    float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0, 1.0);

    // At this point, start_position must be and should be between 0.0 and 1.0

    //
    // Process Jitter input
    //

    float jitter_spread = 0;

    if(inputs[JITTER_CV_INPUT].isConnected())
    {
      jitter_spread = params[JITTER_KNOB].getValue() * MAX_JITTER_SPREAD * inputs[JITTER_CV_INPUT].getVoltage();
    }
    else
    {
      jitter_spread = params[JITTER_KNOB].getValue() * MAX_JITTER_SPREAD;
    }

    // If jitter_spread is 124, then the jitter will be between -124 and 124.
    float jitter = common.randomFloat(-1 * jitter_spread, jitter_spread);

    // Make some room at the beginning and end of the possible range position to
    // allow for the addition of the jitter without pushing the start_position out of
    // range of the buffer size.  Also leave room for the window length so that
    // none of the grains reaches the end of the buffer.
    start_position = common.rescaleWithPadding(start_position, 0.0, 1.0, 0.0, sample.total_sample_count, jitter_spread, jitter_spread + window_length);
    start_position += jitter;

    //
    // Process Pan input
    //

    float pan = 0;
    if(inputs[PAN_INPUT].isConnected())
    {
      if(params[PAN_SWITCH].getValue() == 1) // unipolar
      {
        // Incoming pan signal is unipolar.  Convert it to bipolar.
        pan = (inputs[PAN_INPUT].getVoltage() / 5.0) - 1;
      }
      else // bipolar
      {
        // Incoming pan signal is bipolar.  No conversion necessary.
        pan = (inputs[PAN_INPUT].getVoltage() / 10.0);
      }
    }


    if(inputs[PITCH_INPUT].isConnected())
    {
      // This assumes a unipolar input.  Is that correct?
      pitch = (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 5.0f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
    }
    else
    {
      pitch = params[PITCH_KNOB].getValue();
    }

    // If there's a cable connected to the spawn trigger input, it takes priority
    // over the internal spwn rate.
    if(inputs[SPAWN_TRIGGER_INPUT].isConnected())
    {
      if(spawn_trigger.process(inputs[SPAWN_TRIGGER_INPUT].getVoltage())) grain_engine_mk2_core.add(start_position, window_length, pan, &sample, max_grains, pitch);

      lights[SPAWN_INDICATOR_LIGHT].setBrightness(0);
      lights[EXT_CLK_INDICATOR_LIGHT].setBrightness(1);
    }
    else if(spawn_throttling_countdown == 0)
    {
      grain_engine_mk2_core.add(start_position, window_length, pan, &sample, max_grains, pitch);

      float spawn_inputs_value = rescale(calculate_inputs(SPAWN_INPUT, SPAWN_KNOB, SPAWN_ATTN_KNOB, 1.0), 1.f, 0.f, 1.f, 512.f);
      if (spawn_inputs_value < 0) spawn_inputs_value = 0;
      spawn_throttling_countdown = spawn_inputs_value;

      lights[SPAWN_INDICATOR_LIGHT].setBrightness(1);
      lights[EXT_CLK_INDICATOR_LIGHT].setBrightness(0);
    }

    if (! grain_engine_mk2_core.isEmpty())
    {
      smooth_rate = 128.0f / args.sampleRate;

      // Get the output and increase the age of each grain
      std::pair<float, float> stereo_output = grain_engine_mk2_core.process(smooth_rate, contour_index);
      float left_mix_output = stereo_output.first * params[TRIM_KNOB].getValue();
      float right_mix_output = stereo_output.second * params[TRIM_KNOB].getValue();

      // Send audio to outputs
      outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_mix_output);
      outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_mix_output);
    }

    if(spawn_throttling_countdown > 0) spawn_throttling_countdown--;

  }

};
