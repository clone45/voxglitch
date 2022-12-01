//
// Voxglitch Groovebox
//
// By Bret Truchan
//
// - Thank you to all the friendly people on the VCV Rack Community for answering
//   my questions and providing feedback on early builds.
//
// - Thank you to Jim Allman (https://ibang.com/) for his incredible front panel design.
//
// TODO: 
// 
//  - Use a real delay library that de-clicks.  My hack solution is prone to clicks and pops.
//  - Fix missing buttons in library browser
//  - Get expander buttons to glow
//
// Quick overview of the architecture:
//
// - The groovebox contains memory slots.  
// - The memory slots contain 8 Track objects each
// - Some resources, such as the sample players, slew limiters, and delay 
//   buffers are shared amongst tracks and are passed in as pointers to the tracks.

#include <thread>
#include <future>

struct GrooveBox : VoxglitchSamplerModule
{
  MemorySlot memory_slots[NUMBER_OF_MEMORY_SLOTS];

  // Schmitt Triggers
  dsp::BooleanTrigger memory_slot_button_triggers[NUMBER_OF_MEMORY_SLOTS];
  dsp::BooleanTrigger parameter_lock_button_triggers[NUMBER_OF_PARAMETER_LOCKS];
  dsp::BooleanTrigger copy_button_trigger;
  dsp::BooleanTrigger paste_button_trigger;
  dsp::BooleanTrigger step_trigger;
  dsp::BooleanTrigger reset_trigger;

  // Pointers to select track and memory
  Track *selected_track = NULL;
  MemorySlot *selected_memory_slot = NULL;

  // Assorted variables
  unsigned int memory_slot_index = 0;
  unsigned int copied_memory_index = 0;
  unsigned int track_index = 0;
  unsigned int playback_step = 0;
  unsigned int selected_parameter_lock_id = 0;
  unsigned int selected_parameter_slot_id = 0;
  unsigned int clock_division = 8;
  unsigned int clock_counter = clock_division;
  bool first_step = true;
  bool shift_key = false;

  std::array<bool, NUMBER_OF_TRACKS> mutes{};
  std::array<bool, NUMBER_OF_TRACKS> solos{};
  
  bool any_track_soloed = false;
  bool expander_connected = false;

  unsigned int copied_step_index = 0;
  unsigned int lcd_screen_mode = TRACK;

  // These booleans tell the sequence position lights whether to be ON or OFF
  std::array<bool, NUMBER_OF_STEPS> light_booleans{};
  std::array<bool, NUMBER_OF_STEPS> inner_light_booleans{};

  unsigned int visualizer_step = 0;
  unsigned int sample_position_snap_track_values[NUMBER_OF_TRACKS];

  // A pair of GrooveBoxExpanderMessage structures for sending information
  // from the expander to the groovebox.  Note that they both essentially
  // serve the same purpose, but are rotated during read/write cycles
  ExpanderToGrooveboxMessage expander_to_groovebox_message_a;
  ExpanderToGrooveboxMessage expander_to_groovebox_message_b;
  std::array<float, NUMBER_OF_TRACKS> track_volumes{};
  
  // The track_triggers array is used to send trigger information to the
  // expansion module.  Once a trigger is sent, it's immediately set back to zero.
  std::array<bool, NUMBER_OF_TRACKS> track_triggers{};

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
  // TODO: remove the loaded_filenames array, which is not necessary
  std::string loaded_filenames[NUMBER_OF_TRACKS]; // for display on the front panel
  std::string path;

  // Each of the 8 tracks has dedicated sample player engines
  // The samples assigned to each track are NOT dependent on which memory bank
  // is active.  This means that you cannot load samples in one memory bank without
  // it affecting all memory banks.

  SamplePlayer sample_players[NUMBER_OF_TRACKS];

  // Each of the 8 tracks has dedicated slew limiters:
  rack::dsp::SlewLimiter volume_slew_limiters[NUMBER_OF_TRACKS];
  rack::dsp::SlewLimiter pan_slew_limiters[NUMBER_OF_TRACKS];
  rack::dsp::SlewLimiter filter_cutoff_slew_limiters[NUMBER_OF_TRACKS];
  rack::dsp::SlewLimiter filter_resonance_slew_limiters[NUMBER_OF_TRACKS];

  SimpleDelay delay_dsps[NUMBER_OF_TRACKS];

