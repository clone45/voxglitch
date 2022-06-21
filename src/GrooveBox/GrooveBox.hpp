//
// Voxglitch Groovebox
//
// By Bret Truchan
//
// TODO:
//
// - Thank you to all the friendly people on the VCV Rack Community for answering
//   my questions and providing feedback on early builds.


struct GrooveBox : VoxglitchSamplerModule
{
  MemorySlot memory_slots[NUMBER_OF_MEMORY_SLOTS];

  // Schmitt Triggers
  dsp::SchmittTrigger drum_pad_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger step_select_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger track_button_triggers[NUMBER_OF_TRACKS];
  dsp::SchmittTrigger memory_slot_button_triggers[NUMBER_OF_MEMORY_SLOTS];
  dsp::SchmittTrigger function_button_triggers[NUMBER_OF_FUNCTIONS];
  dsp::SchmittTrigger copy_button_trigger;
  dsp::SchmittTrigger paste_button_trigger;
  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger resetTrigger;

  // Pointers to select track and memory
  Track *selected_track = NULL;
  MemorySlot *selected_memory_slot = NULL;

  // Assorted variables
  unsigned int memory_slot_index = 0;
  unsigned int copied_memory_index = 0;
  unsigned int track_index = 0;
  unsigned int playback_step = 0;
  unsigned int selected_function = 0;
  unsigned int old_selected_function = 0;
  unsigned int clock_division = 8;
  unsigned int clock_counter = clock_division;
  bool first_step = true;
  bool shift_key = false;
  bool control_key = false;
  bool mutes[NUMBER_OF_TRACKS];
  bool solos[NUMBER_OF_TRACKS];
  bool any_track_soloed = false;
  bool expander_connected = false;
  bool show_sample_visualizer = false;
  unsigned int sample_visualizer_step = 0;
  unsigned int sample_position_snap_track_values[NUMBER_OF_TRACKS];

  // A pair of GrooveBoxExpanderMessage structures for sending information
  // from the expander to the groovebox.  Note that they both essentially
  // serve the same purpose, but are rotated during read/write cycles
  ExpanderToGrooveboxMessage expander_to_groovebox_message_a;
  ExpanderToGrooveboxMessage expander_to_groovebox_message_b;
  float track_volumes[NUMBER_OF_TRACKS];


  // The track_triggers array is used to send trigger information to the
  // expansion module.  Once a trigger is sent, it's immediately set back to
  // zero.
  bool track_triggers[NUMBER_OF_TRACKS];


  //
  // Sample related variables
  //
  // root_directory: Used to store the last folder which the user accessed to
  // load samples.  This is to alleviate the tedium of having to navigate to
  // the same folder every time a user goes to load new samples.
  //
  // loaded_filenames: Filenames are displayed next to the tracks.  This variable
  // keeps track of the filenames to display.
  //
  std::string loaded_filenames[NUMBER_OF_TRACKS]; // for display on the front panel
  std::string root_directory;
	std::string path;

  // Each of the 8 tracks has dedicated sample player engines
  // The samples assigned to each track are NOT dependent on which memory bank
  // is active.  This means that you cannot load samples in one memory bank without
  // it affecting all memory banks.

  SamplePlayer sample_players[NUMBER_OF_TRACKS];

  // sample position snap settings
  unsigned int sample_position_snap_indexes[NUMBER_OF_TRACKS];


  //
  // Basic VCV Parameter, Input, Output, and Light definitions
  //

  enum ParamIds {
    ENUMS(DRUM_PADS, NUMBER_OF_STEPS),
    ENUMS(STEP_SELECT_BUTTONS, NUMBER_OF_STEPS),
    ENUMS(STEP_KNOBS, NUMBER_OF_STEPS),
    ENUMS(FUNCTION_BUTTONS, NUMBER_OF_FUNCTIONS),
    ENUMS(TRACK_BUTTONS, NUMBER_OF_TRACKS),
    ENUMS(MEMORY_SLOT_BUTTONS, NUMBER_OF_MEMORY_SLOTS),
    COPY_BUTTON,
    PASTE_BUTTON,
    MASTER_VOLUME,
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
		NUM_LIGHTS
	};

