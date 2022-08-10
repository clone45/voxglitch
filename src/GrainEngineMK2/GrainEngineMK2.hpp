//
// TODO: Provide options for different window max and min
//

struct LoadQueue
{
  bool sample_queued_for_loading = false;
  std::string path_to_file = "";
  std::string filename = "";
  unsigned int sample_number = 0;

  void queue_sample_for_loading(std::string path_to_file, unsigned int sample_number)
  {
    // DEBUG("queue_sample_for_loading called");
    this->sample_queued_for_loading = true;
    this->path_to_file = path_to_file;
    this->sample_number = sample_number;
    this->filename = filename;
  }
};


struct GrainEngineMK2 : VoxglitchSamplerModule
{
  // Various internal variables
  unsigned int selected_sample_index = 0;
  double step_amount = 0.0;
  double sample_rate_division = 0.0;
  float smooth_rate = 0;
  int spawn_throttling_countdown = 0;
  unsigned int selected_waveform = 0;
	std::string loaded_filenames[NUMBER_OF_SAMPLES];
	std::string root_dir;
	std::string path;
  float pan = 0;
  LoadQueue load_queue;
  StereoFadeOut stereo_fade_out;
  StereoFadeIn stereo_fade_in;

  // Structs
  SamplePlayer sample_players[NUMBER_OF_SAMPLES];

  // Common common;
  GrainManager grain_manager;
  float start_position = 0.0;
  float draw_position = 0.0;

  // Triggers
  dsp::SchmittTrigger spawn_trigger;

  enum ParamIds {
    WINDOW_KNOB,
    WINDOW_ATTN_KNOB,
    POSITION_KNOB,
    POSITION_ATTN_KNOB,
    PITCH_KNOB,
    PITCH_ATTN_KNOB, // unused
    TRIM_KNOB,
    JITTER_KNOB,
    GRAINS_KNOB,
    GRAINS_ATTN_KNOB,
    RATE_KNOB,
    RATE_ATTN_KNOB,
    SAMPLE_KNOB,
    SAMPLE_ATTN_KNOB,
    NUM_PARAMS
  };
  enum InputIds {
    JITTER_INPUT,
    WINDOW_INPUT,
    POSITION_INPUT,
    PITCH_INPUT,
    SPAWN_TRIGGER_INPUT,
    PAN_INPUT,
    GRAINS_INPUT,
    RATE_INPUT,
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
    configParam(POSITION_KNOB, 0.0f, 1.0f, 0.0f, "PositionKnob");
    configParam(POSITION_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PositionAttnKnob");
    /*
    configParam(POSITION_COARSE_KNOB, 0.0f, 1.0f, 0.0f, "PositionCourseKnob");
    configParam(POSITION_COARSE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PositionCourseAttnKnob");
    configParam(POSITION_MEDIUM_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PositionMediumAttnKnob");
    configParam(POSITION_FINE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PositionFineAttnKnob");
    */
    configParam(PITCH_KNOB, -2.0f, 2.0f, 0.0f, "PitchKnob");
    // configParam(PITCH_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "PitchAttnKnob");
    configParam(TRIM_KNOB, 0.0f, 2.0f, 1.0f, "TrimKnob");
    configParam(JITTER_KNOB, 0.f, 1.0f, 0.5f, "JitterKnob");
    configParam(GRAINS_KNOB, 0.0f, 1.0f, 0.5f, "GrainsKnob");
    configParam(GRAINS_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "GrainsAttnKnob");
    configParam(RATE_KNOB, 0.0f, 1.0f, 0.7f, "RateKnob");
    configParam(RATE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "RateAttnKnob");
    configParam(SAMPLE_KNOB, 0.0f, 1.0f, 0.0f, "SampleKnob");
    configParam(SAMPLE_ATTN_KNOB, 0.0f, 1.0f, 0.0f, "SampleAttnKnob");

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    leftExpander.producerMessage = producer_message;
    leftExpander.consumerMessage = consumer_message;
  }

  ~GrainEngineMK2()
  {
    /*
    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      // delete samples[i];
      // samples[i] = NULL;
    }
    */
  }

  json_t *dataToJson() override
	{
		json_t *root = json_object();

    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_object_set_new(root, ("loaded_sample_path_" + std::to_string(i+1)).c_str(), json_string(sample_players[i].getPath().c_str()));
		}