  // sample position snap settings
  std::array<unsigned int, NUMBER_OF_TRACKS> sample_position_snap_indexes{};

  //
  // Basic VCV Parameter, Input, Output, and Light definitions
  //

  enum ParamIds
  {
    ENUMS(DRUM_PADS, NUMBER_OF_STEPS),
    ENUMS(STEP_SELECT_BUTTONS, NUMBER_OF_STEPS),
    ENUMS(STEP_KNOBS, NUMBER_OF_STEPS),
    ENUMS(PARAMETER_LOCK_BUTTONS, NUMBER_OF_PARAMETER_LOCKS),
    ENUMS(DUMMY_UNUSED_PARAMS, NUMBER_OF_TRACKS),
    ENUMS(MEMORY_SLOT_BUTTONS, NUMBER_OF_MEMORY_SLOTS),
    COPY_BUTTON,
    PASTE_BUTTON,
    MASTER_VOLUME,
    NUM_PARAMS
  };
  enum InputIds
  {
    STEP_INPUT,
    RESET_INPUT,
    MEM_INPUT,
    NUM_INPUTS
  };
  enum OutputIds
  {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
    ENUMS(TRACK_OUTPUTS, NUMBER_OF_TRACKS * 2),
    NUM_OUTPUTS
  };
  enum LightIds
  {
    NUM_LIGHTS
  };

  enum LCD_MODES
  {
    TRACK,
    SAMPLE,
    RATCHET,
    UPDATE,
    INTRO
  };

  unsigned int left_output_enum_lookup_table[NUMBER_OF_TRACKS] = { 
    TRACK_OUTPUTS + 0, 
    TRACK_OUTPUTS + 2, 
    TRACK_OUTPUTS + 4, 
    TRACK_OUTPUTS + 6, 
    TRACK_OUTPUTS + 8, 
    TRACK_OUTPUTS + 10,
    TRACK_OUTPUTS + 12,
    TRACK_OUTPUTS + 14
  };

  unsigned int right_output_enum_lookup_table[NUMBER_OF_TRACKS] = {
    TRACK_OUTPUTS + 1, 
    TRACK_OUTPUTS + 3, 
    TRACK_OUTPUTS + 5, 
    TRACK_OUTPUTS + 7, 
    TRACK_OUTPUTS + 9, 
    TRACK_OUTPUTS + 11,
    TRACK_OUTPUTS + 13,
    TRACK_OUTPUTS + 15
  };


  /*
 

    █▀▀ █▀█ █▄░█ █▀ ▀█▀ █▀█ █░█ █▀▀ ▀█▀ █▀█ █▀█
    █▄▄ █▄█ █░▀█ ▄█ ░█░ █▀▄ █▄█ █▄▄ ░█░ █▄█ █▀▄
    Text created using https://fsymbols.com/generators/carty/

  */

  GrooveBox()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // Configure all of the step buttons and parameter lock knobs
    for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
    {
      configParam(DRUM_PADS + i, 0.0, 1.0, 0.0, "Step Button");
      configOnOff(DRUM_PADS + i, 0.0, "Step Button");
      configParam(STEP_KNOBS + i, 0.0, 1.0, 0.0, "Parameter Lock Value " + std::to_string(i));
      configParam(PARAMETER_LOCK_BUTTONS + i, 0.0, 1.0, 0.0);
      configOnOff(PARAMETER_LOCK_BUTTONS + i, 0.0, PARAMETER_LOCK_NAMES[i]);
    }


    // Configure slew limiters
    // 
    //  Be careful when setting the slew limiter values.  You'd think that 
    //  larger numbers would increase the slew time, but it's actually the opposite.
    //
    //  See: https://community.vcvrack.com/t/difficulty-understanding-how-to-use-the-dsp-library-in-the-sdk/16225/3?u=clone45
    //  
    //  const float MS_1{1000.0f}; // 1000 Hz = 1 millisecond
    //  const float MS_5{200.0f};  // 200 Hz  = 5 milliseconds
    //  const float MS_10{100.0f}; // 100 Hz  = 10 milliseconds
    //  const float MS_25{40.0f};  // 40 Hz   = 25 milliseconds
    //  const float MS_50{20.0f};  // 20 Hz   = 50 milliseconds
    //  const float MS_100{10.0f}; // 10 Hz   = 100 milliseconds
        
