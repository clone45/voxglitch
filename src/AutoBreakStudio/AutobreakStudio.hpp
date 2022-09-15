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
  double bpm = 0.0;
  double timer_before = 0;
  bool clock_triggered = false;
  bool ratchet_triggered = false;
  unsigned int ratchet_counter = 0;
  bool reset_signal = false;
  bool do_not_step_sequencers = false;
  long clock_ignore_on_reset = 0;

  // StereoSmoothSubModule loop_smooth;
  DeclickFilter declick_filter;
  StereoPan stereo_pan;

  std::string root_dir;
  std::string path;

  Sample samples[NUMBER_OF_SAMPLES];
  std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger clockTrigger;
  // dsp::SchmittTrigger ratchetTrigger;

  VoltageSequencer position_sequencer;
  VoltageSequencer *selected_position_sequencer = &position_sequencer;

  VoltageSequencer sample_sequencer;
  VoltageSequencer *selected_sample_sequencer = &sample_sequencer;

  VoltageSequencer volume_sequencer;
  VoltageSequencer *selected_volume_sequencer = &volume_sequencer;

  VoltageSequencer pan_sequencer;
  VoltageSequencer *selected_pan_sequencer = &pan_sequencer;

  VoltageSequencer reverse_sequencer;
  VoltageSequencer *selected_reverse_sequencer = &reverse_sequencer;

  VoltageSequencer ratchet_sequencer;
  VoltageSequencer *selected_ratchet_sequencer = &ratchet_sequencer;

  float left_output = 0;
  float right_output = 0;

  enum ParamIds
  {
    NUM_PARAMS
  };
  enum InputIds
  {
    CLOCK_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds
  {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    ENUMS(LEFT_INDIVIDUAL_OUTPUTS, NUMBER_OF_SAMPLES),
    ENUMS(RIGHT_INDIVIDUAL_OUTPUTS, NUMBER_OF_SAMPLES),
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

    configInput(CLOCK_INPUT, "Clock Input");
    configInput(RESET_INPUT, "Reset Input");

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");

    // There are some hacks here to modify the snap divisions
    // that come standard with the voltage sequencer.  I should
    // rethink how the voltage sequencer is assigned the snap
    // division in the future, possibly moving the snap values
    // array into the module. 

    // Position sequencer
    position_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    position_sequencer.setSnapDivisionIndex(4);

    // Volume sequencer
    volume_sequencer.assign(NUMBER_OF_STEPS, 1.0);
    
    // Sample selection sequencer
    sample_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    sample_sequencer.snap_divisions[1] = NUMBER_OF_SAMPLES - 1;
    sample_sequencer.setSnapDivisionIndex(1);
    
    // Pan sequencer
    pan_sequencer.assign(NUMBER_OF_STEPS, 0.5);

    // Reverse sequencer
    reverse_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    
    ratchet_sequencer.assign(NUMBER_OF_STEPS, 0.0);
    ratchet_sequencer.snap_divisions[1] = 5;
    ratchet_sequencer.setSnapDivisionIndex(1);  

    clock_ignore_on_reset = (long) (44100 / 100);  
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
    // Check to see if the reset input has been triggered.  If so, reset
    // all of the sequencers and sample playback variables and set reset_signal
    // to true.  The reset_signal flag is used to delay any additional execution
    // until the next clock trigger happens.

    if (inputs[RESET_INPUT].isConnected())
    {
      if (resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
      {
        // Reset counters
        actual_playback_position = 0;
        theoretical_playback_position = 0;
        ratchet_counter = 0;

        // Smooth back into playback
        declick_filter.trigger();

        // Reset all of the sequencers
        position_sequencer.reset();
        sample_sequencer.reset();
        volume_sequencer.reset();
        pan_sequencer.reset();
        ratchet_sequencer.reset();
        reverse_sequencer.reset();

        // set flag to wait for next trigger to continue
        reset_signal = true;
        clock_ignore_on_reset = (long) (args.sampleRate / 100);
      }
    }


    // See if the clock input has triggered and store the results
    bool clock_trigger = false;
    
    if(clock_ignore_on_reset == 0)
    {
      clock_trigger = clockTrigger.process(inputs[CLOCK_INPUT].getVoltage());
    }
    else
    {
      clock_ignore_on_reset--;
    }

    // The reset_signal flag is set when the reset input is triggered.  When reset
    // is triggered, then we want to wait until the next clock pulse before
    // doing any sample output or stepping the sequencers.
    if(reset_signal == true)
    {
      // Once reset is triggered, don't run any sequencer stuff until
      // the next clock trigger comes in.
      if (! clock_trigger)
      {
        return;
      } 
      else // The clock trigger we've been waiting for has come!
      {
        // Clear the reset lock!
        reset_signal = false;

        // Don't step the sequencers ahead until next incoming clock trigger
        do_not_step_sequencers = true;

        // Don't compute new BPM until there's reasonable amounts of data
        timer_before = 0.0;
      }
    }

    //
    // Handle wav selection
    //
    unsigned int sample_value = (sample_sequencer.getValue() * NUMBER_OF_SAMPLES);
    if (sample_value != selected_sample_slot)
    {
      // Reset the smooth ramp if the selected sample has changed
      declick_filter.trigger();

      // Set the selected sample
      selected_sample_slot = sample_value;
    }
    Sample *selected_sample = &samples[selected_sample_slot];


    //
    // Handle BPM detection
    //
    time_counter += args.sampleTime;
    if (clock_trigger)
    {
      if (timer_before != 0)
      {
        // Compute BPM based on incoming clock
        double elapsed_time = time_counter - timer_before;
        if (elapsed_time > 0)
          bpm = 30.0 / elapsed_time;
      }

      timer_before = time_counter;
      // clock_triggered = true;
    }

    // If BPM hasn't been determined yet, wait until it has to start 
    // running the engine.
    if(bpm == 0.0) return;

    //
    // Handle ratcheting
    //

    // Ratchet will range from 0 to 1.0
    float ratchet = selected_ratchet_sequencer->getValue();

    if (ratchet > 0)
    {
      unsigned int samples_in_a_beat = ((60.0 / bpm) * args.sampleRate);

      // Ratchet divisions is an array defined in defines.h.  It contains 5 different
      // ratchet divisors for controlling the ratchet timing.  The larger the number,
      // the faster the ratchet.

      float ratchet_division = ratchet_divisions[int(ratchet * 4.0)];

      if (ratchet_counter >= (samples_in_a_beat / ratchet_division)) // double ratchet
      {
        ratchet_triggered = true;
        ratchet_counter = 0;
      }
      else
      {
        ratchet_counter++;
      }
    }
    else
    {
      ratchet_counter = 0;
    }


    // 60.0 is for conversion from minutes to seconds
    // 8.0 is for 8 beats (2 bars) of loops, which is a typical drum loop length
    float samples_to_play_per_loop = ((60.0 / bpm) * args.sampleRate) * 8.0;

    if (selected_sample->loaded && (selected_sample->size() > 0))
    {
      actual_playback_position = clamp(actual_playback_position, 0.0, selected_sample->size() - 1);

      selected_sample->read((int)actual_playback_position, &left_output, &right_output);

      // Apply volume sequencer to output values
      left_output = selected_volume_sequencer->getValue() * left_output;
      right_output = selected_volume_sequencer->getValue() * right_output;

      // Apply pan sequencer to output values
      stereo_pan.process(&left_output, &right_output, ((selected_pan_sequencer->getValue() * 2.0) - 1.0));

      // Handle smoothing
      declick_filter.process(&left_output, &right_output);

      // Output audio
      outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_output * GAIN);
      outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_output * GAIN);

      // Output individual track
      outputs[LEFT_INDIVIDUAL_OUTPUTS + selected_sample_slot].setVoltage(left_output * GAIN);
      outputs[RIGHT_INDIVIDUAL_OUTPUTS + selected_sample_slot].setVoltage(right_output * GAIN);
    }

    // Step the theoretical playback position
    if (selected_reverse_sequencer->getValue() >= 0.5)
    {
      theoretical_playback_position = theoretical_playback_position - 1;
    }
    else
    {
      theoretical_playback_position = theoretical_playback_position + 1;
    }

    // Optionally jump to new breakbeat position
    // if (clock_triggered &&  (clock_ignore_on_reset == 0))
    if (clock_trigger)
    {
      if(do_not_step_sequencers == false)
      {
        position_sequencer.step();
        sample_sequencer.step();
        volume_sequencer.step();
        pan_sequencer.step();
        ratchet_sequencer.step();
        reverse_sequencer.step();
      }

      float sequence_value = position_sequencer.getValue();
      int breakbeat_location = (sequence_value * 16) - 1;
      breakbeat_location = clamp(breakbeat_location, -1, 15);

      if (breakbeat_location != -1)
      {
        theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
      }

      // clock_triggered = false;
      ratchet_counter = 0;
    }
    else
    {
      if (ratchet_triggered)
      {
        float sequence_value = position_sequencer.getValue();
        int breakbeat_location = (sequence_value * 16) - 1;
        breakbeat_location = clamp(breakbeat_location, -1, 15);

        if (breakbeat_location != -1)
        {
          theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
        }
        ratchet_triggered = false;
      }
    }

    // Clear out this do_not_step_sequencers flag
    if(do_not_step_sequencers == true) do_not_step_sequencers = false;

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
};
