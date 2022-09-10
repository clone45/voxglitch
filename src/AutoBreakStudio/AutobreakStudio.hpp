/*

Autobreak Studio

Automatic Breakbeat module for VCV Rack by Voxglitch,
with extra stuff.

*/

struct AutobreakStudio : VoxglitchSamplerModule
{
  unsigned int selected_sample_slot = 0;

  // Actual index into the sample's array for playback
  float actual_playback_position = 0;

  // A location in a theoretical sample that's 8 bars at the selected BPM.
  // This value is stepped and repositioned when jumping around in a breakbeat.
  // This value is then used to figure out the actual_playback_position based
  // on the sample's length.

  float theoretical_playback_position = 0;

  // Pattern lock is a flag which, when true, stops the pattern from changing
  // due to changes in the pattern knob or the CV input.  This flag is set
  // to true when the user is actively editing the current pattern.

  double time_counter = 0.0;
  double bpm = 160;
  double timer_before = 0;
  bool clock_triggered = false;
  bool ratchet_triggered = false;

  // StereoSmoothSubModule loop_smooth;
  DeclickFilter declick_filter;

  std::string root_dir;
  std::string path;

  Sample samples[NUMBER_OF_SAMPLES];
  std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger clockTrigger;
  dsp::SchmittTrigger ratchetTrigger;

  VoltageSequencer position_sequencer;
  VoltageSequencer *selected_position_sequencer = &position_sequencer;

  VoltageSequencer sample_sequencer;
  VoltageSequencer *selected_sample_sequencer = &sample_sequencer;

  VoltageSequencer volume_sequencer;
  VoltageSequencer *selected_volume_sequencer = &volume_sequencer;

  GateSequencer reverse_sequencer;
  GateSequencer *selected_reverse_sequencer = &reverse_sequencer;

  float left_output = 0;
  float right_output = 0;

  enum ParamIds
  {
    WAV_KNOB,
    WAV_ATTN_KNOB,
    ENUMS(GATE_TOGGLE_BUTTONS, NUMBER_OF_STEPS),
    ENUMS(REVERSE_TOGGLE_BUTTONS, NUMBER_OF_STEPS),
    ENUMS(RATCHET_KNOBS, NUMBER_OF_STEPS),
    ENUMS(SAMPLE_KNOBS, NUMBER_OF_STEPS),
    NUM_PARAMS
  };
  enum InputIds
  {
    CLOCK_INPUT,
    RESET_INPUT,
    SEQUENCE_INPUT,
    // WAV_INPUT,
    RATCHET_INPUT,
    // REVERSE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds
  {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    DEBUG_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds
  {
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  AutobreakStudio()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(WAV_KNOB, 0.0f, 1.0f, 0.0f, "SampleSelectKnob");
    configParam(WAV_ATTN_KNOB, 0.0f, 1.0f, 1.0f, "SampleSelectAttnKnob");

    configInput(CLOCK_INPUT, "Clock Input");

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    position_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    volume_sequencer.assign(NUMBER_OF_STEPS, 1.0);
    sample_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    reverse_sequencer.assign(NUMBER_OF_STEPS, 0.0);

    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      configParam(SAMPLE_KNOBS + i, 0.0f, NUMBER_OF_SAMPLES - 1, 0.0f, "Sample Selection");
      paramQuantities[SAMPLE_KNOBS + i]->snapEnabled = true;
    }
  }

  // Autosave settings
  json_t *dataToJson() override
  {
    //
    // Save selected samples
    //

    json_t *json_root = json_object();
    for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
    {
      json_object_set_new(json_root, ("loaded_sample_path_" + std::to_string(i + 1)).c_str(), json_string(samples[i].path.c_str()));
    }

    return json_root;
  }

  // Autoload settings
  void dataFromJson(json_t *json_root) override
  {
    //
    // Load samples
    //

    for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
    {
      json_t *loaded_sample_path = json_object_get(json_root, ("loaded_sample_path_" + std::to_string(i + 1)).c_str());
      if (loaded_sample_path)
      {
        samples[i].load(json_string_value(loaded_sample_path));
        loaded_filenames[i] = samples[i].filename;
      }
    }
  }

  float calculate_inputs(int input_index, int knob_index, int attenuator_index, float scale)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0;
    float knob_value = params[knob_index].getValue();
    float attenuator_value = params[attenuator_index].getValue();

    return (((input_value * scale) * attenuator_value) + (knob_value * scale));
  }

