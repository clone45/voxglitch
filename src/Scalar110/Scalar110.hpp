//
// Where I left off.
// - rename pattern to memory
// - tooltips?
// - more ratcheting patterns!


struct Scalar110 : Module
{
  MemorySlot memory_slots[NUMBER_OF_MEMORY_SLOTS];

  dsp::SchmittTrigger drum_pad_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger step_select_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger track_button_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger memory_slot_button_triggers[NUMBER_OF_MEMORY_SLOTS];
  dsp::SchmittTrigger function_button_triggers[NUMBER_OF_FUNCTIONS];
  dsp::SchmittTrigger track_length_button_triggers[NUMBER_OF_STEPS];

  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger resetTrigger;

  Track *selected_track = NULL;
  MemorySlot *selected_memory_slot = NULL;

  unsigned int memory_slot_index = 0;
  unsigned int track_index = 0;
  unsigned int playback_step = 0;
  unsigned int selected_function = 0;
  unsigned int old_selected_function = 0;
  unsigned int clock_division = 8;
  unsigned int clock_counter = 0;
  bool first_step = true;
  long clock_ignore_on_reset = 0;

  bool shift_key = false;

  //
  // Sample related variables
  //
  // * root_directory *: Used to store the last folder which the user accessed to
  // load samples.  This is to alleviate the tedium of having to navigate to
  // the same folder every time a user goes to load new samples.
  //
  // path: ??
  //
  // * loaded_filenames *: Filenames are displayed next to the tracks.  This variable
  // keeps track of the filenames to display.  Filenames are saved and loaded
  // with the patch
  //
  SamplePlayer sample_players[NUMBER_OF_TRACKS];
  std::string loaded_filenames[NUMBER_OF_TRACKS]; // for display on the front panel
  std::string root_directory;
	std::string path;

