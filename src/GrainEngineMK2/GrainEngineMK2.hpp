struct GrainEngineMK2 : Module
{
  // Various internal variables
  unsigned int selected_sample_slot = 0;
  float pitch = 0;
  float smooth_rate = 0;
  int spawn_throttling_countdown = 0;
  float max_grains = 0;
  unsigned int selected_waveform = 0;
	std::string loaded_filenames[NUMBER_OF_SAMPLES];
	std::string root_dir;
	std::string path;
  float pan = 0;

  // Structs
  Sample *samples[NUMBER_OF_SAMPLES];
  Sample *selected_sample;

  Common common;
  GrainEngineMK2Core grain_engine_mk2_core;

  // Triggers
  dsp::SchmittTrigger spawn_trigger;

  enum ParamIds {
    WINDOW_KNOB,
    WINDOW_ATTN_KNOB,
    POSITION_COARSE_KNOB,
    POSITION_COARSE_ATTN_KNOB,
    POSITION_MEDIUM_ATTN_KNOB,
    POSITION_FINE_ATTN_KNOB,
    PITCH_KNOB,
    PITCH_ATTN_KNOB,
    TRIM_KNOB,
    JITTER_KNOB,
    GRAINS_KNOB,
    GRAINS_ATTN_KNOB,
    SPAWN_KNOB,
    SPAWN_ATTN_KNOB,
    SAMPLE_KNOB,
    SAMPLE_ATTN_KNOB,
    NUM_PARAMS
  };
  enum InputIds {
    JITTER_INPUT,
    WINDOW_INPUT,
    POSITION_COARSE_INPUT,
    POSITION_MEDIUM_INPUT,
    POSITION_FINE_INPUT,
    PITCH_INPUT,
    SPAWN_TRIGGER_INPUT,
    PAN_INPUT,
    GRAINS_INPUT,
    SPAWN_INPUT,
    SAMPLE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  GrainEngineExpanderMessage *producer_message = new GrainEngineExpanderMessage;
  GrainEngineExpanderMessage *consumer_message = new GrainEngineExpanderMessage;

  //
  // Constructor
  //
  GrainEngineMK2()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(WINDOW_KNOB, 0.0f, 1.0f, 1.0f, "WindowKnob");
    configParam(WINDOW_ATTN_KNOB, 0.0f, 1.0f, 0.00f, "WindowAttnKnob");
    configParam(POSITION_COARSE_KNOB, 0.0f, 1.0f, 0.0f, "PositionCourseKnob");
    configParam(POSITION_COARSE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PositionCourseAttnKnob");
    configParam(POSITION_MEDIUM_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PositionMediumAttnKnob");
    configParam(POSITION_FINE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PositionFineAttnKnob");
    configParam(PITCH_KNOB, -1.0f, 2.0f, 1.0f, "PitchKnob");
    configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PitchAttnKnob");
    configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
    configParam(JITTER_KNOB, 0.f, 1.0f, 0.5f, "JitterKnob");
    configParam(GRAINS_KNOB, 0.0f, 1.0f, 0.5f, "GrainsKnob");
    configParam(GRAINS_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "GrainsAttnKnob");
    configParam(SPAWN_KNOB, 0.0f, 1.0f, 0.7f, "SpawnKnob");
    configParam(SPAWN_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SpawnAttnKnob");
    configParam(SAMPLE_KNOB, 0.0f, 1.0f, 0.0f, "SampleKnob");
    configParam(SAMPLE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SampleAttnKnob");

    grain_engine_mk2_core.common = &common;
    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      samples[i] = new Sample();
    }

    leftExpander.producerMessage = producer_message;
    leftExpander.consumerMessage = consumer_message;
  }

  ~GrainEngineMK2()
  {
    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      // delete samples[i];
      // samples[i] = NULL;
    }
  }

  json_t *dataToJson() override
	{
		json_t *root = json_object();

    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(root, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(samples[i]->path.c_str()));
		}

		return root;
	}

	void dataFromJson(json_t *rootJ) override
	{
    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(rootJ, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				samples[i]->load(json_string_value(loaded_sample_path));
				loaded_filenames[i] = samples[i]->filename;
			}
		}
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

  void process(const ProcessArgs &args) override
  {
    this->processExpander();

    //
		//  Set selected sample based on inputs.
		//  This must happen before we calculate start_position

		selected_sample_slot = (unsigned int) calculate_inputs(SAMPLE_INPUT, SAMPLE_KNOB, SAMPLE_ATTN_KNOB, NUMBER_OF_SAMPLES_FLOAT);
		selected_sample_slot = clamp(selected_sample_slot, 0, NUMBER_OF_SAMPLES - 1);
		Sample *selected_sample = samples[selected_sample_slot];

    if(! selected_sample->loaded) return;

    // Process Max Grains knob
    this->max_grains = calculate_inputs(GRAINS_INPUT, GRAINS_KNOB, GRAINS_ATTN_KNOB, MAX_GRAINS);

    unsigned int contour_index = 0;

    // Process window (width of the grains) inputs
    float window_knob_value = calculate_inputs(WINDOW_INPUT, WINDOW_KNOB, WINDOW_ATTN_KNOB, 1.0, 6400.0);

    // unsigned int window_length = args.sampleRate / window_knob_value;
    unsigned int window_length = window_knob_value;

    float start_position = calculate_inputs(POSITION_COARSE_INPUT, POSITION_COARSE_KNOB, POSITION_COARSE_ATTN_KNOB, 0.0, 1.0);

    // At this point, start_position must be and should be between 0.0 and 1.0

    //
    // Process fine and medium position inputs
    //

    float position_fine = 0;
    float position_medium = 0;

    if(inputs[POSITION_FINE_INPUT].isConnected())
    {
      position_fine = inputs[POSITION_FINE_INPUT].getVoltage() / 10.0; // -1 to 1
      position_fine = clamp(position_fine, -1.0, 1.0);
      position_fine = position_fine * MAX_POSITION_FINE * params[POSITION_FINE_ATTN_KNOB].getValue();
    }

    if(inputs[POSITION_MEDIUM_INPUT].isConnected())
    {
      position_medium = inputs[POSITION_MEDIUM_INPUT].getVoltage() / 10.0; // -1 to 1
      position_medium = clamp(position_medium, -1.0, 1.0);
      position_medium = position_medium * MAX_POSITION_MEDIUM * params[POSITION_MEDIUM_ATTN_KNOB].getValue();
    }

    //
    // Process Jitter input
    //

    float jitter_spread = 0;

    if(inputs[JITTER_INPUT].isConnected())
    {
      jitter_spread = params[JITTER_KNOB].getValue() * MAX_JITTER_SPREAD * inputs[JITTER_INPUT].getVoltage();
    }
    else
    {
      jitter_spread = params[JITTER_KNOB].getValue() * MAX_JITTER_SPREAD;
    }

    // If jitter_spread is 124, then the jitter will be between -124 and 124.
    float jitter = common.randomFloat(-1 * jitter_spread, jitter_spread);

    // Make some room at the beginning and end of the possible range position to
    // allow for the addition of the jitter without pushing the start_position out of
    // range of the expander_message size.  Also leave room for the window length so that
    // none of the grains reaches the end of the expander_message.
    // start_position = common.rescaleWithPadding(start_position, 0.0, 1.0, 0.0, sample.total_sample_count, jitter_spread + MAX_POSITION_FINE, jitter_spread + MAX_POSITION_FINE + window_length);

    start_position = start_position * selected_sample->total_sample_count;
    start_position += (jitter + position_fine + position_medium);

    // Process Pan input
    if(inputs[PAN_INPUT].isConnected()) pan = (inputs[PAN_INPUT].getVoltage() / 10.0);

    // Process Pitch input
    if(inputs[PITCH_INPUT].isConnected())
    {
      // This assumes a unipolar input.  Is that correct?
      pitch = (((inputs[PITCH_INPUT].getVoltage() / 10.0f) - 5.0f) * params[PITCH_ATTN_KNOB].getValue()) + params[PITCH_KNOB].getValue();
    }
    else
    {
      pitch = params[PITCH_KNOB].getValue();
    }

    // If there's a cable connected to the spawn trigger input, it takes priority over the internal spwn rate.
    if(inputs[SPAWN_TRIGGER_INPUT].isConnected())
    {
      if(spawn_trigger.process(inputs[SPAWN_TRIGGER_INPUT].getVoltage())) grain_engine_mk2_core.add(start_position, window_length, pan, selected_sample, max_grains, pitch);
    }
    else if(spawn_throttling_countdown == 0)
    {
      grain_engine_mk2_core.add(start_position, window_length, pan, selected_sample, max_grains, pitch);

      float spawn_inputs_value = rescale(calculate_inputs(SPAWN_INPUT, SPAWN_KNOB, SPAWN_ATTN_KNOB, 1.0), 1.f, 0.f, 0.f, 2096.f);
      if (spawn_inputs_value < 0) spawn_inputs_value = 0;
      spawn_throttling_countdown = spawn_inputs_value;
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

  void processExpander()
  {
    if (leftExpander.module && leftExpander.module->model == modelGrainEngineMK2Expander)
    {
      // Receive message from expander
      GrainEngineExpanderMessage *expander_message = (GrainEngineExpanderMessage *) leftExpander.producerMessage;

      if(expander_message->message_received == false)
      {
        // Retrieve the path name
        std::string path = expander_message->path;
        std::string filename = expander_message->filename;

        // Retrieve the sample slot
        unsigned int sample_slot = expander_message->sample_slot;
        sample_slot = clamp(sample_slot, 0, 4);
        this->samples[sample_slot]->load(path + filename);
  			this->loaded_filenames[sample_slot] = this->samples[sample_slot]->filename;

        // Set the received flag so we don't process the message every single frame
        expander_message->message_received = true;
      }

      leftExpander.messageFlipRequested = true;
    }
  }

};