  void process(const ProcessArgs &args) override
  {
    // 
    // Handle wav selection
    //
    // unsigned int wav_input_value = calculate_inputs(WAV_INPUT, WAV_KNOB, WAV_ATTN_KNOB, NUMBER_OF_SAMPLES);
    // wav_input_value = clamp(wav_input_value, 0, NUMBER_OF_SAMPLES - 1);

    // unsigned int sample_knob_value = params[SAMPLE_KNOBS + selected_volume_sequencer->getPlaybackPosition()].getValue();

    // unsigned int sample_knob_value = 0.0;

    unsigned int sample_value = (sample_sequencer.getValue() * NUMBER_OF_SAMPLES);

    if (sample_value != selected_sample_slot)
    {
      // Reset the smooth ramp if the selected sample has changed
      declick_filter.trigger();

      // Set the selected sample
      selected_sample_slot = sample_value;
    }
    Sample *selected_sample = &samples[selected_sample_slot];

    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      // Copy volume buttons into the currently selected gate sequencer
      // selected_volume_sequencer->setValue(i, params[GATE_TOGGLE_BUTTONS + i].getValue());

      // Copy reverse buttons into the currently selected reverse sequencer
      selected_reverse_sequencer->setValue(i, params[REVERSE_TOGGLE_BUTTONS + i].getValue());
    }

    //
    // Handle BPM detection
    //

    time_counter += 1.0 / args.sampleRate;

    if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
    {
      if (timer_before != 0)
      {
        // Compute BPM based on incoming clock
        double elapsed_time = time_counter - timer_before;
        if (elapsed_time > 0)
          bpm = 30.0 / elapsed_time;
      }

      timer_before = time_counter;
      clock_triggered = true;
    }

    if (ratchetTrigger.process(inputs[RATCHET_INPUT].getVoltage()))
    {
      ratchet_triggered = true;
    }

    //
    // Handle reset input
    //

    if (inputs[RESET_INPUT].isConnected())
    {
      if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
      {
        // Reset counters
        actual_playback_position = 0;
        theoretical_playback_position = 0;

        // Smooth back into playback
        declick_filter.trigger();
      }
    }

    if (selected_sample->loaded && (selected_sample->size() > 0))
    {
      // 60.0 is for conversion from minutes to seconds
      // 8.0 is for 8 beats (2 bars) of loops, which is a typical drum loop length
      float samples_to_play_per_loop = ((60.0 / bpm) * args.sampleRate) * 8.0;

      actual_playback_position = clamp(actual_playback_position, 0.0, selected_sample->size() - 1);

      selected_sample->read((int)actual_playback_position, &left_output, &right_output);

      // Apply volume sequencer to output values
      left_output = selected_volume_sequencer->getValue() * left_output;
      right_output = selected_volume_sequencer->getValue() * right_output;
      

      // Handle smoothing
      // float smooth_rate = (128.0f / args.sampleRate);
      // loop_smooth.process(left_output * GAIN, right_output * GAIN, smooth_rate, &left_output, &right_output);
      declick_filter.process(&left_output, &right_output);

      // Output audio
      outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_output * GAIN);
      outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_output * GAIN);

      // Step the theoretical playback position
      /*
      if (inputs[REVERSE_INPUT].getVoltage() >= 5)
      {
        theoretical_playback_position = theoretical_playback_position - 1;
      }
      else
      {
        theoretical_playback_position = theoretical_playback_position + 1;
      }
      */

      if(selected_reverse_sequencer->getValue())
      {
        theoretical_playback_position = theoretical_playback_position - 1;
      }
      else
      {
        theoretical_playback_position = theoretical_playback_position + 1;
      }

      // Optionally jump to new breakbeat position
      if (clock_triggered)
      {
        // float sequence_value = inputs[SEQUENCE_INPUT].getVoltage() / 10.0;

        // TODO: Dont step on first clock.  Bring over that logic from Digital Sequencer
        // TODO: loop through and step all squencers once memory is implemented
        position_sequencer.step();
        sample_sequencer.step();
        volume_sequencer.step();
        selected_reverse_sequencer->step();

        float sequence_value = position_sequencer.getValue();
        int breakbeat_location = (sequence_value * 16) - 1;
        breakbeat_location = clamp(breakbeat_location, -1, 15);

        if (breakbeat_location != -1)
        {
          theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
        }

        clock_triggered = false;
      }
      else
      {
        if (ratchet_triggered)
        {
          float sequence_value = inputs[SEQUENCE_INPUT].getVoltage() / 10.0;
          int breakbeat_location = (sequence_value * 16) - 1;
          breakbeat_location = clamp(breakbeat_location, -1, 15);

          if (breakbeat_location != -1)
          {
            theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
          }
          ratchet_triggered = false;
        }
      }

      // Loop the theoretical_playback_position
      if (theoretical_playback_position >= samples_to_play_per_loop)
      {
        theoretical_playback_position = 0;
        declick_filter.trigger();
      }
      else if (theoretical_playback_position < 0)
      {
        theoretical_playback_position = samples_to_play_per_loop;
        declick_filter.trigger();
      }

      // Map the theoretical playback position to the actual sample playback position
      actual_playback_position = ((float)theoretical_playback_position / samples_to_play_per_loop) * selected_sample->size();
    }
  }
};
