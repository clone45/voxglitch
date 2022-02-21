//
// Next: It looks like I've figured out the laoding parameters part.  Next
// I need to save and load the selected engine per/track.  This should be
// relatively straightforward


struct Scalar110 : Module
{
  dsp::SchmittTrigger drum_pad_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger step_select_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger stepTrigger;
  Track tracks[NUMBER_OF_TRACKS];
  Track *selected_track;
  unsigned int track_index;
  unsigned int playback_step = 0;
  unsigned int selected_step = 0;
  unsigned int engine_index = 0;
  unsigned int old_engine_index = 0;
  unsigned int old_track_index = 0;
  StepParams step_parameters;
  float left_output;
  float right_output;

  enum ParamIds {
    ENUMS(DRUM_PADS, NUMBER_OF_STEPS),
    ENUMS(STEP_SELECT_BUTTONS, NUMBER_OF_STEPS),
    ENUMS(ENGINE_PARAMS, NUMBER_OF_PARAMETERS),
    TRACK_SELECT_KNOB,
    ENGINE_SELECT_KNOB,
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
		NUM_LIGHTS
	};

	Scalar110()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    for(unsigned int i=0; i < NUMBER_OF_STEPS; i++)
    {
      configParam(DRUM_PADS + i, 0.0, 1.0, 0.0, "drum_button_" + std::to_string(i));
      configParam(STEP_SELECT_BUTTONS + i, 0.0, 1.0, 0.0, "selected_step_" + std::to_string(i));
    }

    for(unsigned int i=0; i < NUMBER_OF_PARAMETERS; i++)
    {
      configParam(ENGINE_PARAMS + i, 0.0, 1.0, 1.0, "engine_parameter_" + std::to_string(i));
    }

    // Configure track knob
    configParam(TRACK_SELECT_KNOB, 0.0, NUMBER_OF_TRACKS - 1, 0.0, "Track");
    paramQuantities[TRACK_SELECT_KNOB]->snapEnabled = true;

    // Configure engine selection knob (to be replaced with menu?)
    configParam(ENGINE_SELECT_KNOB, 0.0, 2.0, 0.0, "Engine");
    paramQuantities[ENGINE_SELECT_KNOB]->snapEnabled = true;

    // Set default selected track
    selected_track = &tracks[0];

    // TODO: Remove this line
    selected_track->setEngine(new Foo());
	}

  void selectStep(unsigned int i)
  {
    selected_step = i;
    for(unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETERS; parameter_number++)
    {
      // set the knob positions for the selected step
      params[ENGINE_PARAMS + parameter_number].setValue(selected_track->step_parameters[selected_step].p[parameter_number]);
    }
  }

  void switchTrack(unsigned int track_index)
  {
    selected_track = &tracks[track_index];

    // Set all of the parameter knobs of the selected step to the correct position
    for(unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETERS; parameter_number++)
    {
      params[ENGINE_PARAMS + parameter_number].setValue(selected_track->step_parameters[selected_step].p[parameter_number]);
    }
  }

  void refreshKnobs()
  {
    // Set all of the parameter knobs of the selected step to the correct position
    for(unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETERS; parameter_number++)
    {
      params[ENGINE_PARAMS + parameter_number].setValue(selected_track->step_parameters[selected_step].p[parameter_number]);
    }
  }

	// Autosave module data.  VCV Rack decides when this should be called.
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
        json_object_set(step_data, "trigger_value", json_integer(this->tracks[track_number].getValue(step_index)));

        // I could have saved the parameters as named kay/value pairs, but there's going to be a LOT of them,
        // so I'm being contientious and storing them in a minimal-format
        json_t *parameter_json_array = json_array();
        for(int parameter_index = 0; parameter_index < 4; parameter_index++)
        {
          json_array_append_new(parameter_json_array, json_real(this->tracks[track_number].getParameter(step_index,parameter_index)));
        }

        json_object_set(step_data, "p", parameter_json_array);

        json_array_append_new(steps_json_array, step_data);
      }

      json_array_append_new(tracks_json_array, steps_json_array);
    }
    json_object_set(json_root, "tracks", tracks_json_array);
    json_decref(tracks_json_array);

		return json_root;
	}

	// Load module data
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
      size_t parameter_index;
      json_t *json_steps_array;
      json_t *json_step_object;

      json_array_foreach(tracks_arrays_data, track_index, json_steps_array)
      {
        json_array_foreach(json_steps_array, step_index, json_step_object)
        {
          // First, read the trigger state
          this->tracks[track_index].setValue(step_index, json_integer_value(json_object_get(json_step_object, "trigger_value")));

          // Secondly, read the parameters
          json_t *parameter_json_array = json_object_get(json_step_object, "p");
          json_t *parameter_array_data;

          json_array_foreach(parameter_json_array, parameter_index, parameter_array_data)
          {
            float parameter_value = json_real_value(parameter_array_data);
            DEBUG(std::to_string(parameter_value).c_str());
            this->tracks[track_index].setParameter(step_index, parameter_index, parameter_value);
          }
        }
      }
      switchTrack(0);
    }

	}

	void process(const ProcessArgs &args) override
	{
    // selected_track = &tracks[(int) params[TRACK_SELECT_KNOB].getValue()];
    track_index = params[TRACK_SELECT_KNOB].getValue();
    if(track_index != old_track_index)
    {
      switchTrack(track_index);
      old_track_index = track_index;
    }

    // Read the engine selection knob
    engine_index = params[ENGINE_SELECT_KNOB].getValue() * (NUMBER_OF_ENGINES - 1);

    // If the engine selection has changed, then assign the newly selected
    // engine to the currently selected track.
    if(engine_index != old_engine_index)
    {
      switch(engine_index) {
        case 0:
          selected_track->setEngine(new Foo());
          break;
        case 1: //
          selected_track->setEngine(new LowDrums());
          break;
        default:
          selected_track->setEngine(new Foo());
          break;
      }
      old_engine_index = engine_index;
    }

    //
    // Handle drum pads, drum location, and drum selection interactions and
    // lights.
    //
    for(unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
    {
      // Process drum pads
      if(drum_pad_triggers[i].process(params[DRUM_PADS + i].getValue()))
      {
        // Toggle the drum pad
        selected_track->toggleStep(i);

        // Also set the step selection for convenience
        selectStep(i);
      }

      // Process step select triggers.
      if(step_select_triggers[i].process(params[STEP_SELECT_BUTTONS + i].getValue()))
      {
        selectStep(i);
      }

      // Light up drum pads
      lights[DRUM_PAD_LIGHTS + i].setBrightness(selected_track->getValue(i));

      // Light up step selection light
      lights[STEP_SELECT_BUTTON_LIGHTS + i].setBrightness(selected_step == i);

      // Show location
      lights[STEP_LOCATION_LIGHTS + i].setBrightness(playback_step == i);
    }

    //
    // Read all of the parameter knobs and assign them to the selected Track/Step
    for(unsigned int i = 0; i < NUMBER_OF_PARAMETERS; i++)
    {
      // Let's dive into this next line.
      // * Selected_track is a pointer to a Track.
      // * A track contains an array of StepParams, one element for each step
      // * StepParams is a structure containing an array "p" of 8 floats
      // So this next line is setting the active playback step to the current
      // knob positions for the selected track.
      //
      selected_track->step_parameters[selected_step].p[i] = params[ENGINE_PARAMS + i].getValue();
    }

    //
    // compute output
    // The return result from tne engine should be -5v to 5v
    std::tie(left_output, right_output) = selected_track->process();

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
  }
};