	GrooveBox()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Configure all of the step buttons and parameter lock knobs
    for(unsigned int i=0; i < NUMBER_OF_STEPS; i++)
    {
      configParam(DRUM_PADS + i, 0.0, 1.0, 0.0, "Step Button");
      configParam(STEP_KNOBS + i, 0.0, 1.0, 0.0, "Parameter Lock Value " + std::to_string(i));
      configParam(FUNCTION_BUTTONS + i, 0.0, 1.0, 0.0, FUNCTION_NAMES[i]);
    }

    // Configure the track select buttons
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      configParam(TRACK_BUTTONS + i, 0.0, 1.0, 0.0, "Track Selection Button");
      this->mutes[i] = false;
      this->solos[i] = false;
      this->track_volumes[i] = 1.0;
      this->sample_position_snap_indexes[i] = 0;
    }

    // Configure the individual track outputs
    for(unsigned int i=0; i < (NUMBER_OF_TRACKS * 2); i+=2)
    {
      configOutput(TRACK_OUTPUTS + i, "Track " + std::to_string((i/2) + 1) + ": left");
      configOutput(TRACK_OUTPUTS + i + 1, "Track " + std::to_string((i/2) + 1) + ": right");
    }

    // Configure the stereo mix outputs
    configOutput(AUDIO_OUTPUT_LEFT, "Left Mix");
    configOutput(AUDIO_OUTPUT_RIGHT, "Right Mix");

    // Create tooltips for the memory slots
    for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++) {
      configParam(MEMORY_SLOT_BUTTONS + i, 0.0, 1.0, 0.0, "Memory Slot #" + std::to_string(i+1));
    }

    // Configure the master output knob
    configParam(MASTER_VOLUME, 0.0, 1.0, 0.5, "Master Volume");
    paramQuantities[MASTER_VOLUME]->randomizeEnabled = false;

    // There are 8 sample players, one for each track.  These sample players
    // are shared across the tracks contained in the memory slots.
    for(unsigned int p=0; p<NUMBER_OF_MEMORY_SLOTS; p++) {
      for(unsigned int t=0; t<NUMBER_OF_TRACKS; t++) {
        memory_slots[p].setSamplePlayer(t, &sample_players[t]);
      }
    }



    // Store a pointer to the active memory slot
    selected_memory_slot = &memory_slots[0];

    // Store a pointer to the active track
    selected_track = selected_memory_slot->getTrack(0);

    // Update parameter lock knobs.  I'm not sure if this is necessary.
    updateKnobPositions();

    // Set the leftExpander pointers to the two message holders
    leftExpander.producerMessage = &expander_to_groovebox_message_a;
    leftExpander.consumerMessage = &expander_to_groovebox_message_b;
	}

  // copyMemory(src_index, dst_index)
  //
  // Helper function to copy one memory slot to another memory slot
  //
  void copyMemory(unsigned int src_index, unsigned int dst_index)
  {
      memory_slots[dst_index].copy(&memory_slots[src_index]);
      updateKnobPositions();
  }

  void initialize()
  {
    for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++)
    {
      memory_slots[i].initialize();
    }

    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      // this->sample_players[i].initialize();
      this->loaded_filenames[i] = "";
      this->sample_position_snap_indexes[i] = 0;
    }

    this->updateKnobPositions();
  }

  void updateKnobPositions()
  {
    for(unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      float value = 0;

      switch(selected_function)
      {
        case FUNCTION_SAMPLE_START: value = selected_track->getSampleStart(step_number); break;
        case FUNCTION_SAMPLE_END: value = selected_track->getSampleEnd(step_number); break;
        case FUNCTION_PAN: value = selected_track->getPan(step_number); break;
        case FUNCTION_VOLUME: value = selected_track->getVolume(step_number); break;
        case FUNCTION_PITCH: value = selected_track->getPitch(step_number); break;
        case FUNCTION_RATCHET: value = selected_track->getRatchet(step_number); break;
        case FUNCTION_REVERSE: value = selected_track->getReverse(step_number); break;
        case FUNCTION_PROBABILITY: value = selected_track->getProbability(step_number); break;
        case FUNCTION_LOOP: value = selected_track->getLoop(step_number); break;
        case FUNCTION_ATTACK: value = selected_track->getAttack(step_number); break;
        case FUNCTION_RELEASE: value = selected_track->getRelease(step_number); break;
        case FUNCTION_DELAY_MIX: value = selected_track->getDelayMix(step_number); break;
        case FUNCTION_DELAY_LENGTH: value = selected_track->getDelayLength(step_number); break;
        case FUNCTION_DELAY_FEEDBACK: value = selected_track->getDelayFeedback(step_number); break;
        default: value = 0;
      }

      params[STEP_KNOBS + step_number].setValue(value);
    }
  }

  void switchMemory(unsigned int new_memory_slot)
  {
    memory_slot_index = new_memory_slot;

    // Switch memory_slots and set the selected track
    selected_memory_slot = &memory_slots[new_memory_slot];
    selected_track = selected_memory_slot->getTrack(this->track_index);

    // set all track positions
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
       selected_memory_slot->tracks[i].setPosition(playback_step);
    }

    updateKnobPositions();
  }

  void shiftAllTracks(unsigned int amount)
  {
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
       selected_memory_slot->tracks[i].shift(amount);
    }
    updateKnobPositions();
  }

  void shiftTrack(unsigned int amount)
  {
    selected_memory_slot->tracks[this->track_index].shift(amount);
    updateKnobPositions();
  }

  void setSamplePositionSnapIndex(unsigned int sample_position_snap_index, unsigned int track_index)
  {
    this->sample_position_snap_indexes[track_index] = sample_position_snap_index;
    this->sample_position_snap_track_values[track_index] = sample_position_snap_values[sample_position_snap_index];
  }

  //
	// SAVE module data
  //
	json_t *dataToJson() override
	{
		json_t *json_root = json_object();

    //
    // handle track related information that is tied to the track index and
    // doesn't change when the memory slot is changed.
    //
    json_t *track_data_json_array = json_array();

    for(unsigned int track_number=0; track_number < NUMBER_OF_TRACKS; track_number++)
    {
      std::string filename = this->sample_players[track_number].getFilename();
      std::string path = this->sample_players[track_number].getPath();

      json_t *track_json_object = json_object();

      json_object_set(track_json_object, "sample_filename", json_string(filename.c_str()));
      json_object_set(track_json_object, "sample_path", json_string(path.c_str()));
      json_object_set(track_json_object, "sample_position_snap_index", json_integer(this->sample_position_snap_indexes[track_number]));

      json_array_append_new(track_data_json_array, track_json_object);
    }
    json_object_set(json_root, "shared_track_data", track_data_json_array);


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
          json_object_set(step_data, "sample_start", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getSampleStart(step_index)));
          json_object_set(step_data, "sample_end", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getSampleEnd(step_index)));
          json_object_set(step_data, "volume", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getVolume(step_index)));
          json_object_set(step_data, "pitch", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getPitch(step_index)));
          json_object_set(step_data, "pan", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getPan(step_index)));
          json_object_set(step_data, "ratchet", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getRatchet(step_index)));
          json_object_set(step_data, "reverse", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getReverse(step_index)));
          json_object_set(step_data, "probability", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getProbability(step_index)));
          json_object_set(step_data, "loop", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getLoop(step_index)));
          json_object_set(step_data, "attack", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getAttack(step_index)));
          json_object_set(step_data, "release", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getRelease(step_index)));
          json_object_set(step_data, "delay_mix", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getDelayMix(step_index)));
          json_object_set(step_data, "delay_length", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getDelayLength(step_index)));
          json_object_set(step_data, "delay_feedback", json_real(this->memory_slots[memory_slot_number].tracks[track_number].getDelayFeedback(step_index)));

          json_array_append_new(steps_json_array, step_data);
        }

        json_t *track_data = json_object();

        json_object_set(track_data, "steps", steps_json_array);
        json_object_set(track_data, "range_start", json_integer(this->memory_slots[memory_slot_number].tracks[track_number].getRangeStart()));
        json_object_set(track_data, "range_end", json_integer(this->memory_slots[memory_slot_number].tracks[track_number].getRangeEnd()));
        json_array_append_new(tracks_json_array, track_data);
      }

      json_t *tracks_json_object = json_object();
      json_object_set(tracks_json_object, "tracks", tracks_json_array);
      json_array_append_new(memory_slots_json_array, tracks_json_object);
    }
    json_object_set(json_root, "memory_slots", memory_slots_json_array);

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
    json_t *shared_track_data = json_object_get(json_root, "shared_track_data");

    if(shared_track_data)
    {
      size_t track_index;
      json_t *json_track_data_object;

      json_array_foreach(shared_track_data, track_index, json_track_data_object)
      {
        json_t *sample_path_json = json_object_get(json_track_data_object, "sample_path");
        if(sample_path_json)
        {
          std::string path = json_string_value(sample_path_json);
          if(path != "") this->sample_players[track_index].loadSample(path);
          this->loaded_filenames[track_index] = this->sample_players[track_index].getFilename();
        }

        // Deprecated, but around for a while while people still have old versions
        json_t *offset_snap_index_json = json_object_get(json_track_data_object, "offset_snap_index");
        if(offset_snap_index_json)
        {
          unsigned int offset_snap_index = json_integer_value(offset_snap_index_json);
          setSamplePositionSnapIndex(offset_snap_index, track_index);
        }

        // Newer version that replaces the version above ^
        json_t *sample_position_snap_index_json = json_object_get(json_track_data_object, "sample_position_snap_index");
        if(sample_position_snap_index_json)
        {
          unsigned int sample_position_snap_index = json_integer_value(sample_position_snap_index_json);
          setSamplePositionSnapIndex(sample_position_snap_index, track_index);
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
            // Load track ranges
            json_t *range_end_json = json_object_get(json_track_object, "range_end");
            if(range_end_json) this->memory_slots[memory_slot_index].tracks[track_index].setRangeEnd(json_integer_value(range_end_json));

            json_t *range_start_json = json_object_get(json_track_object, "range_start");
            if(range_start_json) this->memory_slots[memory_slot_index].tracks[track_index].setRangeStart(json_integer_value(range_start_json));


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

                // Deprecated.  Will be removed eventually
                json_t *offset_json = json_object_get(json_step_object, "offset");
                if(offset_json) this->memory_slots[memory_slot_index].tracks[track_index].setSampleStart(step_index, json_real_value(offset_json));

                json_t *sample_start_json = json_object_get(json_step_object, "sample_start");
                if(sample_start_json) this->memory_slots[memory_slot_index].tracks[track_index].setSampleStart(step_index, json_real_value(sample_start_json));

                json_t *sample_end_json = json_object_get(json_step_object, "sample_end");
                if(sample_end_json) this->memory_slots[memory_slot_index].tracks[track_index].setSampleEnd(step_index, json_real_value(sample_end_json));

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

                json_t *attack_json = json_object_get(json_step_object, "attack");
                if(attack_json) this->memory_slots[memory_slot_index].tracks[track_index].setAttack(step_index, json_real_value(attack_json));

                json_t *release_json = json_object_get(json_step_object, "release");
                if(release_json) this->memory_slots[memory_slot_index].tracks[track_index].setRelease(step_index, json_real_value(release_json));

                json_t *delay_mix_json = json_object_get(json_step_object, "delay_mix");
                if(delay_mix_json) this->memory_slots[memory_slot_index].tracks[track_index].setDelayMix(step_index, json_real_value(delay_mix_json));

                json_t *delay_length_json = json_object_get(json_step_object, "delay_length");
                if(delay_length_json) this->memory_slots[memory_slot_index].tracks[track_index].setDelayLength(step_index, json_real_value(delay_length_json));

                json_t *delay_feedback_json = json_object_get(json_step_object, "delay_feedback");
                if(delay_feedback_json) this->memory_slots[memory_slot_index].tracks[track_index].setDelayFeedback(step_index, json_real_value(delay_feedback_json));

              }
            }
          }
        } // end if tracks array data
      } // end foreach memory slot
    } // end if memory_slots array data

    updateKnobPositions();
	}

  bool trigger(unsigned int track_id)
  {
    unsigned int sample_position_snap_value = sample_position_snap_track_values[track_id];
    if(notMuted(track_id)) return(selected_memory_slot->tracks[track_id].trigger(sample_position_snap_value));
    return(false);
  }

  bool notMuted(unsigned int track_id)
  {
    if(any_track_soloed) return(solos[track_id]);
    return(! mutes[track_id]);
  }

	void process(const ProcessArgs &args) override
	{
    if(leftExpander.module && leftExpander.module->model == modelGrooveBoxExpander)
    {
      expander_connected = true;
    }
    else
    {
      if(expander_connected) detachExpander();
      expander_connected = false;
    }

    if(expander_connected) readFromExpander(args.sampleRate);

    //
    // If the user has pressed a track button, switch tracks and update the
    // knob positions for the selected function.

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      if(track_button_triggers[i].process(params[TRACK_BUTTONS + i].getValue()))
      {
        track_index = i;
        selected_track = selected_memory_slot->getTrack(track_index);

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
        if(memory_selection != memory_slot_index) switchMemory(memory_selection);
      }
      else
      {
        if(memory_slot_button_triggers[i].process(params[MEMORY_SLOT_BUTTONS + i].getValue()))
        {
          // If shift-clicking, then copy the current memory into the clicked memory slot
          // before switching to the new memory slot
          if(shift_key) copyMemory(memory_slot_index, i);

          switchMemory(i);
        }
      }
    }

    // COPY: If the user has pressed the copy button, then store the index of the
    // current memory, which will be used when pasting.
    if(copy_button_trigger.process(params[COPY_BUTTON].getValue()))
    {
      copied_memory_index = memory_slot_index;
    }

    // PASTE: If the user has pressed the paste button, then copy previously
    // copied memory to the current memory location.
    if(paste_button_trigger.process(params[PASTE_BUTTON].getValue()))
    {
      copyMemory(copied_memory_index, memory_slot_index);
    }

    // On incoming RESET, reset the sequencers
    if(resetTrigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      // Set up a (reverse) counter so that the clock input will ignore
      // incoming clock pulses for 1 millisecond after a reset input. This
      // is to comply with VCV Rack's standards.  See section "Timing" at
      // https://vcvrack.com/manual/VoltageStandards

      first_step = true;
      clock_counter = clock_division;

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
        selected_track->toggleStep(step_number);
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

      // Special code to ensure that sample end isn't smaller than sample start
      /*
      if (selected_function == FUNCTION_SAMPLE_END)
      {
        float sample_start = selected_track->getSampleStart(step_number);

        // If sample end knob is greater than sample start, then adjust sample
        // end knob to be equal to the sample start.

        if(value < sample_start)
        {
          // params[STEP_KNOBS + step_number].setValue(sample_start);
          value = sample_start;
        }

      }
      */

      switch(selected_function)
      {
        case FUNCTION_SAMPLE_START: selected_track->setSampleStart(step_number, value); break;
        case FUNCTION_SAMPLE_END: selected_track->setSampleEnd(step_number, value); break;
        case FUNCTION_PAN: selected_track->setPan(step_number, value); break;
        case FUNCTION_VOLUME: selected_track->setVolume(step_number, value); break;
        case FUNCTION_PITCH: selected_track->setPitch(step_number, value); break;
        case FUNCTION_RATCHET: selected_track->setRatchet(step_number, value); break;
        case FUNCTION_REVERSE: selected_track->setReverse(step_number, value); break;
        case FUNCTION_PROBABILITY: selected_track->setProbability(step_number, value); break;
        case FUNCTION_LOOP: selected_track->setLoop(step_number, value); break;
        case FUNCTION_ATTACK: selected_track->setAttack(step_number, value); break;
        case FUNCTION_RELEASE: selected_track->setRelease(step_number, value); break;
        case FUNCTION_DELAY_MIX: selected_track->setDelayMix(step_number, value); break;
        case FUNCTION_DELAY_LENGTH: selected_track->setDelayLength(step_number, value); break;
        case FUNCTION_DELAY_FEEDBACK: selected_track->setDelayFeedback(step_number, value); break;
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
          // track_triggers are used to send values to the expander
          this->track_triggers[i] = this->trigger(i);
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
          if(notMuted(i)) this->track_triggers[i] = selected_memory_slot->tracks[i].ratchety();
        }
      }
      clock_counter++;
    }

    //
    // For each track...
    //  a. Get the output of the tracks and sum them for the stereo output
    //  b. Once the output has been read, increment the sample position

    float mix_left_output = 0;
    float mix_right_output = 0;
    float track_left_output = 0;
    float track_right_output = 0;

    //
    // Output individual stereo pairs for each track
    //
    for(unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      std::tie(track_left_output, track_right_output) = selected_memory_slot->tracks[i].getStereoOutput(this->interpolation);

      //
      // Apply track volumes from expander
      //
      // https://mathcracker.com/exponential-function-calculator
      // Using first t (t1) = .5,
      // First y (f(t1)) = 1
      // Second t (t2) = 2
      // Second y (f(t2)) = 12
      //
      // f(t) = 0.5503 * e^(1.1945 * t)

      float track_volume_multiplier = 0.4368 * std::exp(1.16566 * track_volumes[i]);
      track_left_output = track_left_output * track_volume_multiplier;
      track_right_output = track_right_output * track_volume_multiplier;

      // Individual outputs
      unsigned int left_index = i * 2;
      unsigned int right_index = left_index + 1;

      outputs[TRACK_OUTPUTS + left_index].setVoltage(track_left_output);
      outputs[TRACK_OUTPUTS + right_index].setVoltage(track_right_output);

      // Calculate summed output
      mix_left_output += track_left_output;
      mix_right_output += track_right_output;

      selected_memory_slot->tracks[i].incrementSamplePosition(args.sampleRate);
    }

    // Read master volume knob
    float master_volume = params[MASTER_VOLUME].getValue() * 8.0;

    // Summed output voltages at stereo outputs
    outputs[AUDIO_OUTPUT_LEFT].setVoltage(mix_left_output * master_volume);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(mix_right_output * master_volume);

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

    if(expander_connected) writeToExpander();
  }

  void readFromExpander(float rack_sample_rate)
  {
    // Receive message from expander.  Always read from the consumer.
    // when reading from the expander, we're using the __GrooveBox's__ consumer and producer message pair
    ExpanderToGrooveboxMessage *consumer_message = (ExpanderToGrooveboxMessage *) leftExpander.consumerMessage;

    // Retrieve the data from the expander
    if(consumer_message && consumer_message->message_received == false)
    {
      this->any_track_soloed = false;

      // We'll need to know if any track is soloed to decide if an un-soloed
      // track should be faded out.  Here, we loop through each solo value
      // provided by the expander to find out.

      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        if(this->solos[i]) this->any_track_soloed = true;
      }

      // Iterate over each track's information sent by the expander.
      // Copy the information sent from the expander into the variables
      // this->mutes and this->solos, which will later be used by the tracks
      // to determine if they should trigger or not.
      //
      // When a track should stop playback based on the mute and solo
      // configuration, then we fade out the track so there's not a jarring
      // experience.

      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        bool expander_mute_value = consumer_message->mutes[i];
        bool expander_solo_value = consumer_message->solos[i];

        // Shorthand to make code more readable
        Track *track = &this->selected_memory_slot->tracks[i];

        if((track->sample_player->playing == true) && (! track->fading_out))
        {
          bool fade_out = false;

          if(any_track_soloed && (expander_solo_value == false)) fade_out = true;
          if(expander_mute_value == true && (expander_solo_value == false)) fade_out = true;

          if (fade_out) track->fadeOut(rack_sample_rate);
        }

        this->mutes[i] = expander_mute_value;
        this->solos[i] = expander_solo_value;
        this->track_volumes[i] = consumer_message->track_volumes[i];

        this->selected_memory_slot->tracks[i].setTrackPan(consumer_message->track_pans[i]);
        this->selected_memory_slot->tracks[i].setTrackPitch(consumer_message->track_pitches[i]);
      }

      // Set the received flag
      consumer_message->message_received = true;
    }

    leftExpander.messageFlipRequested = true;
  }

  void writeToExpander()
  {
    // Always write to the producerMessage
    GrooveboxToExpanderMessage *groovebox_to_expander_message = (GrooveboxToExpanderMessage *) leftExpander.module->rightExpander.producerMessage;

    if(groovebox_to_expander_message && groovebox_to_expander_message->message_received == true)
    {
      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        groovebox_to_expander_message->track_triggers[i] = this->track_triggers[i];
        if(this->track_triggers[i]) this->track_triggers[i] = false;
      }

      groovebox_to_expander_message->message_received = false;
    }
  }

  void detachExpander()
  {
    this->any_track_soloed = false;

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      this->mutes[i] = false;
      this->solos[i] = false;
      this->track_volumes[i] = 1.0;
    }

    for(unsigned int m=0; m < NUMBER_OF_MEMORY_SLOTS; m++)
    {
      for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
      {
        this->memory_slots[m].tracks[i].setTrackPan(0.0);
        this->memory_slots[m].tracks[i].setTrackPitch(0.0);
      }
    }
  }

};
