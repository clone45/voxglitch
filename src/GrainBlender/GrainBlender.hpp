// Where I left off
//
// I've been copying elements from Goblins into Grain Blender to support multiple
// samples.  The most recent of these updates is the load/save stuff.
// I still need to update all the code that uses the single sample to start
// using the array of samples.

struct GrainBlender : Module
{
	std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

  float spawn_rate_counter = 0;
  float step_amount = 0;
  float smooth_rate = 0;
	unsigned int selected_sample_slot = 0;

  int step = 0;
  std::string root_dir;
  std::string path;

  GrainBlenderEx grain_blender_core;

  Sample samples[NUMBER_OF_SAMPLES];

  dsp::SchmittTrigger purge_trigger;
  dsp::SchmittTrigger purge_button_trigger;
  dsp::SchmittTrigger spawn_trigger;

  float jitter_divisor = 1;

  // The filename of the loaded sample.  This is used to display the currently
  // loaded sample in the right-click context menu.
  std::string loaded_filename = "[ EMPTY ]";

  enum ParamIds {
    LENGTH_KNOB,
    LENGTH_ATTN_KNOB,
    SAMPLE_PLAYBACK_POSITION_KNOB,
    SAMPLE_PLAYBACK_POSITION_ATTN_KNOB,
    PITCH_KNOB,
    PITCH_ATTN_KNOB,
    X_KNOB,
    X_ATTN_KNOB,
    Y_KNOB,
    Y_ATTN_KNOB,
    TRIM_KNOB,
    JITTER_KNOB,
    LEN_MULT_KNOB,
    PAN_SWITCH,
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
    configParam(LENGTH_KNOB, 0.0f, 1.0f, 0.5f, "GhostLengthKnob");
    configParam(LENGTH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "GhostLengthAttnKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionKnob");
    configParam(SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SamplePlaybackPositionAttnKnob");
    configParam(PITCH_KNOB, -0.3f, 1.0f, 0.0f, "PitchKnob");
    configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "PitchAttnKnob");
    configParam(X_KNOB, 0.0f, 1.0f, 0.0f, "XKnob");
    configParam(X_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "XAttnKnob");
    configParam(Y_KNOB, 0.0f, 1.0f, 0.0f, "YKnob");
    configParam(Y_ATTN_KNOB, 0.0f, 1.0f, 1.00f, "YAttnKnob");
    configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
    configParam(LEN_MULT_KNOB, 1.0f, 128.0f, 1.0f, "LenMultKnob");
    configParam(JITTER_KNOB, 0.f, 1.0f, 0.0f, "JitterKnob");
    configParam(PAN_SWITCH, 0.0f, 1.0f, 0.0f, "PanSwitch");

    jitter_divisor = static_cast <float> (RAND_MAX / 1024.0);
  }

  json_t *dataToJson() override
  {
    //json_t *rootJ = json_object();
    //json_object_set_new(rootJ, "path", json_string(sample.path.c_str()));
    //return rootJ;

    json_t *root = json_object();
		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(root, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i].path.c_str()));
		}
		return root;
  }

  void dataFromJson(json_t *root) override
  {
    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(root, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				samples[i].load(json_string_value(loaded_sample_path), false);
				loaded_filenames[i] = samples[i].filename;
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
    //
		//  Set selected sample based on inputs.
		//  This must happen before we calculate start_position

		// selected_sample_slot = (unsigned int) calculate_inputs(SAMPLE_SELECT_INPUT, SAMPLE_SELECT_KNOB, SAMPLE_SELECT_ATTN_KNOB, NUMBER_OF_SAMPLES_FLOAT);
		// selected_sample_slot = clamp(selected_sample_slot, 0, NUMBER_OF_SAMPLES - 1);
		// Sample *selected_sample = &samples[selected_sample_slot];

    Sample *selected_sample = &samples[0];

    float length_multiplier = params[LEN_MULT_KNOB].getValue();
    float playback_length = calculate_inputs(LENGTH_INPUT, LENGTH_KNOB, LENGTH_ATTN_KNOB, 128) * length_multiplier;
    float start_position = calculate_inputs(SAMPLE_PLAYBACK_POSITION_INPUT, SAMPLE_PLAYBACK_POSITION_KNOB, SAMPLE_PLAYBACK_POSITION_ATTN_KNOB, selected_sample->total_sample_count);

    // Ensure that the inputs are within range
    if(start_position >= (selected_sample->total_sample_count - playback_length)) start_position = selected_sample->total_sample_count - playback_length;

    // Shorten the playback length if it would result in playback passing the end of the sample data.
    if(playback_length > (selected_sample->total_sample_count - start_position)) playback_length = selected_sample->total_sample_count - start_position;

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

    if(spawn_trigger.process(inputs[SPAWN_TRIGGER_INPUT].getVoltage()))
    {
      grain_blender_core.add(start_position, playback_length, pan, selected_sample);
    }

    if (selected_sample->loaded && grain_blender_core.isEmpty() == false)
    {
      // pre-calculate step amount and smooth rate. This is to reduce the amount of math needed
      // within each Ghost's getStereoOutput() and age() functions.

      if(inputs[PITCH_INPUT].isConnected())
      {
        step_amount = (selected_sample->sample_rate / args.sampleRate) + (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 0.5f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
      }
      else
      {
        step_amount = (selected_sample->sample_rate / args.sampleRate) + params[PITCH_KNOB].getValue();
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