  enum ParamIds {
    ENUMS(DRUM_PADS, NUMBER_OF_STEPS),
    ENUMS(STEP_SELECT_BUTTONS, NUMBER_OF_STEPS),
    ENUMS(STEP_KNOBS, NUMBER_OF_STEPS),
    ENUMS(FUNCTION_BUTTONS, NUMBER_OF_FUNCTIONS),
    ENUMS(TRACK_BUTTONS, NUMBER_OF_TRACKS),
    ENUMS(MEMORY_SLOT_BUTTONS, NUMBER_OF_MEMORY_SLOTS),
    // ENUMS(TRACK_LENGTH_BUTTONS, NUMBER_OF_STEPS),
		NUM_PARAMS
	};
	enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
    MEM_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    ENUMS(TRACK_OUTPUTS, NUMBER_OF_TRACKS * 2),
		NUM_OUTPUTS
	};
  enum LightIds {
    ENUMS(DRUM_PAD_LIGHTS, NUMBER_OF_STEPS),
    ENUMS(STEP_LOCATION_LIGHTS, NUMBER_OF_STEPS),
    ENUMS(FUNCTION_BUTTON_LIGHTS, NUMBER_OF_FUNCTIONS),
    ENUMS(TRACK_BUTTON_LIGHTS, NUMBER_OF_TRACKS),
    ENUMS(MEMORY_SLOT_BUTTON_LIGHTS, NUMBER_OF_MEMORY_SLOTS),
    // ENUMS(TRACK_LENGTH_BUTTON_LIGHTS, NUMBER_OF_STEPS),
		NUM_LIGHTS
	};

	Scalar110()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    clock_ignore_on_reset = (long) (44100 / 100);

    for(unsigned int i=0; i < NUMBER_OF_STEPS; i++)
    {
      configParam(DRUM_PADS + i, 0.0, 1.0, 0.0, "drum_button_" + std::to_string(i));
      configParam(STEP_KNOBS + i, 0.0, 1.0, 0.0, "step_knob_" + std::to_string(i));
    }

    // There are 8 sample players, one for each track.  These sample players
    // are shared across the tracks contained in the memory slots.
    for(unsigned int p=0; p<NUMBER_OF_MEMORY_SLOTS; p++) {
      for(unsigned int t=0; t<NUMBER_OF_TRACKS; t++) {
        memory_slots[p].setSamplePlayer(t, &sample_players[t]);
      }
    }

    selected_memory_slot = &memory_slots[0];

    // Set default selected track
    selected_track = selected_memory_slot->getTrack(0);

    updateKnobPositions();
	}

  void updateKnobPositions()
  {
    for(unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      float value = 0;

      switch(selected_function)
      {
        case FUNCTION_OFFSET: value = selected_track->getOffset(step_number); break;
        case FUNCTION_PAN: value = selected_track->getPan(step_number); break;
        case FUNCTION_VOLUME: value = selected_track->getVolume(step_number); break;
        case FUNCTION_PITCH: value = selected_track->getPitch(step_number); break;
        case FUNCTION_RATCHET: value = selected_track->getRatchet(step_number); break;
        case FUNCTION_REVERSE: value = selected_track->getReverse(step_number); break;
        case FUNCTION_PROBABILITY: value = selected_track->getProbability(step_number); break;
        case FUNCTION_LOOP: value = selected_track->getLoop(step_number); break;
      }

      params[STEP_KNOBS + step_number].setValue(value);
    }
  }

  void switchMemory(unsigned int new_memory_slot)
  {
    memory_slot_index = new_memory_slot;

    // Switch memory_slots and set the selected track
    selected_memory_slot = &memory_slots[new_memory_slot];
    selected_track = selected_memory_slot->getTrack(track_index);

    // set all track positions
    for(unsigned int track_index=0; track_index < NUMBER_OF_TRACKS; track_index++)
    {
       selected_memory_slot->tracks[track_index].setPosition(playback_step);
    }

    updateKnobPositions();
  }

  //
	// SAVE module data
  //
	json_t *dataToJson() override
	{
		json_t *json_root = json_object();

    //
    // handle sample related save/load items
    //
    json_t *samples_json_array = json_array();

    for(unsigned int track_number=0; track_number < NUMBER_OF_TRACKS; track_number++)
    {
      std::string filename = this->sample_players[track_number].getFilename();
      std::string path = this->sample_players[track_number].getPath();

      json_t *sample_json_object = json_object();

      json_object_set(sample_json_object, "sample_filename", json_string(filename.c_str()));
      json_object_set(sample_json_object, "sample_path", json_string(path.c_str()));

      json_array_append_new(samples_json_array, sample_json_object);
    }
    json_object_set(json_root, "samples", samples_json_array);

    //
    // Save all memory slot data
    //
    json_t *memory_slots_json_array = json_array();
    for(int memory_slot_number=0; memory_slot_number<NUMBER_OF_MEMORY_SLOTS; memory_slot_number++)
    {
      // Save all track data
      json_t *tracks_json_array = json_array();
      for(int track_number=0; track_number<NUMBER_OF_TRACKS; track_number++)
      {
        json_t *steps_json_array = json_array();

        for(int step_index=0; step_index<NUMBER_OF_STEPS; step_index++)
        {
          json_t *step_data = json_object();
          json_object_set(step_data, "trigger", json_integer(this->memory_slots[memory_slot_number].tracks[track_number].getValue(step_index)));

          //  json_array_append_new(parameter_json_array, json_real(this->tracks[track_number].getParameter(step_index,parameter_index)));
          json_object_set(step_data, "offset", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getOffset(step_index)));
          json_object_set(step_data, "volume", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getVolume(step_index)));
          json_object_set(step_data, "pitch", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getPitch(step_index)));
          json_object_set(step_data, "pan", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getPan(step_index)));
          json_object_set(step_data, "ratchet", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getRatchet(step_index)));
          json_object_set(step_data, "reverse", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getReverse(step_index)));
          json_object_set(step_data, "probability", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getProbability(step_index)));
          json_object_set(step_data, "loop", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getLoop(step_index)));

          json_array_append_new(steps_json_array, step_data);
        }

        json_t *track_data = json_object();

        json_object_set(track_data, "steps", steps_json_array);
        json_object_set(track_data, "length", json_integer(this->memory_slots[memory_slot_number].tracks[track_number].getLength()));
        json_array_append_new(tracks_json_array, track_data);
      }

      json_t *tracks_json_object = json_object();
      json_object_set(tracks_json_object, "tracks", tracks_json_array);
      json_array_append_new(memory_slots_json_array, tracks_json_object);
    }
    json_object_set(json_root, "memory_slots", memory_slots_json_array);

    // Save path of the sample bank
    // json_object_set_new(json_root, "path", json_string(this->sample_bank.path.c_str()));

		return json_root;
	}

  //
	// LOAD module data
  //

	void dataFromJson(json_t *json_root) override
	{

    //
    // Load samples
    //
    json_t *samples_arrays_data = json_object_get(json_root, "samples");

    if(samples_arrays_data)
    {
      size_t sample_index;
      json_t *json_sample_object;

      json_array_foreach(samples_arrays_data, sample_index, json_sample_object)
      {
        json_t *sample_path_json = json_object_get(json_sample_object, "sample_path");

        if(sample_path_json)
        {
          std::string path = json_string_value(sample_path_json);
          if(path != "") this->sample_players[sample_index].loadSample(path);
          this->loaded_filenames[sample_index] = this->sample_players[sample_index].getFilename();
        }
      }
    }

    //
    // Load memory slots and track information
    //
    json_t *memory_slots_arrays_data = json_object_get(json_root, "memory_slots");

    if(memory_slots_arrays_data)
    {
      size_t memory_slot_index;
      json_t *json_memory_slot_object;

      json_array_foreach(memory_slots_arrays_data, memory_slot_index, json_memory_slot_object)
      {

        // Load all track data
        json_t *tracks_arrays_data = json_object_get(json_memory_slot_object, "tracks");

        if(tracks_arrays_data)
        {
          size_t track_index;
          size_t step_index;
          json_t *json_step_object;
          json_t *json_track_object;

          json_array_foreach(tracks_arrays_data, track_index, json_track_object)
          {
            // Load track length
            json_t *length_json = json_object_get(json_track_object, "length");
            if(length_json) this->memory_slots[memory_slot_index].tracks[track_index].setLength(json_integer_value(length_json));


            //
            // Load all of the step information, including trigger and parameter locks
            //

            json_t *steps_json_array = json_object_get(json_track_object, "steps");

            if(steps_json_array)
            {
              json_array_foreach(steps_json_array, step_index, json_step_object)
              {

                json_t *trigger_json = json_object_get(json_step_object, "trigger");
                if(trigger_json) this->memory_slots[memory_slot_index].tracks[track_index].setValue(step_index, json_integer_value(trigger_json));

                json_t *offset_json = json_object_get(json_step_object, "offset");
                if(offset_json) this->memory_slots[memory_slot_index].tracks[track_index].setOffset(step_index, json_real_value(offset_json));

                json_t *volume_json = json_object_get(json_step_object, "volume");
                if(volume_json) this->memory_slots[memory_slot_index].tracks[track_index].setVolume(step_index, json_real_value(volume_json));

                json_t *pitch_json = json_object_get(json_step_object, "pitch");
                if(pitch_json) this->memory_slots[memory_slot_index].tracks[track_index].setPitch(step_index, json_real_value(pitch_json));

                json_t *pan_json = json_object_get(json_step_object, "pan");
                if(pan_json) this->memory_slots[memory_slot_index].tracks[track_index].setPan(step_index, json_real_value(pan_json));

                json_t *ratchet_json = json_object_get(json_step_object, "ratchet");
                if(ratchet_json) this->memory_slots[memory_slot_index].tracks[track_index].setRatchet(step_index, json_real_value(ratchet_json));

                json_t *reverse_json = json_object_get(json_step_object, "reverse");
                if(reverse_json) this->memory_slots[memory_slot_index].tracks[track_index].setReverse(step_index, json_real_value(reverse_json));

                json_t *probability_json = json_object_get(json_step_object, "probability");
                if(probability_json) this->memory_slots[memory_slot_index].tracks[track_index].setProbability(step_index, json_real_value(probability_json));

                json_t *loop_json = json_object_get(json_step_object, "loop");
                if(loop_json) this->memory_slots[memory_slot_index].tracks[track_index].setLoop(step_index, json_real_value(loop_json));
              }
            }
          }
        } // end if tracks array data
      } // end foreach memory slot
    } // end if memory_slots array data

    updateKnobPositions();
	}


	void process(const ProcessArgs &args) override
	{
    //
    // If the user has pressed a track button, switch tracks and update the
    // knob positions for the selected function.

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      if(track_button_triggers[i].process(params[TRACK_BUTTONS + i].getValue()))
      {
        track_index = i;
        selected_track = selected_memory_slot->getTrack(track_index);

        // DEBUG(std::to_string(selected_track->length).c_str());

        updateKnobPositions();
      }
    }

    //
    // If the user has pressed a memory slot button, switch memory slots and update the
    // knob positions for the selected function.

    for(unsigned int i=0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      if(inputs[MEM_INPUT].isConnected())
      {
        unsigned int memory_selection = (inputs[MEM_INPUT].getVoltage() / 10.0) * NUMBER_OF_MEMORY_SLOTS;
        memory_selection = clamp(memory_selection, 0, NUMBER_OF_MEMORY_SLOTS - 1);
        switchMemory(memory_selection);
      }
      else
      {
        if(memory_slot_button_triggers[i].process(params[MEMORY_SLOT_BUTTONS + i].getValue()))
        {
          switchMemory(i);
        }
      }
    }

    // On incoming RESET, reset the sequencers
    if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      // Set up a (reverse) counter so that the clock input will ignore
      // incoming clock pulses for 1 millisecond after a reset input. This
      // is to comply with VCV Rack's standards.  See section "Timing" at
      // https://vcvrack.com/manual/VoltageStandards

      // clock_ignore_on_reset = (long) (args.sampleRate / 100);
      // clock_counter = clock_division;  // skip ratcheting
      first_step = true;

      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        selected_memory_slot->tracks[i].reset();
      }
    }

    //
    // Pad editing features:
    // Handle drum pads, drum location, and drum selection interactions and lights.
    //

    for(unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      // Process drum pads
      if(drum_pad_triggers[step_number].process(params[DRUM_PADS + step_number].getValue()))
      {
        // If the user is holding the control key, then set the track length instead
        // of toggling the drum pad.
        if(shift_key)
        {
          selected_track->length = step_number;
        }
        else
        {
          // Toggle the drum pad
          selected_track->toggleStep(step_number);
        }
      }

      // Light up drum pads
      lights[DRUM_PAD_LIGHTS + step_number].setBrightness(selected_track->getValue(step_number));

      // Show location
      lights[STEP_LOCATION_LIGHTS + step_number].setBrightness(playback_step == step_number);
    }

    //
    //  Function selection
    //

    for(unsigned int i=0; i < NUMBER_OF_FUNCTIONS; i++)
    {
      if(function_button_triggers[i].process(params[FUNCTION_BUTTONS + i].getValue()))
      {
        if(old_selected_function != i)
        {
          selected_function = i;
          old_selected_function = selected_function;
          updateKnobPositions();
        }
      }
    }

    // Process the knobs below the steps.  These change behavior depending on the selected function.
    for(unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      float value = params[STEP_KNOBS + step_number].getValue();

      switch(selected_function)
      {
        case FUNCTION_OFFSET: selected_track->setOffset(step_number, value); break;
        case FUNCTION_PAN: selected_track->setPan(step_number, value); break;
        case FUNCTION_VOLUME: selected_track->setVolume(step_number, value); break;
        case FUNCTION_PITCH: selected_track->setPitch(step_number, value); break;
        case FUNCTION_RATCHET: selected_track->setRatchet(step_number, value); break;
        case FUNCTION_REVERSE: selected_track->setReverse(step_number, value); break;
        case FUNCTION_PROBABILITY: selected_track->setProbability(step_number, value); break;
        case FUNCTION_LOOP: selected_track->setLoop(step_number, value); break;
      }
    }

    //
    // Clock and step features
    //
    if(stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      if(clock_counter == clock_division)
      {

        if(first_step == false) // If not the first step
        {
          // Step all of the tracks
          for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
          {
            selected_memory_slot->tracks[i].step();
          }
        }
        else
        {
          first_step = false;
        }

        // Trigger the samples
        // ===================
        // This may or may not actually trigger based on the track button
        // status and the probability parameter lock.
        for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
        {
          selected_memory_slot->tracks[i].trigger();
        }

        // Step the visual playback indicator (led) as well
        playback_step = selected_memory_slot->tracks[track_index].getPosition();

        // Reset clock division counter
        clock_counter = 0;
      }
      else
      {
        // Manage ratcheting
        for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
        {
          selected_memory_slot->tracks[i].ratchety();
        }
      }
      clock_counter++;
    }

    //
    // For each track...
    //  a. Get the output of the tracks and sum them for the stereo output
    //  b. Once the output has been read, increment the sample position

    float left_output = 0;
    float right_output = 0;
    float track_left_output;
    float track_right_output;

    for(unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      std::tie(track_left_output, track_right_output) = selected_memory_slot->tracks[i].getStereoOutput();

      // Individual outputs
      unsigned int left_index = i * 2;
      unsigned int right_index = left_index + 1;

      outputs[TRACK_OUTPUTS + left_index].setVoltage(track_left_output);
      outputs[TRACK_OUTPUTS + right_index].setVoltage(track_right_output);

      // Summed output
      left_output += track_left_output;
      right_output += track_right_output;
      selected_memory_slot->tracks[i].incrementSamplePosition(args.sampleRate);
    }

    // Summed output voltages at stereo outputs
    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_output);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_output);

    // function button lights
    for(unsigned int i=0; i<NUMBER_OF_FUNCTIONS; i++)
    {
      lights[FUNCTION_BUTTON_LIGHTS + i].setBrightness(selected_function == i);
    }

    // memory slot button lights
    for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++)
    {
      lights[MEMORY_SLOT_BUTTON_LIGHTS + i].setBrightness(memory_slot_index == i);
    }

    // track button lights
    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      lights[TRACK_BUTTON_LIGHTS + i].setBrightness(track_index == i);
    }

    if (clock_ignore_on_reset > 0) clock_ignore_on_reset--;
  }
};
