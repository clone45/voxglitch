/*

Autobreak Studio

Automatic Breakbeat module for VCV Rack by Voxglitch,
with extra "stuff".

To do:

Next: Ability to select sample length [postponed]

1. draw horizontal lines for some sequencers
2. see if I can center pan sequencer
3. add ability to load a folder of files
4. Update panel artwork
5. Add instructional intro message for new users
6. Copy / Save / Load sequence lengths

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
  unsigned int selected_memory_index = 0;
  unsigned int previously_selected_memory_index = 0;
  bool copy_mode = false;
  std::array<bool, NUMBER_OF_MEMORY_SLOTS> blink_memory_during_copy_mode;

  // Sequencer step keeps track of where the sequencers are.  This is important
  // because when a memory slot is loaded, the sequencers need to be set to 
  // the correct step.
  unsigned int sequencer_step = 0;

  // StereoSmoothSubModule loop_smooth;
  DeclickFilter declick_filter;
  StereoPan stereo_pan;

  std::string root_dir;
  std::string path;

  Sample samples[NUMBER_OF_SAMPLES];
  std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

  dsp::SchmittTrigger resetTrigger;
  dsp::SchmittTrigger clockTrigger;
  dsp::SchmittTrigger memory_button_triggers[NUMBER_OF_MEMORY_SLOTS];
  dsp::BooleanTrigger copyButtonTrigger;
  dsp::BooleanTrigger clearButtonTrigger;
  dsp::PulseGenerator clearPulse;

  AutobreakMemory autobreak_memory[16];

  VoltageSequencer* position_sequencer = &autobreak_memory[0].position_sequencer;
  VoltageSequencer* sample_sequencer = &autobreak_memory[0].sample_sequencer;
  VoltageSequencer* volume_sequencer = &autobreak_memory[0].volume_sequencer;
  VoltageSequencer* pan_sequencer = &autobreak_memory[0].pan_sequencer;
  VoltageSequencer* reverse_sequencer = &autobreak_memory[0].reverse_sequencer;
  VoltageSequencer* ratchet_sequencer = &autobreak_memory[0].ratchet_sequencer;

  float left_output = 0;
  float right_output = 0;

  enum ParamIds
  {
    ENUMS(MEMORY_BUTTONS, NUMBER_OF_MEMORY_SLOTS),
    COPY_BUTTON,
    CLEAR_BUTTON,
    NUM_PARAMS
  };
  enum InputIds
  {
    CLOCK_INPUT,
    RESET_INPUT,
    MEMORY_SELECT_INPUT,
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
    COPY_LIGHT,
    CLEAR_LIGHT,
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
    configInput(MEMORY_SELECT_INPUT, "Memory Select");

    std::fill_n(loaded_filenames, NUMBER_OF_SAMPLES, "[ EMPTY ]");
    blink_memory_during_copy_mode.fill(true);

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

    //
    // Save memory data (meaning, sequencer data)
    //
    json_t *memory_json = json_object();

    for(unsigned int memory_index = 0; memory_index < NUMBER_OF_MEMORY_SLOTS; memory_index++)
    {
      json_t *sequencers_json = json_object();

      // Save position sequencer values
      saveSequencer(sequencers_json, &autobreak_memory[memory_index].position_sequencer, "position_sequencer");
      saveSequencer(sequencers_json, &autobreak_memory[memory_index].sample_sequencer, "sample_sequencer");
      saveSequencer(sequencers_json, &autobreak_memory[memory_index].volume_sequencer, "volume_sequencer");
      saveSequencer(sequencers_json, &autobreak_memory[memory_index].pan_sequencer, "pan_sequencer");
      saveSequencer(sequencers_json, &autobreak_memory[memory_index].reverse_sequencer, "reverse_sequencer");
      saveSequencer(sequencers_json, &autobreak_memory[memory_index].ratchet_sequencer, "ratchet_sequencer");

      json_object_set(memory_json, std::string("memory_slot_" + std::to_string(memory_index)).c_str(), sequencers_json);
    }

    json_object_set(json_root, "memory", memory_json);

    // Save which memory is selected
    json_object_set(json_root, "selected_memory_index", json_integer(selected_memory_index));

    return json_root;
  }

  void saveSequencer(json_t *memory_json, VoltageSequencer* sequencer, std::string sequencer_name)
  {
      json_t *sequencer_values_json_array = json_array();
      for(unsigned int column = 0; column < NUMBER_OF_STEPS; column++)
      {
        json_array_append_new(sequencer_values_json_array, json_real(sequencer->getValue(column)));
      }

      json_object_set(memory_json, sequencer_name.c_str(), sequencer_values_json_array);
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

    //
    // Load Memory Data
    //   
    json_t *memory_json = json_object_get(json_root, "memory");

    if(memory_json)
    {
      for(unsigned int memory_slot_index=0; memory_slot_index<NUMBER_OF_MEMORY_SLOTS; memory_slot_index++)
      {
        std::string key = "memory_slot_" + std::to_string(memory_slot_index);
        json_t *memory_slot = json_object_get(memory_json, key.c_str());

        if(memory_slot)
        {
          loadSequencer(memory_slot, &autobreak_memory[memory_slot_index].position_sequencer, "position_sequencer");
          loadSequencer(memory_slot, &autobreak_memory[memory_slot_index].sample_sequencer, "sample_sequencer");
          loadSequencer(memory_slot, &autobreak_memory[memory_slot_index].volume_sequencer, "volume_sequencer");
          loadSequencer(memory_slot, &autobreak_memory[memory_slot_index].pan_sequencer, "pan_sequencer");
          loadSequencer(memory_slot, &autobreak_memory[memory_slot_index].reverse_sequencer, "reverse_sequencer");
          loadSequencer(memory_slot, &autobreak_memory[memory_slot_index].ratchet_sequencer, "ratchet_sequencer");
        }
      }
    }

    // Load selected memory index
    json_t *selected_memory_index_json = json_object_get(json_root, "selected_memory_index");
    if(selected_memory_index_json) selectMemory(json_integer_value(selected_memory_index_json));
  }

  void loadSequencer(json_t *memory_slot_json, VoltageSequencer* sequencer, std::string sequencer_name)
  {
    json_t *sequencer_array_json = json_object_get(memory_slot_json, sequencer_name.c_str());
    if(sequencer_array_json) 
    {
      size_t sequencer_index; 
      json_t *value_json;
      json_array_foreach(sequencer_array_json, sequencer_index, value_json) 
      {
        sequencer->setValue(sequencer_index, json_real_value(value_json));
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

  void selectMemory(unsigned int i)
  {
    selected_memory_index = i;

    // change memory locations here
    position_sequencer = &autobreak_memory[selected_memory_index].position_sequencer;
    sample_sequencer = &autobreak_memory[selected_memory_index].sample_sequencer;
    volume_sequencer = &autobreak_memory[selected_memory_index].volume_sequencer;
    pan_sequencer = &autobreak_memory[selected_memory_index].pan_sequencer;
    reverse_sequencer = &autobreak_memory[selected_memory_index].reverse_sequencer;
    ratchet_sequencer = &autobreak_memory[selected_memory_index].ratchet_sequencer;

    position_sequencer->setPosition(sequencer_step);
    sample_sequencer->setPosition(sequencer_step);
    volume_sequencer->setPosition(sequencer_step);
    pan_sequencer->setPosition(sequencer_step);
    reverse_sequencer->setPosition(sequencer_step);
    ratchet_sequencer->setPosition(sequencer_step);
  }



  /*

  ______
  | ___ \
  | |_/ / __ ___   ___ ___  ___ ___
  |  __/ '__/ _ \ / __/ _ \/ __/ __|  ==========================================
  | |  | | | (_) | (_|  __/\__ \__ \  ==========================================
  \_|  |_|  \___/ \___\___||___/___/  ==========================================
================================================================================
================================================================================
================================================================================
================================================================================
================================================================================

*/

  void process(const ProcessArgs &args) override
  {

    // Process clear button
    if (clearButtonTrigger.process(params[CLEAR_BUTTON].getValue())) 
    {
      autobreak_memory[selected_memory_index].clear();
			clearPulse.trigger(1e-3f);
		}

    // Process copy button 
    if (copyButtonTrigger.process(params[COPY_BUTTON].getValue())) 
    {
      copy_mode ^= true;

      if(copy_mode == true) 
      {
        blink_memory_during_copy_mode.fill(true);
        blink_memory_during_copy_mode[selected_memory_index] = false;
      }
		}

    // Memory selection
    //
    if(copy_mode == false)
    {
      if(inputs[MEMORY_SELECT_INPUT].isConnected())
      {
        // Read the memory input.  If the reading is different than the currently
        // selected memory slot, then load up the newly selected slot.
        unsigned int memory_input_value = int((inputs[MEMORY_SELECT_INPUT].getVoltage() / 10.0) * NUMBER_OF_MEMORY_SLOTS);
        memory_input_value = clamp(memory_input_value, 0, NUMBER_OF_MEMORY_SLOTS - 1);
        
        if(memory_input_value != previously_selected_memory_index)
        {
          selectMemory(memory_input_value);
          previously_selected_memory_index = selected_memory_index;
        }
      }
      else
      {
        for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++)
        {
          if(memory_button_triggers[i].process(params[MEMORY_BUTTONS + i].getValue()))
          {
            selectMemory(i);
          }
        }
      }

      // Highlight only selected memory buttton
      for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++)
      {
        params[MEMORY_BUTTONS + i].setValue(selected_memory_index == i);
      }
    }
    else // copy_mode == true
    {
      // copy_mode is true, so now, when a memory bank is clicked on, 
      // copy the currently selected memory to that desination.
      for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++)
      {
        if(memory_button_triggers[i].process(params[MEMORY_BUTTONS + i].getValue()))
        {
          autobreak_memory[i].copy(&autobreak_memory[selected_memory_index]);
        }
      }
    }



    // Process reset input
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
        position_sequencer->reset();
        sample_sequencer->reset();
        volume_sequencer->reset();
        pan_sequencer->reset();
        reverse_sequencer->reset();
        ratchet_sequencer->reset();

        // set flag to wait for next trigger to continue
        reset_signal = true;
        clock_ignore_on_reset = (long) (args.sampleRate / 100);
      }
    }


    // See if the clock input has triggered and store the results
    bool clock_trigger = false;
    
    // Read clock trigger
    //
    // A certain amount of time must pass after a reset before a clock input
    // will trigger the sequencers to step.  This time is measured by the 
    // clock_ignore_on_reset counter.  When it's 0, that means enough time
    // has passed that the module should pay attention to clock inputs.
    if(clock_ignore_on_reset == 0)
    {
      clock_trigger = clockTrigger.process(inputs[CLOCK_INPUT].getVoltage());
    }
    else
    {
      clock_ignore_on_reset--;
    }

    // Handle reset
    //
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
    unsigned int sample_value = (sample_sequencer->getValue() * NUMBER_OF_SAMPLES);
    sample_value = clamp(sample_value, 0, 7);

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
    }

    // If BPM hasn't been determined yet, wait until it has to start 
    // running the engine.
    if(bpm == 0.0) return;

    //
    // Handle ratcheting
    //

    // Ratchet will range from 0 to 1.0
    float ratchet = ratchet_sequencer->getValue();

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
      left_output = volume_sequencer->getValue() * left_output;
      right_output = volume_sequencer->getValue() * right_output;

      // Apply pan sequencer to output values
      stereo_pan.process(&left_output, &right_output, ((pan_sequencer->getValue() * 2.0) - 1.0));

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
    if (reverse_sequencer->getValue() >= 0.5)
    {
      theoretical_playback_position = theoretical_playback_position - 1;
    }
    else
    {
      theoretical_playback_position = theoretical_playback_position + 1;
    }

    // Optionally jump to new breakbeat position
    if (clock_trigger)
    {
      if(do_not_step_sequencers == false)
      {
        position_sequencer->step();
        sample_sequencer->step();
        volume_sequencer->step();
        pan_sequencer->step();
        reverse_sequencer->step();
        ratchet_sequencer->step();

        // Store the playback position so that we can restore the playback
        // positions when the active memory slot is changed.
        sequencer_step = position_sequencer->getPlaybackPosition();
      }

      float sequence_value = position_sequencer->getValue();
      int breakbeat_location = (sequence_value * 16) - 1;
      breakbeat_location = clamp(breakbeat_location, -1, 15);

      if (breakbeat_location != -1)
      {
        theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
      }

      ratchet_counter = 0;
    }
    else
    {
      if (ratchet_triggered)
      {
        float sequence_value = position_sequencer->getValue();
        int breakbeat_location = (sequence_value * 16) - 1;
        breakbeat_location = clamp(breakbeat_location, -1, 15);

        if (breakbeat_location != -1)
        {
          theoretical_playback_position = breakbeat_location * (samples_to_play_per_loop / 16.0f);
        }
        ratchet_triggered = false;
      }
    }

    // Clear out the do_not_step_sequencers flag
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
    
    lights[COPY_LIGHT].setBrightness(copy_mode);

    bool clearGate = clearPulse.process(args.sampleTime);
    lights[CLEAR_LIGHT].setSmoothBrightness(clearGate, args.sampleTime);
  }
};
