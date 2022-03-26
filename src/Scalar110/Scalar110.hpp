//
// Where I left off.
//
// - bring parameter meaning to the sample players
//
// *
// * Step selection using shift key
// * sample pitch controls?
// * figure out ratcheting / pattern ratcheting
// * context aware copy/paste
// * save and load root_directory so you always start loading samples in the right place
// * add gate sequencing to LCD for speed ?? (but it wouldn't change per parameter)
// * ability to name tracks?

struct Scalar110 : Module
{
  dsp::SchmittTrigger drum_pad_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger step_select_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger stepTrigger;
  Track tracks[NUMBER_OF_TRACKS];
  Track *selected_track = NULL;
  unsigned int track_index = 0;
  unsigned int old_track_index = 0;
  unsigned int playback_step = 0;
  unsigned int selected_step = 0;
  unsigned int selected_parameter = 0;
  unsigned int selected_function = 0;
  float left_output;
  float right_output;
  float track_left_output;
  float track_right_output;

  // Sample related variables
  std::string root_directory;
	std::string path;

  dsp::SchmittTrigger function_button_triggers[6];

  enum ParamIds {
    ENUMS(DRUM_PADS, NUMBER_OF_STEPS),
    ENUMS(STEP_SELECT_BUTTONS, NUMBER_OF_STEPS),
    SAMPLE_OFFSET_KNOB,
    SAMPLE_VOLUME_KNOB,
    SAMPLE_PITCH_KNOB,
    SAMPLE_PAN_KNOB,
    TRACK_SELECT_KNOB,
    ENUMS(STEP_KNOBS, NUMBER_OF_STEPS),
    ENUMS(FUNCTION_BUTTONS, 6),
		NUM_PARAMS
	};
	enum InputIds {
    STEP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
		NUM_OUTPUTS
	};
  enum LightIds {
    ENUMS(DRUM_PAD_LIGHTS, NUMBER_OF_STEPS),
    ENUMS(STEP_SELECT_BUTTON_LIGHTS, NUMBER_OF_STEPS),
    ENUMS(STEP_LOCATION_LIGHTS, NUMBER_OF_STEPS),
    ENUMS(FUNCTION_BUTTON_LIGHTS, 6),
		NUM_LIGHTS
	};

	Scalar110()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for(unsigned int i=0; i < NUMBER_OF_STEPS; i++)
    {
      configParam(DRUM_PADS + i, 0.0, 1.0, 0.0, "drum_button_" + std::to_string(i));
      configParam(STEP_SELECT_BUTTONS + i, 0.0, 1.0, 0.0, "selected_step_" + std::to_string(i));
      configParam(STEP_KNOBS + i, 0.0, 1.0, 0.0, "step_knob_" + std::to_string(i));
    }

    configParam(SAMPLE_OFFSET_KNOB, 0.0, 1.0, 0.0,"sample_offset");
    configParam(SAMPLE_VOLUME_KNOB, 0.0, 1.0, 0.0,"sample_volume");
    configParam(SAMPLE_PITCH_KNOB, 0.0, 1.0, 0.0,"sample_pitch");
    configParam(SAMPLE_PAN_KNOB, 0.0, 1.0, 0.0,"sample_pan");

    // Configure track knob
    configParam(TRACK_SELECT_KNOB, 0.0, NUMBER_OF_TRACKS - 1, 0.0, "Track");
    paramQuantities[TRACK_SELECT_KNOB]->snapEnabled = true;