    float slew_speed = 100.0f;
    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      volume_slew_limiters[i].setRiseFall(slew_speed, slew_speed);
      pan_slew_limiters[i].setRiseFall(slew_speed, slew_speed);
      filter_cutoff_slew_limiters[i].setRiseFall(slew_speed, slew_speed);
      filter_resonance_slew_limiters[i].setRiseFall(slew_speed, slew_speed);
    }

    // Configure delay dsps
    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      delay_dsps[i].setBufferSize(APP->engine->getSampleRate() / 30.0);
    }

    // Configure the individual track outputs
    for (unsigned int i = 0; i < (NUMBER_OF_TRACKS * 2); i += 2)
    {
      configOutput(TRACK_OUTPUTS + i, "Track " + std::to_string((i / 2) + 1) + ": left");
      configOutput(TRACK_OUTPUTS + i + 1, "Track " + std::to_string((i / 2) + 1) + ": right");
    }

    configInput(STEP_INPUT, "Clock Input x32");
    configInput(RESET_INPUT, "Reset Input");
    configInput(MEM_INPUT, "Memory CV Select Input (0-10v)");

    // Configure the stereo mix outputs
    configOutput(AUDIO_OUTPUT_LEFT, "Left Mix");
    configOutput(AUDIO_OUTPUT_RIGHT, "Right Mix");

    // Create tooltips for the memory slots
    for (unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      configParam(MEMORY_SLOT_BUTTONS + i, 0.0, 1.0, 0.0);
      configOnOff(MEMORY_SLOT_BUTTONS + i, 0.0, "Memory Slot #" + std::to_string(i + 1));
    }

    // Configure the master output knob
    configParam(MASTER_VOLUME, 0.0, 1.0, 0.5, "Master Volume");
    paramQuantities[MASTER_VOLUME]->randomizeEnabled = false;

    //
    // Some objects, such as sample players, slew limiters, and delays apply
    // to tracks.  However, having an instance of each for each track is undesireable.
    // It's much better to share these resources amongst tracks.  Here's the code
    // which sends the tracks pointers to the resources.
    //
    for (unsigned int m = 0; m < NUMBER_OF_MEMORY_SLOTS; m++)
    {
      for (unsigned int t = 0; t < NUMBER_OF_TRACKS; t++)
      {
        // There are 8 sample players, one for each track.  These sample players
        // are shared across the tracks contained in the memory slots.
        memory_slots[m].setSamplePlayer(t, &sample_players[t]);

        // Similarly, set the shared slew limiters
        memory_slots[m].setVolumeSlewLimiter(t, &volume_slew_limiters[t]);
        memory_slots[m].setPanSlewLimiter(t, &pan_slew_limiters[t]);
        memory_slots[m].setFilterCutoffSlewLimiter(t, &filter_cutoff_slew_limiters[t]);
        memory_slots[m].setFilterResonanceSlewLimiter(t, &filter_resonance_slew_limiters[t]);

        // Same idea with the dsp::delay objects
        SimpleDelay *simple_delay = &delay_dsps[t];
        memory_slots[m].setDelayDsp(t, simple_delay);
      }
    }

    // Store a pointer to the active memory slot
    selected_memory_slot = &memory_slots[0];

    // Store a pointer to the active track
    selected_track = selected_memory_slot->getTrack(0);

    // Update parameter lock knobs.  I'm not sure if this is necessary.
    updatePanelControls();

    // Set the leftExpander pointers to the two message holders
    leftExpander.producerMessage = &expander_to_groovebox_message_a;
    leftExpander.consumerMessage = &expander_to_groovebox_message_b;
  }

  /*
 
    █░█ █▀▀ █░░ █▀█ █▀▀ █▀█   █▀▀ █░█ █▄░█ █▀▀ ▀█▀ █ █▀█ █▄░█ █▀
    █▀█ ██▄ █▄▄ █▀▀ ██▄ █▀▄   █▀░ █▄█ █░▀█ █▄▄ ░█░ █ █▄█ █░▀█ ▄█
    Text created using https://fsymbols.com/generators/carty/

  */

  // copyMemory(src_index, dst_index)
  // Helper function to copy one memory slot to another memory slot
  void copyMemory(unsigned int src_index, unsigned int dst_index)
  {
    memory_slots[dst_index].copy(&memory_slots[src_index]);
    updatePanelControls();
  }

  // pasteStep(src_index, dst_index)
  // Helper function to copy one step to another, including all parameter locks
  void pasteStep(unsigned int dst_index)
  {
    selected_track->copyStep(this->copied_step_index, dst_index);
    updatePanelControls();
  }

  void clearStepParameters(unsigned int step_index)
  {
    selected_track->clearStepParameters(step_index);
    updatePanelControls();
  }

  void initialize()
  {
    for (unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      memory_slots[i].initialize();
    }

    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      this->loaded_filenames[i] = "";
      this->sample_position_snap_indexes[i] = 0;
    }

    this->updatePanelControls();
  }

  void updatePanelControls()
  {
    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      float value = 0;

      value = selected_track->getParameter(selected_parameter_lock_id, step_number);

      params[STEP_KNOBS + step_number].setValue(value);
      params[DRUM_PADS + step_number].setValue(selected_track->getValue(step_number));
    }

    // Update selected function button
    for (unsigned int slot_id = 0; slot_id < NUMBER_OF_PARAMETER_LOCKS; slot_id++)
    {
      unsigned int parameter_id = parameter_slots[slot_id];
      params[PARAMETER_LOCK_BUTTONS + parameter_id].setValue(selected_parameter_slot_id == slot_id);
    }
  }


  // void switchMemory(unsigned int new_memory_slot)
  // 
  // What happens when the user switches memory?
  //
  // #1. There's a pointer to a memory structure called "selected_memory_slot".
  //     This pointer points to the new memory slot.
  // 
  // #2. Each memory holds 8 tracks.  None of these tracks are stepped unless
  //     they belong to the active memory slot.  So, after switching memory
  //     slots, all 8 tracks need to have their "position" set to the current
  //     step.
  //
  // #3. The selected memory slot needs to be highlighted on the front panel.
  //     This is achieved by setting the param[MEMORY_SLOT_BUTTONS + i] = true
  //     for the selected memory slot.
  //
  // #4. The panel controls (knobs) need to be positioned to match the track's
  //     new values.
  // 
  void switchMemory(unsigned int new_memory_slot)
  {
    memory_slot_index = new_memory_slot;

    // Switch memory_slots and set the selected track
    selected_memory_slot = &memory_slots[new_memory_slot];
    selected_track = selected_memory_slot->getTrack(this->track_index);

    // set all track positions
    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      selected_memory_slot->tracks[i].setPosition(playback_step);
    }

    for (unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      params[MEMORY_SLOT_BUTTONS + i].setValue(memory_slot_index == i);
    }

    updatePanelControls();
  }

  //
  // Track helper functions
  // 

  void selectTrack(unsigned int new_active_track)
  {
    track_index = new_active_track;
    selected_track = selected_memory_slot->getTrack(track_index);

    updatePanelControls();
  }

  void shiftAllTracks(unsigned int amount)
  {
    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      selected_memory_slot->tracks[i].shift(amount);
    }
    updatePanelControls();
  }

  void shiftTrack(unsigned int amount)
  {
    selected_memory_slot->tracks[this->track_index].shift(amount);
    updatePanelControls();
  }

  bool trigger(unsigned int track_id)
  {
    unsigned int sample_position_snap_value = sample_position_snap_track_values[track_id];
    if (notMuted(track_id))
      return (selected_memory_slot->tracks[track_id].trigger(sample_position_snap_value));
    return (false);
  }

  bool notMuted(unsigned int track_id)
  {
    if (any_track_soloed)
      return (solos[track_id]);
    return (!mutes[track_id]);
  }  

  void randomizeSteps()
  {
    this->selected_track->randomizeSteps();
    updatePanelControls();
  }

  void clearSteps()
  {
    this->selected_track->clearSteps();
    updatePanelControls();
  }

  void clearTrackSteps(unsigned int track_id)
  {
    Track *track = this->selected_memory_slot->getTrack(track_id);
    track->clearSteps();
    updatePanelControls();
  }

  void clearTrackParameters(unsigned int track_id)
  {
    Track *track = this->selected_memory_slot->getTrack(track_id);
    track->clearParameters();
    updatePanelControls();
  }

  void clearTrack(unsigned int track_id)
  {
      Track *track = this->selected_memory_slot->getTrack(track_id);
      track->clear();
      updatePanelControls();
  }

  //
  // Knobs and parameter helper functions
  //

  void shiftKnobValuesLeft()
  {
    float temp = params[STEP_KNOBS + 0].getValue();

    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      params[STEP_KNOBS + step_number].setValue(params[STEP_KNOBS + step_number + 1].getValue());
    }

    params[STEP_KNOBS + NUMBER_OF_STEPS - 1].setValue(temp);
  }

  void shiftKnobValuesRight()
  {
    float temp = params[STEP_KNOBS + NUMBER_OF_STEPS - 1].getValue();

    for (unsigned int step_number = NUMBER_OF_STEPS; step_number > 0; step_number--)
    {
      params[STEP_KNOBS + step_number].setValue(params[STEP_KNOBS + step_number - 1].getValue());
    }

    params[STEP_KNOBS + 0].setValue(temp);
  }

  void randomizeSelectedParameter()
  {
    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      params[STEP_KNOBS + step_number].setValue(rand() / double(RAND_MAX));
    }
  }

  void resetSelectedParameter()
  {
    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      params[STEP_KNOBS + step_number].setValue(default_parameter_values[selected_parameter_lock_id]);
    }
  }

  void boostSelectedParameter()
  {
    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      float boosted_value = std::min(1.0, params[STEP_KNOBS + step_number].getValue() + 0.125);
      params[STEP_KNOBS + step_number].setValue(boosted_value);
    }
  }

  void reduceSelectedParameter()
  {
    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      float reduced_value = std::max(0.0, params[STEP_KNOBS + step_number].getValue() - 0.125);
      params[STEP_KNOBS + step_number].setValue(reduced_value);
    }
  }

  void matchSelectedParameter(unsigned int src_index)
  {
    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      if(src_index != step_number)
      {
        params[STEP_KNOBS + step_number].setValue(params[STEP_KNOBS + src_index].getValue());
      }
    }
  }

  void setSamplePositionSnapIndex(unsigned int sample_position_snap_index, unsigned int track_index)
  {
    this->sample_position_snap_indexes[track_index] = sample_position_snap_index;
    this->sample_position_snap_track_values[track_index] = sample_position_snap_values[sample_position_snap_index];
  }

  // This function is called from GrooveboxMemoryButton and is used to ignore
  // user input when a cable is connected to the memory CV input.
  bool memCableIsConnected()
  {
    return(inputs[MEM_INPUT].isConnected());
  }


  /*
 

    █▀ ▄▀█ █░█ █▀▀   ▄▀█ █▄░█ █▀▄   █░░ █▀█ ▄▀█ █▀▄
    ▄█ █▀█ ▀▄▀ ██▄   █▀█ █░▀█ █▄▀   █▄▄ █▄█ █▀█ █▄▀
    Text created using https://fsymbols.com/generators/carty/

  */

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

    for (unsigned int track_number = 0; track_number < NUMBER_OF_TRACKS; track_number++)
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
    for (int memory_slot_number = 0; memory_slot_number < NUMBER_OF_MEMORY_SLOTS; memory_slot_number++)
    {
      // Save all track data
      json_t *tracks_json_array = json_array();
      for (int track_number = 0; track_number < NUMBER_OF_TRACKS; track_number++)
      {
        json_t *steps_json_array = json_array();

        for (int step_index = 0; step_index < NUMBER_OF_STEPS; step_index++)
        {
          json_t *step_data = json_object();
          json_object_set(step_data, "trigger", json_integer(this->memory_slots[memory_slot_number].tracks[track_number].getValue(step_index)));

          for(unsigned int parameter_index = 0; parameter_index < NUMBER_OF_PARAMETER_LOCKS; parameter_index++)
          {
            std::string key = PARAMETER_LOCK_NAMES[parameter_index];
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            std::replace( key.begin(), key.end(), ' ', '_'); // replace all ' ' to '_'

            json_object_set(step_data, key.c_str(), json_real(this->memory_slots[memory_slot_number].tracks[track_number].getParameter(parameter_index, step_index)));
          }
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

    // Save selected color theme
    json_object_set(json_root, "selected_color_theme", json_integer(LCDColorScheme::selected_color_scheme));
    json_object_set(json_root, "selected_memory_index", json_integer(memory_slot_index));

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

    if (shared_track_data)
    {
      size_t track_index;
      json_t *json_track_data_object;

      json_array_foreach(shared_track_data, track_index, json_track_data_object)
      {
        json_t *sample_path_json = json_object_get(json_track_data_object, "sample_path");
        if (sample_path_json)
        {
          std::string path = json_string_value(sample_path_json);
          if (path != "")
            this->sample_players[track_index].loadSample(path);
          this->loaded_filenames[track_index] = this->sample_players[track_index].getFilename();
        }

        // Deprecated, but around for a while while people still have old versions
        json_t *offset_snap_index_json = json_object_get(json_track_data_object, "offset_snap_index");
        if (offset_snap_index_json)
        {
          unsigned int offset_snap_index = json_integer_value(offset_snap_index_json);
          setSamplePositionSnapIndex(offset_snap_index, track_index);
        }

        // Newer version that replaces the version above ^
        json_t *sample_position_snap_index_json = json_object_get(json_track_data_object, "sample_position_snap_index");
        if (sample_position_snap_index_json)
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

    if (memory_slots_arrays_data)
    {
      size_t memory_slot_index;
      json_t *json_memory_slot_object;

      json_array_foreach(memory_slots_arrays_data, memory_slot_index, json_memory_slot_object)
      {

        // Load all track data
        json_t *tracks_arrays_data = json_object_get(json_memory_slot_object, "tracks");

        if (tracks_arrays_data)
        {
          size_t track_index;
          size_t step_index;
          json_t *json_step_object;
          json_t *json_track_object;

          json_array_foreach(tracks_arrays_data, track_index, json_track_object)
          {
            // Load track ranges
            json_t *range_end_json = json_object_get(json_track_object, "range_end");
            if (range_end_json)
              this->memory_slots[memory_slot_index].tracks[track_index].setRangeEnd(json_integer_value(range_end_json));

            json_t *range_start_json = json_object_get(json_track_object, "range_start");
            if (range_start_json)
              this->memory_slots[memory_slot_index].tracks[track_index].setRangeStart(json_integer_value(range_start_json));

            //
            // Load all of the step information, including trigger and parameter locks
            //

            json_t *steps_json_array = json_object_get(json_track_object, "steps");

            if (steps_json_array)
            {
              json_array_foreach(steps_json_array, step_index, json_step_object)
              {

                json_t *trigger_json = json_object_get(json_step_object, "trigger");
                if (trigger_json)
                  this->memory_slots[memory_slot_index].tracks[track_index].setValue(step_index, json_integer_value(trigger_json));
                
                // Load all parameter information for all steps
                for(unsigned int parameter_index=0; parameter_index < NUMBER_OF_PARAMETER_LOCKS; parameter_index++)
                {

                  std::string key = PARAMETER_LOCK_NAMES[parameter_index];
                  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                  std::replace( key.begin(), key.end(), ' ', '_'); // replace all ' ' to '_'

                  json_t *parameter_json = json_object_get(json_step_object, key.c_str());
                  if (parameter_json) 
                  {
                    this->memory_slots[memory_slot_index].tracks[track_index].setParameter(parameter_index, step_index, json_real_value(parameter_json));
                  }
                  else
                  {
                    this->memory_slots[memory_slot_index].tracks[track_index].setParameter(parameter_index, step_index, default_parameter_values[parameter_index]);
                  }
                }
              }
            }
          }
        } // end if tracks array data
      }   // end foreach memory slot
    }     // end if memory_slots array data

    json_t *selected_color_theme_json = json_object_get(json_root, "selected_color_theme");
    if (selected_color_theme_json) LCDColorScheme::selected_color_scheme = json_integer_value(selected_color_theme_json);

    json_t *selected_memory_index_json = json_object_get(json_root, "selected_memory_index");
    if(selected_memory_index_json) this->switchMemory(json_integer_value(selected_memory_index_json));

    updatePanelControls();
	}


  /*
 
    █▀█ █▀█ █▀█ █▀▀ █▀▀ █▀ █▀
    █▀▀ █▀▄ █▄█ █▄▄ ██▄ ▄█ ▄█
    Text created using https://fsymbols.com/generators/carty/

  */

  void process(const ProcessArgs &args) override
  {
    if (expander_connected)
      readFromExpander();

    //
    // If the user has pressed a memory slot button, switch memory slots and update the
    // knob positions for the selected function.

    if (inputs[MEM_INPUT].isConnected())
    {
      unsigned int memory_selection = (inputs[MEM_INPUT].getVoltage() / 10.0) * NUMBER_OF_MEMORY_SLOTS;
      memory_selection = clamp(memory_selection, 0, NUMBER_OF_MEMORY_SLOTS - 1);
      if (memory_selection != memory_slot_index)
        switchMemory(memory_selection);
    }
    else
    {
      // Would it be more efficient to move this code into the memory button's
      // onClick method, or something like that?
      for (unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
      {
        if (memory_slot_button_triggers[i].process(params[MEMORY_SLOT_BUTTONS + i].getValue()))
        {
          // If shift-clicking, then copy the current memory into the clicked memory slot
          // before switching to the new memory slot
          if (shift_key)
            copyMemory(memory_slot_index, i);

          switchMemory(i);
        }
      }
    }

    // COPY: If the user has pressed the copy button, then store the index of the
    // current memory, which will be used when pasting.
    if (copy_button_trigger.process(params[COPY_BUTTON].getValue()))
    {
      copied_memory_index = memory_slot_index;
    }

    // PASTE: If the user has pressed the paste button, then copy previously
    // copied memory to the current memory location.
    if (paste_button_trigger.process(params[PASTE_BUTTON].getValue()))
    {
      copyMemory(copied_memory_index, memory_slot_index);
    }

    // On incoming RESET, reset the sequencers
    if (reset_trigger.process(rescale(inputs[RESET_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      // Set up a (reverse) counter so that the clock input will ignore
      // incoming clock pulses for 1 millisecond after a reset input. This
      // is to comply with VCV Rack's standards.  See section "Timing" at
      // https://vcvrack.com/manual/VoltageStandards

      first_step = true;
      clock_counter = clock_division;

      for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
      {
        selected_memory_slot->tracks[i].reset();
      }
    }

    //
    // step button processing
    //

    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      // Process step key-buttons (awesome clackity clack!)
      bool step_button_value = params[DRUM_PADS + step_number].getValue();

      selected_track->setValue(step_number, step_button_value);
      inner_light_booleans[step_number] = step_button_value;

      // Show location
      light_booleans[step_number] = (playback_step == step_number);
    }

    //
    //  Parameter selection

    for (unsigned int slot_id = 0; slot_id < NUMBER_OF_PARAMETER_LOCKS; slot_id++)
    {
      // parameter_slots keep track of this parameter is associated with which parameter slot
      // parameter_slots is defined in defines.h
      unsigned int parameter_id = parameter_slots[slot_id];

      if (parameter_lock_button_triggers[slot_id].process(params[PARAMETER_LOCK_BUTTONS + parameter_id].getValue()))
      {
        selected_parameter_lock_id = parameter_id;
        selected_parameter_slot_id = slot_id;
        updatePanelControls();
      }
    }

    // Process the knobs below the steps.
    for (unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      float value = params[STEP_KNOBS + step_number].getValue();
      selected_track->setParameter(selected_parameter_lock_id, step_number, value);
    }

    //
    // Clock and step features
    //
    if (step_trigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      if (clock_counter == clock_division)
      {
        if (first_step == false) // If not the first step
        {
          // Step all of the tracks
          for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
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
        for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
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
        for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
        {
          if (notMuted(i))
            this->track_triggers[i] = selected_memory_slot->tracks[i].ratchety();
        }
      }
      clock_counter++;
    }

    float mix_left_output = 0;
    float mix_right_output = 0;

    for (unsigned int track_index = 0; track_index < NUMBER_OF_TRACKS; track_index++)
    {
      processTrack(track_index, &mix_left_output, &mix_right_output);
    }

    // Read master volume knob
    float master_volume = params[MASTER_VOLUME].getValue() * 8.0;

    // Summed output voltages at stereo outputs
    outputs[AUDIO_OUTPUT_LEFT].setVoltage(mix_left_output *master_volume);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(mix_right_output *master_volume);

    if (expander_connected)
      writeToExpander();
  }

  bool processTrack(unsigned int track_index, float *mix_left_output, float *mix_right_output)
  {
    //  1. Get the output of the tracks and sum them for the stereo output
    //  2. Once the output has been read, increment the sample position

    float track_left_output = 0;
    float track_right_output = 0;

    // This one line of code takes up most of the CPU
    selected_memory_slot->tracks[track_index].getStereoOutput(&track_left_output, &track_right_output, this->interpolation);

    // Apply track volumes from expander
    if (expander_connected)
    {
      track_left_output = track_left_output * track_volumes[track_index];
      track_right_output = track_right_output * track_volumes[track_index];
    }

    outputs[left_output_enum_lookup_table[track_index]].setVoltage(track_left_output);
    outputs[right_output_enum_lookup_table[track_index]].setVoltage(track_right_output);

    // Calculate summed output
    *mix_left_output += track_left_output;
    *mix_right_output += track_right_output;

    selected_memory_slot->tracks[track_index].incrementSamplePosition();

    return(true);
  }

  /*
 
    █▀▀ ▀▄▀ █▀█ ▄▀█ █▄░█ █▀▄ █▀▀ █▀█   █▀█ █▀█ █▀█ █▀▀ █▀▀ █▀ █▀ █ █▄░█ █▀▀
    ██▄ █░█ █▀▀ █▀█ █░▀█ █▄▀ ██▄ █▀▄   █▀▀ █▀▄ █▄█ █▄▄ ██▄ ▄█ ▄█ █ █░▀█ █▄█
    Text created using https://fsymbols.com/generators/carty/

  */

  void onExpanderChange(const ExpanderChangeEvent &e) override
  {
    if(e.side == false) // false == left, true == right
    {
      if (leftExpander.module && leftExpander.module->model == modelGrooveBoxExpander)
      {
        expander_connected = true;
      }
      else
      {
        if (expander_connected) detachExpander();
        expander_connected = false;
      }
    }
  }

  void readFromExpander()
  {
    // Receive message from expander.  Always read from the consumer.
    // when reading from the expander, we're using the __GrooveBox's__ consumer and producer message pair
    ExpanderToGrooveboxMessage *consumer_message = (ExpanderToGrooveboxMessage *)leftExpander.consumerMessage;

    // Retrieve the data from the expander
    if (consumer_message && consumer_message->message_received == false)
    {
      this->any_track_soloed = false;

      // We'll need to know if any track is soloed to decide if an un-soloed
      // track should be faded out.  Here, we loop through each solo value
      // provided by the expander to find out.

      for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
      {
        if (this->solos[i])
          this->any_track_soloed = true;
      }

      // Iterate over each track's information sent by the expander.
      // Copy the information sent from the expander into the variables
      // this->mutes and this->solos, which will later be used by the tracks
      // to determine if they should trigger or not.
      //
      // When a track should stop playback based on the mute and solo
      // configuration, then we fade out the track so there's not a jarring
      // experience.

      for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
      {
        bool expander_mute_value = consumer_message->mutes[i];
        bool expander_solo_value = consumer_message->solos[i];

        // Shorthand to make code more readable
        Track *track = &this->selected_memory_slot->tracks[i];

        //
        // If the sample is playing and not fading out, then see if the
        // expander settings should stop playback.  If so, start fading out the sound.
        if ((track->sample_player->playing == true) && (!track->isFadingOut()))
        {
          bool fade_out = false;

          // Is any track is soloed and this one is not, then fade out
          if (any_track_soloed && (expander_solo_value == false))
             fade_out = true;

          // If this track is muted but not soloed, then fade it out
          if (expander_mute_value == true && (expander_solo_value == false))
            fade_out = true;

          if (fade_out)
          {
            track->fadeOut();
          }
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
    GrooveboxToExpanderMessage *groovebox_to_expander_message = (GrooveboxToExpanderMessage *)leftExpander.module->rightExpander.producerMessage;

    if (groovebox_to_expander_message && groovebox_to_expander_message->message_received == true)
    {
      for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
      {
        groovebox_to_expander_message->track_triggers[i] = this->track_triggers[i];
        if (this->track_triggers[i])
          this->track_triggers[i] = false;
      }

      groovebox_to_expander_message->message_received = false;
    }
  }

  void detachExpander()
  {
    this->any_track_soloed = false;

    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      this->mutes[i] = false;
      this->solos[i] = false;
      this->track_volumes[i] = 1.0;
    }

    for (unsigned int m = 0; m < NUMBER_OF_MEMORY_SLOTS; m++)
    {
      for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
      {
        this->memory_slots[m].tracks[i].setTrackPan(0.0);
        this->memory_slots[m].tracks[i].setTrackPitch(0.0);
      }
    }
  }

  void onSampleRateChange(const SampleRateChangeEvent &e) override
  {
    for (unsigned int m = 0; m < NUMBER_OF_MEMORY_SLOTS; m++)
    {
      for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
      {
        this->memory_slots[m].tracks[i].updateRackSampleRate();
      }
    }
  }
};
