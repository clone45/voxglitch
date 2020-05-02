// Where I left off
//
// I've been copying elements from Goblins into Grain Blender to support multiple
// samples.  The most recent of these updates is the load/save stuff.
// I still need to update all the code that uses the single sample to start
// using the array of samples.

struct GrainBlender : Module
{
  float spawn_rate_counter = 0;
  float pitch = 0;
  float smooth_rate = 0;
  float max_window_divisor = 4.0;
	unsigned int selected_sample_slot = 0;
  unsigned int spawn_throttling_countdown = 0;
  unsigned int max_grains = MAX_GRAINS;

  AudioBuffer audio_buffer;
  SimpleTableOsc internal_modulation_oscillator;

  int step = 0;
  std::string root_dir;
  std::string path;
  bool is_spawn_cable_connected = false;

  GrainBlenderEx grain_blender_core;

  dsp::SchmittTrigger purge_trigger;
  dsp::SchmittTrigger purge_button_trigger;
  dsp::SchmittTrigger spawn_trigger;

  float jitter_divisor = 1;

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
    FREEZE_SWITCH,
    MAX_GRAINS_KNOB,
    SPAWN_THROTTLING_KNOB,
    CONTOUR_KNOB,
    SPAWN_KNOB,
    SPAWN_ATTN_KNOB,
    INTERNAL_MODULATION_FREQUENCY_KNOB,
    INTERNAL_MODULATION_AMPLITUDE_KNOB,
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
    FREEZE_INPUT,
    AUDIO_INPUT_LEFT,
    AUDIO_INPUT_RIGHT,
    SPAWN_INPUT,
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
    configParam(WINDOW_KNOB, WINDOW_KNOB_MIN, WINDOW_KNOB_MAX, WINDOW_KNOB_DEFAULT, "WindowKnob");
    configParam(WINDOW_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "WindowAttnKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
    configParam(PITCH_KNOB, -1.3f, 2.0f, 0.0f, "PitchKnob");
    configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 0.20f, "PitchAttnKnob");
    configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
    configParam(JITTER_KNOB, 0.f, 1.0f, 0.0f, "JitterKnob");
    configParam(PAN_SWITCH, 0.0f, 1.0f, 0.0f, "PanSwitch");
    configParam(FREEZE_SWITCH, 0.0f, 1.0f, 0.0f, "FreezeSwitch");
    configParam(MAX_GRAINS_KNOB, 0.0f, MAX_GRAINS, 100.0f, "MaxGrains");
    configParam(CONTOUR_KNOB, 0.0f, 1.0f, 0.0f, "ContourKnob");
    configParam(SPAWN_KNOB, 0.0f, 1.0f, 1.0f, "SpawnKnob");
    configParam(SPAWN_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SpawnAttnKnob");
    configParam(INTERNAL_MODULATION_FREQUENCY_KNOB, 0.1f, 1.0f, 0.5f, "InternalModulateionFrequencyKnob");
    configParam(INTERNAL_MODULATION_AMPLITUDE_KNOB, 0.1f, 1.0f, 0.5f, "InternalModulateionAmplitudeKnob");

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

  float calculate_inputs(int input_index, int knob_index, int attenuator_index)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0;
    float knob_value = params[knob_index].getValue();
    float attenuator_value = params[attenuator_index].getValue();

    return((input_value * attenuator_value) + knob_value);
  }

  void process(const ProcessArgs &args) override
  {
    audio_buffer.push(inputs[AUDIO_INPUT_LEFT].getVoltage(), inputs[AUDIO_INPUT_RIGHT].getVoltage());

    // Process max_window input
    // max_window_divisor = params[MAX_WINDOW_KNOB].getValue();

    // Process Max Grains knob
    this->max_grains = params[MAX_GRAINS_KNOB].getValue();

    unsigned int contour_index = params[CONTOUR_KNOB].getValue() * 9.0;


    // float window_knob_value = calculate_inputs(WINDOW_INPUT, WINDOW_KNOB, WINDOW_ATTN_KNOB, WINDOW_KNOB_MAX);

    float window_knob_value = params[WINDOW_KNOB].getValue() + 60.0;

    // DEBUG("window knob value");
    // DEBUG(std::to_string(window_knob_value).c_str());

    // unsigned int window_length = args.sampleRate / window_knob_value;
    unsigned int window_length = window_knob_value;

    float start_position;

    if(inputs[SAMPLE_PLAYBACK_POSITION_INPUT].isConnected())
    {
      start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, MAX_BUFFER_SIZE);
    }
    else
    {

      internal_modulation_oscillator.setFrequency((params[INTERNAL_MODULATION_FREQUENCY_KNOB].getValue() * 500.0) + 0.10);
      start_position = internal_modulation_oscillator.next() * ((float) MAX_BUFFER_SIZE / 4.0) * params[INTERNAL_MODULATION_AMPLITUDE_KNOB].getValue();
    }

    //
    // Process Jitter input
    //

    /*
    float jitter = 0;
    float jitter_spread = 0;

    if(inputs[JITTER_CV_INPUT].isConnected())
    {
      jitter_spread = params[JITTER_KNOB].getValue() * JITTER_SPREAD * inputs[JITTER_CV_INPUT].getVoltage();
    }
    else
    {
      jitter_spread = params[JITTER_KNOB].getValue() * JITTER_SPREAD;
    }

    jitter = fmod(rand(), jitter_spread) - jitter_spread;


    // DEBUG(std::to_string(window_knob_value).c_str());

    // Ensure that the inputs are within range
    start_position += jitter;

    // start_position = fmod(start_position, MAX_BUFFER_SIZE - window_length);


    if((start_position + jitter) >= (MAX_BUFFER_SIZE - window_length))
    {
      start_position = (start_position + jitter) - (MAX_BUFFER_SIZE + window_length);
    }
    else
    {
      start_position += jitter;
    }
    */

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

    // is_spawn_cable_connected = inputs[SPAWN_TRIGGER_INPUT].isConnected() ? true : false;

    // if(spawn_trigger.process(inputs[SPAWN_TRIGGER_INPUT].getVoltage()) || is_spawn_cable_connected == false)
    // {
      if(spawn_throttling_countdown == 0)
      {
        if(inputs[PITCH_INPUT].isConnected())
        {
          pitch = (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 5.0f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
        }
        else
        {
          pitch = params[PITCH_KNOB].getValue();
        }

        grain_blender_core.add(start_position, window_length, pan, &audio_buffer, max_grains, pitch);

        float spawn_inputs_value = rescale(calculate_inputs(SPAWN_INPUT, SPAWN_KNOB, SPAWN_ATTN_KNOB, 1.0), 0.f, 1.f, 1.f, 512.f);
        if (spawn_inputs_value < 0) spawn_inputs_value = 0;
        spawn_throttling_countdown = spawn_inputs_value;
      }

    // }

    if (! grain_blender_core.isEmpty())
    {
      smooth_rate = 128.0f / args.sampleRate;

      // Get the output and increase the age of each grain
      std::pair<float, float> stereo_output = grain_blender_core.process(smooth_rate, contour_index);
      float left_mix_output = stereo_output.first * params[TRIM_KNOB].getValue();
      float right_mix_output = stereo_output.second * params[TRIM_KNOB].getValue();

      // Send audio to outputs
      outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_mix_output);
      outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_mix_output);
    }

    if(spawn_throttling_countdown > 0) spawn_throttling_countdown--;
  }
};