    // Save bipolar pitch mode
    // json_object_set_new(root, "bipolar_pitch_mode", json_integer(bipolar_pitch_mode));

		return root;
	}

	void dataFromJson(json_t *root) override
	{
    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			json_t *loaded_sample_path = json_object_get(root, ("loaded_sample_path_" +  std::to_string(i+1)).c_str());
			if (loaded_sample_path)
			{
				sample_players[i].loadSample(json_string_value(loaded_sample_path));
				loaded_filenames[i] = sample_players[i].getFilename();
			}
		}

    // Load bipolar pitch mode
    /*
    json_t* bipolar_pitch_mode_json = json_object_get(root, "bipolar_pitch_mode");
    if (bipolar_pitch_mode_json) bipolar_pitch_mode = json_integer_value(bipolar_pitch_mode_json);
    */
	}

  float randomFloat(float min, float max)
  {
    float r = (float)rand() / (float)RAND_MAX;
    return(min + r * (max - min));
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
  double calculate_start_position(int input_index, int knob_index, int attenuator_index)
  {
    double input_value = inputs[input_index].getVoltage() / 10.0;
    double knob_value = params[knob_index].getValue();
    double attenuator_value = params[attenuator_index].getValue();

    if(input_value < 0.0) input_value = 0.0;
    if(input_value > 1.0) input_value = 1.0;

    return((input_value * attenuator_value) + knob_value);
  }
  */

  void process(const ProcessArgs &args) override
  {

    //
		//  Set selected sample based on inputs.
		//  This must happen before we calculate start_position

		selected_sample_index = (unsigned int) calculate_inputs(SAMPLE_INPUT, SAMPLE_KNOB, SAMPLE_ATTN_KNOB, NUMBER_OF_SAMPLES_FLOAT);
		selected_sample_index = clamp(selected_sample_index, 0, NUMBER_OF_SAMPLES - 1);

    // TODO: If sample selection changed, call updateSampleRateDivision();

    // If there's an expander module attached, communicate with it and find
    // out if there's a new sample that needs to be loaded.

    this->processExpander();

    if(load_queue.sample_queued_for_loading)
    {
      // If either there's no loaded sample in the sample slot, or the fade out
      // of the existing sample has completed then load the new sample and start fading in.
      if((stereo_fade_out.isFadingOut() == false) || (sample_players[load_queue.sample_number].isLoaded() == false))
      {
        // dequeue the request.  We're going to process it right now!
        load_queue.sample_queued_for_loading = false;

        // Load the sample!
        // DEBUG(("GrainEngineMK2 loading file " + load_queue.path_to_file + " into slot " + std::to_string(load_queue.sample_number)).c_str());
        sample_players[load_queue.sample_number].loadSample(load_queue.path_to_file);
        std::string path = sample_players[load_queue.sample_number].getPath();

        this->root_dir = path; // This is used by the widget class
        this->path = path;     // This is used by the widget class
        loaded_filenames[load_queue.sample_number] = sample_players[load_queue.sample_number].getFilename();

        stereo_fade_in.trigger();
      }
    }

    // if(! selected_sample->loaded) return;


    if(sample_players[selected_sample_index].isLoaded() == false) return;

    // Process Max Grains knob
    unsigned int max_grains = calculate_inputs(GRAINS_INPUT, GRAINS_KNOB, GRAINS_ATTN_KNOB, MAX_GRAINS);
    max_grains = clamp(max_grains, 0, MAX_GRAINS);

    // Process window (width of the grains) inputs
    float window_knob_value = calculate_inputs(WINDOW_INPUT, WINDOW_KNOB, WINDOW_ATTN_KNOB, 1.0, 6400.0);

    // unsigned int window_length = args.sampleRate / window_knob_value;
    unsigned int window_length = window_knob_value;

    // start_position = calculate_start_position(POSITION_INPUT, POSITION_KNOB, POSITION_ATTN_KNOB);
    start_position = ((inputs[POSITION_INPUT].getVoltage() / 10.0) * params[POSITION_ATTN_KNOB].getValue()) + params[POSITION_KNOB].getValue();
    start_position = clamp(start_position, 0.0, 1.0);

    // Convert start_position from 0-1 to 0-[sample size]
    start_position = start_position * sample_players[selected_sample_index].sample.size();

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

    if(jitter_spread > 0) start_position += this->randomFloat(-1 * jitter_spread, jitter_spread);

    // In Grain.hpp, it is ensured that start_position stays within the sample array length
    // This is ensured again in sample.h
    // Therefore, we don't jump through any hoops here to clamp the start_position

    // Process Pan input
    if(inputs[PAN_INPUT].isConnected()) pan = (inputs[PAN_INPUT].getVoltage() / 10.0);

    // Process Pitch input
    // float pitch_input = inputs[PITCH_INPUT].getVoltage() + params[PITCH_KNOB].getValue();
    // float step_amount = sample_players[selected_sample_index].getSampleIncrement(pitch_input);
    float step_amount = sample_rate_division * exp2(inputs[PITCH_INPUT].getVoltage() + params[PITCH_KNOB].getValue());


    // If there's a cable connected to the EXT CLOCK input, it takes priority over the internal clock
    // "SPAWN_TRIGGER_INPUT" is a name carried over from the Ghosts module and should eventually be renamed

    if(inputs[SPAWN_TRIGGER_INPUT].isConnected())
    {
      if(spawn_trigger.process(inputs[SPAWN_TRIGGER_INPUT].getVoltage())) {
        grain_manager.addGrain(start_position, window_length, pan, &sample_players[selected_sample_index].sample, max_grains, step_amount);
      }
    }
    else if(spawn_throttling_countdown == 0)
    {
      // This code controls the rate at which new grains are added
      grain_manager.addGrain(start_position, window_length, pan, &sample_players[selected_sample_index].sample, max_grains, step_amount);

      // scale value at RATE_INPUT (which goes from 0 to 1), to 0 to 2096
      float rate_inputs_value = rescale(calculate_inputs(RATE_INPUT, RATE_KNOB, RATE_ATTN_KNOB, 1.0), 1.f, 0.f, 0.f, 2096.f);
      if (rate_inputs_value < 0) rate_inputs_value = 0;
      spawn_throttling_countdown = (unsigned int) clamp(rate_inputs_value, 0.0, 2096.0);
    }

    //
    // Get output from the grain engine
    //
    if (! grain_manager.isEmpty())
    {
      // smooth_rate = 128.0f / args.sampleRate;

      // Get the output and increase the age of each grain
      std::pair<float, float> stereo_output = grain_manager.process();
      float left_mix_output = stereo_output.first * params[TRIM_KNOB].getValue();
      float right_mix_output = stereo_output.second * params[TRIM_KNOB].getValue();

      if(stereo_fade_in.isFadingIn()) stereo_fade_in.process(&left_mix_output, &right_mix_output, 100.0 * APP->engine->getSampleTime()); // 1/100th of a second
      if(stereo_fade_out.isFadingOut()) stereo_fade_out.process(&left_mix_output, &right_mix_output, 100.0 * APP->engine->getSampleTime()); // 1/100th of a second

      // Send audio to outputs
      outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_mix_output);
      outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_mix_output);
    }

    if(spawn_throttling_countdown > 0) spawn_throttling_countdown--;
    draw_position = start_position / sample_players[selected_sample_index].sample.size();
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
        std::string filename = expander_message->filename;
        std::string path = expander_message->path;

        if(filename != "")
        {
          // Retrieve the sample slot
          unsigned int sample_slot = expander_message->sample_slot;
          sample_slot = clamp(sample_slot, 0, 4);

          std::string path_to_file = path + "/" + filename;

          // Queue sample for loading
          load_queue.queue_sample_for_loading(path_to_file, sample_slot);
          stereo_fade_out.trigger();
        }

        // Set the received flag so we don't process the message every single frame
        expander_message->message_received = true;
      }

      leftExpander.messageFlipRequested = true;
    }
  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override
  {
    for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
    {
      sample_players[i].updateSampleRate();
    }
    updateSampleRateDivision();
  }

  void updateSampleRateDivision()
  {
    if(sample_players[selected_sample_index].isLoaded()) {
      sample_rate_division = sample_players[selected_sample_index].getSampleRate() / APP->engine->getSampleRate();
    }
  }

};
