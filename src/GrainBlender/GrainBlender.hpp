// Where I left off
//
// I've been copying elements from Goblins into Grain Blender to support multiple
// samples.  The most recent of these updates is the load/save stuff.
// I still need to update all the code that uses the single sample to start
// using the array of samples.

struct GrainBlender : Module
{
  float spawn_rate_counter = 0;
  float step_amount = 0;
  float smooth_rate = 0;
	unsigned int selected_sample_slot = 0;

  AudioBuffer audio_buffer;

  int step = 0;
  std::string root_dir;
  std::string path;

  GrainBlenderEx grain_blender_core;

  dsp::SchmittTrigger purge_trigger;
  dsp::SchmittTrigger purge_button_trigger;
  dsp::SchmittTrigger spawn_trigger;

  float jitter_divisor = 1;

  enum ParamIds {
    LENGTH_KNOB,
    LENGTH_ATTN_KNOB,
    SAMPLE_PLAYBACK_POSITION_KNOB,
    SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
    PITCH_KNOB,
    PITCH_ATTN_KNOB,
    TRIM_KNOB,
    JITTER_KNOB,
    PAN_SWITCH,
    FREEZE_SWITCH,
    NUM_PARAMS
  };
  enum InputIds {
    JITTER_CV_INPUT,
    LENGTH_INPUT,
    SAMPLE_PLAYBACK_POSITION_INPUT,
    PITCH_INPUT,
    SPAWN_TRIGGER_INPUT,
    AMP_SLOPE_INPUT,
    PAN_INPUT,
    FREEZE_INPUT,
    AUDIO_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    DEBUG_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    PURGE_LIGHT,
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  GrainBlender()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "LengthKnob");
    configParam(LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "LengthAttnKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
    configParam(PITCH_KNOB, -1.3f, 2.0f, 0.0f, "PitchKnob");
    configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 0.20f, "PitchAttnKnob");
    configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
    configParam(JITTER_KNOB, 0.f, 1.0f, 0.0f, "JitterKnob");
    configParam(PAN_SWITCH, 0.0f, 1.0f, 0.0f, "PanSwitch");
    configParam(FREEZE_SWITCH, 0.0f, 1.0f, 0.0f, "FreezeSwitch");

    jitter_divisor = static_cast <float> (RAND_MAX / 1024.0);
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();
		return root;
  }

  void dataFromJson(json_t *root) override
  {
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
    unsigned int max_window = args.sampleRate / 6;
    float audio = inputs[AUDIO_INPUT].getVoltage();
    audio_buffer.push(audio, audio);

    float window = calculate_inputs(LENGTH_INPUT, LENGTH_KNOB, LENGTH_ATTN_KNOB, max_window);
    float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, MAX_BUFFER_SIZE);

    start_position = clamp(start_position, 0.01f, (float)(MAX_BUFFER_SIZE - 1));

    // Ensure that the inputs are within range
    // if(start_position >= (MAX_BUFFER_SIZE - max_window)) start_position = MAX_BUFFER_SIZE - max_window;

    // Shorten the playback length if it would result in playback passing the end of the sample data.
    // if(playback_length > (MAX_BUFFER_SIZE - start_position)) playback_length = MAX_BUFFER_SIZE - start_position;

    //
    // Process Jitter input
    //

    if(inputs[JITTER_CV_INPUT].isConnected())
    {
      float spread = params[JITTER_KNOB].getValue() * 64.0 * inputs[JITTER_CV_INPUT].getVoltage();
      float r = (static_cast <float> (rand()) / (RAND_MAX / spread)) - spread;
      start_position = start_position + r;
    }
    else
    {
      float spread = params[JITTER_KNOB].getValue() * 64.0;
      float r = (static_cast <float> (rand()) / (RAND_MAX / spread)) - spread;
      start_position = start_position + r;
    }

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

    // Process freeze input
    if(inputs[FREEZE_INPUT].isConnected())
    {
      audio_buffer.frozen = inputs[FREEZE_INPUT].getVoltage();
    }
    else
    {
      audio_buffer.frozen = params[FREEZE_SWITCH].getValue();
    }

    if(spawn_trigger.process(inputs[SPAWN_TRIGGER_INPUT].getVoltage()))
    {
      grain_blender_core.add(start_position, window, pan, &audio_buffer);
    }

    if (! grain_blender_core.isEmpty())
    {
      if(inputs[PITCH_INPUT].isConnected())
      {
        step_amount = (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 5.0f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
      }
      else
      {
        step_amount = params[PITCH_KNOB].getValue();
      }

      smooth_rate = 128.0f / args.sampleRate;

      // Get the output from the graveyard and increase the age of each ghost
      std::pair<float, float> stereo_output = grain_blender_core.process(smooth_rate, step_amount);
      float left_mix_output = stereo_output.first * params[TRIM_KNOB].getValue();
      float right_mix_output = stereo_output.second  * params[TRIM_KNOB].getValue();

      // Send audio to outputs
      outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_mix_output);
      outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_mix_output);
    }
  }
};