    // Set default selected track
    selected_track = &tracks[0];
	}


  void updateKnobPositions()
  {
    /*
    params[SAMPLE_OFFSET_KNOB].setValue(selected_track->getOffset(selected_step));
    params[SAMPLE_VOLUME_KNOB].setValue(selected_track->getVolume(selected_step));
    params[SAMPLE_PITCH_KNOB].setValue(selected_track->getPitch(selected_step));
    params[SAMPLE_PAN_KNOB].setValue(selected_track->getPan(selected_step));
    */
  }

  //
	// SAVE module data
  //
	json_t *dataToJson() override
	{
		json_t *json_root = json_object();

    //
    // Save all track data
    //
    json_t *tracks_json_array = json_array();
    for(int track_number=0; track_number<NUMBER_OF_TRACKS; track_number++)
    {
      json_t *steps_json_array = json_array();

      for(int step_index=0; step_index<NUMBER_OF_STEPS; step_index++)
      {
        json_t *step_data = json_object();
        json_object_set(step_data, "trigger", json_integer(this->tracks[track_number].getValue(step_index)));

        // json_t *parameter_json_array = json_array();

        //  json_array_append_new(parameter_json_array, json_real(this->tracks[track_number].getParameter(step_index,parameter_index)));
        json_object_set(step_data, "offset", json_real(this->tracks[track_number].getOffset(step_index)));
        json_object_set(step_data, "volume", json_real(this->tracks[track_number].getVolume(step_index)));
        json_object_set(step_data, "pitch", json_real(this->tracks[track_number].getPitch(step_index)));
        json_object_set(step_data, "pan", json_real(this->tracks[track_number].getPan(step_index)));

        // json_object_set(step_data, "p", parameter_json_array);

        json_array_append_new(steps_json_array, step_data);
      }

      json_t *track_data = json_object();

      json_object_set(track_data, "steps", steps_json_array);

      // Save the file name and path of the loaded sample.  This might be blank
      std::string filename = this->tracks[track_number].sample_player.getFilename();
      std::string path = this->tracks[track_number].sample_player.getPath();

      json_object_set(track_data, "sample_filename", json_string(filename.c_str()));
      json_object_set(track_data, "sample_path", json_string(path.c_str()));

      json_array_append_new(tracks_json_array, track_data);
    }
    json_object_set(json_root, "tracks", tracks_json_array);
    json_decref(tracks_json_array);

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
    // Load all track data
    //

    json_t *tracks_arrays_data = json_object_get(json_root, "tracks");

    if(tracks_arrays_data)
    {
      size_t track_index;
      size_t step_index;
      // size_t parameter_index;
      // json_t *json_steps_array;
      json_t *json_step_object;
      json_t *json_track_object;

      json_array_foreach(tracks_arrays_data, track_index, json_track_object)
      {
        json_t *steps_json_array = json_object_get(json_track_object, "steps");

        if(steps_json_array)
        {
          json_array_foreach(steps_json_array, step_index, json_step_object)
          {
            // First, read the trigger state
            json_t *trigger_json = json_object_get(json_step_object, "trigger");
            if(trigger_json) this->tracks[track_index].setValue(step_index, json_integer_value(trigger_json));

            json_t *offset_json = json_object_get(json_step_object, "offset");
            if(offset_json) this->tracks[track_index].setOffset(step_index, json_real_value(offset_json));

            json_t *volume_json = json_object_get(json_step_object, "volume");
            if(volume_json) this->tracks[track_index].setVolume(step_index, json_real_value(volume_json));

            json_t *pitch_json = json_object_get(json_step_object, "pitch");
            if(pitch_json) this->tracks[track_index].setPitch(step_index, json_real_value(pitch_json));

            json_t *pan_json = json_object_get(json_step_object, "pan");
            if(pan_json) this->tracks[track_index].setPan(step_index, json_real_value(pan_json));
          }
        }

        // json_t *sample_filename_json = json_object_get(json_track_object, "sample_filename");
        json_t *sample_path_json = json_object_get(json_track_object, "sample_path");

        if(sample_path_json)
        {
          std::string path = json_string_value(sample_path_json);
          if(path != "") this->tracks[track_index].sample_player.loadSample(path);
        }
      }
    }

    updateKnobPositions();
	}


	void process(const ProcessArgs &args) override
	{
    // selected_track = &tracks[(int) params[TRACK_SELECT_KNOB].getValue()];

    track_index = params[TRACK_SELECT_KNOB].getValue();
    if(track_index != old_track_index)
    {
      old_track_index = track_index;
      selected_track = &tracks[track_index];
      updateKnobPositions();
    }


    //
    // Handle drum pads, drum location, and drum selection interactions and lights.
    //

    for(unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      // Process drum pads
      if(drum_pad_triggers[step_number].process(params[DRUM_PADS + step_number].getValue()))
      {
        // Toggle the drum pad
        selected_track->toggleStep(step_number);

        // Also set the step selection for convenience
        // selected_step = step_number;
        // updateKnobPositions();
      }

      // Process step select triggers.
      /*
      if(step_select_triggers[step_number].process(params[STEP_SELECT_BUTTONS + step_number].getValue()))
      {
        selected_step = step_number;
        updateKnobPositions();
      }
      */

      // Light up drum pads
      lights[DRUM_PAD_LIGHTS + step_number].setBrightness(selected_track->getValue(step_number));

      // Light up step selection light
      lights[STEP_SELECT_BUTTON_LIGHTS + step_number].setBrightness(selected_step == step_number);

      // Show location
      lights[STEP_LOCATION_LIGHTS + step_number].setBrightness(playback_step == step_number);
    }

    for(unsigned int i=0; i<6; i++)
    {
      if(function_button_triggers[i].process(params[FUNCTION_BUTTONS + i].getValue()))
      {
        selected_function = i;
      }
    }

    /*
    selected_track->setOffset(selected_step, params[SAMPLE_OFFSET_KNOB].getValue());
    selected_track->setVolume(selected_step, params[SAMPLE_VOLUME_KNOB].getValue());
    selected_track->setPitch(selected_step, params[SAMPLE_PITCH_KNOB].getValue());
    selected_track->setPan(selected_step, params[SAMPLE_PAN_KNOB].getValue());
    */

    // Here's where I need to use a switch statement, such as
    // If the offset is the active editing button, then:
    for(unsigned int step_number = 0; step_number < NUMBER_OF_STEPS; step_number++)
    {
      if(selected_function == 0) selected_track->setOffset(step_number, params[STEP_KNOBS + step_number].getValue());
      if(selected_function == 1) selected_track->setPan(step_number, params[STEP_KNOBS + step_number].getValue());
    }

    //
    // compute output and step the sample playback position
    // The return result from tne engine should be -5v to 5v
    left_output = 0;
    right_output = 0;

    for(unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      std::tie(track_left_output, track_right_output) = tracks[i].getStereoOutput();
      left_output += track_left_output;
      right_output += track_right_output;
      tracks[i].incrementSamplePosition(args.sampleRate);
    }

    // Output voltages at stereo outputs
    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_output);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_output);

    // Process Step Input
    if(stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      // Step all of the tracks
      for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
      {
        tracks[i].step();
      }

      // Step the visual playback indicator as well
      playback_step = (playback_step + 1) % NUMBER_OF_STEPS;
    }

    for(unsigned int i=0; i<6; i++)
    {
      lights[FUNCTION_BUTTON_LIGHTS + i].setBrightness(selected_function == i);
    }
  }
};
