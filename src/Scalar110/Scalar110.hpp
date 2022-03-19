//
// Where I left off.
//
// - when switching from track 3 to track 2, step parameter values are all set to 0
//   Maybe this is due to the change in engine?
//
// - disable ability to select a parameter not supported by the selected engine
// - also alter the parameter adjustment dislplay to show the name of the parameter
//
// * Engine idea: Beat looper
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
  unsigned int track_index;
  unsigned int old_track_index = 0;
  unsigned int old_engine_index = 0;
  unsigned int playback_step = 0;
  unsigned int lcd_page_index = 0;
  bool selected_steps[NUMBER_OF_STEPS];
  unsigned int selected_step = 0;
  unsigned int selected_parameter = 0;
  float left_output;
  float right_output;
  float track_left_output;
  float track_right_output;
  SampleBank& sample_bank = SampleBank::get_instance();
  StepParams default_params;

  // Sample related variables
  std::string root_directory;
	std::string path;

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
      selected_steps[i] = false;
    }

    for(unsigned int i=0; i < NUMBER_OF_PARAMETERS; i++)
    {
      configParam(ENGINE_PARAMS + i, 0.0, 1.0, 0.0, "engine_parameter_" + std::to_string(i));
    }

    // Configure track knob
    configParam(TRACK_SELECT_KNOB, 0.0, NUMBER_OF_TRACKS - 1, 0.0, "Track");
    paramQuantities[TRACK_SELECT_KNOB]->snapEnabled = true;

    // Configure engine selection knob (to be replaced with menu?)
    configParam(ENGINE_SELECT_KNOB, 0.0, (NUMBER_OF_ENGINES - 1), 0.0, "Engine");
    paramQuantities[ENGINE_SELECT_KNOB]->snapEnabled = true;

    // Each track knows its own track number, which never changes once
    // the module is loaded.  Tracks are permanent.
    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      tracks[i].track_number = i;
    }

    // Set default selected track
    setTrack(0);

    // Set default parameters
    // loadEngineDefaultParams();
	}

  void setParameterKnobPosition(unsigned int parameter_number, float position)
  {
    params[ENGINE_PARAMS + parameter_number].setValue(position);
  }

  void setDefaultParameter(unsigned int parameter_number, float value)
  {
    this->default_params.p[parameter_number] = value;
  }

  void toggleStep(unsigned int i)
  {
    if(selected_step != i)
    {
      this->selectStep(i);
    }
  }

  void setTrack(unsigned int track_index)
  {
    selected_track = &tracks[track_index];
    selectLCDFunctionOnParameterFocus(this->selected_parameter);
  }

  void switchTrack(unsigned int track_index)
  {
    selected_track = &tracks[track_index];

    params[ENGINE_SELECT_KNOB].setValue(selected_track->getEngine());
    old_engine_index = selected_track->getEngine();

    this->updateParameterKnobs();

    // Display the correct LCD display for the selected track
    selectLCDFunctionOnParameterFocus(this->selected_parameter);
  }

  void switchEngine(unsigned int engine_index)
  {
    // This next line also loads the engine default parameters into the track
    selected_track->setEngine(engine_index);
    selected_track->copyEngineDefaults();
    this->updateParameterKnobs();

    // Set the engine knob to the right position
    params[ENGINE_SELECT_KNOB].setValue(engine_index);
  }

  void selectStep(unsigned int which_step)
  {
    this->selected_step = which_step;
    this->updateParameterKnobs();
  }

  void updateParameterKnobs()
  {
    // Set all of the parameter knobs of the selected step to the correct position
    for(unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETERS; parameter_number++)
    {
      this->setParameterKnobPosition(parameter_number, selected_track->getParameter(selected_step, parameter_number));
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
        json_object_set(step_data, "trigger", json_integer(this->tracks[track_number].getValue(step_index)));
        // json_object_set(step_data, "engine", json_integer(this->tracks[track_number].getValue(step_index)));

        // I could have saved the parameters as named kay/value pairs, but there's going to be a LOT of them,
        // so I'm being conscientious and storing them in a minimal-format
        json_t *parameter_json_array = json_array();
        for(int parameter_index = 0; parameter_index < 4; parameter_index++)
        {
          json_array_append_new(parameter_json_array, json_real(this->tracks[track_number].getParameter(step_index,parameter_index)));
        }

        json_object_set(step_data, "p", parameter_json_array);

        json_array_append_new(steps_json_array, step_data);
      }

      json_t *track_data = json_object();

      json_object_set(track_data, "steps", steps_json_array);
      json_object_set(track_data, "engine", json_integer(this->tracks[track_number].getEngine()));
      json_array_append_new(tracks_json_array, track_data);
    }
    json_object_set(json_root, "tracks", tracks_json_array);
    json_decref(tracks_json_array);

    // Save path of the sample bank
    json_object_set_new(json_root, "path", json_string(this->sample_bank.path.c_str()));

		return json_root;
	}

	// Load module data
	void dataFromJson(json_t *json_root) override
	{
    // Load files in sample_bank
    json_t *loaded_path_json = json_object_get(json_root, ("path"));
		if (loaded_path_json)
		{
			sample_bank.loadSamplesFromPath(json_string_value(loaded_path_json));
		}

    //
    // Load all track data
    //

    json_t *tracks_arrays_data = json_object_get(json_root, "tracks");

    if(tracks_arrays_data)
    {
      size_t track_index;
      size_t step_index;
      size_t parameter_index;
      // json_t *json_steps_array;
      json_t *json_step_object;
      json_t *json_track_object;

      json_array_foreach(tracks_arrays_data, track_index, json_track_object)
      {
        json_t *steps_json_array = json_object_get(json_track_object, "steps");
        this->tracks[track_index].setEngine(json_integer_value(json_object_get(json_track_object, "engine")));

        if(steps_json_array)
        {
          json_array_foreach(steps_json_array, step_index, json_step_object)
          {
            // First, read the trigger state
            this->tracks[track_index].setValue(step_index, json_integer_value(json_object_get(json_step_object, "trigger")));

            // Secondly, read the parameters
            json_t *parameter_json_array = json_object_get(json_step_object, "p");
            json_t *parameter_array_data;

            json_array_foreach(parameter_json_array, parameter_index, parameter_array_data)
            {
              float parameter_value = json_real_value(parameter_array_data);
              // DEBUG(std::to_string(parameter_value).c_str());
              this->tracks[track_index].setParameter(step_index, parameter_index, parameter_value);
            }
          }
        }
      }
    }
	}

  //
  // This should be the only way to set the selected parameter
  //
  void selectParameter(unsigned int parameter_number)
  {
    selected_parameter = parameter_number;
  }

  void setLCDPage(unsigned int new_page_index)
  {
    this->lcd_page_index = new_page_index;
  }

  // This function may not be needed.  It might be replacable with selectLCDFunctionSelectedParam()
  void selectLCDFunctionOnParameterFocus(unsigned int parameter_number)
  {
    // set the LCD focus based on the selected engine
    unsigned int lcd_page_index = this->selected_track->engine->getLCDController(parameter_number);
    this->setLCDPage(lcd_page_index);
  }

  void selectLCDFunctionSelectedParam()
  {
    unsigned int lcd_page_index = this->selected_track->engine->getLCDController(this->selected_parameter);
    this->setLCDPage(lcd_page_index);
  }

	void process(const ProcessArgs &args) override
	{
    bool track_switched = false;
    bool engine_switched = false;

    // selected_track = &tracks[(int) params[TRACK_SELECT_KNOB].getValue()];
    track_index = params[TRACK_SELECT_KNOB].getValue();
    if(track_index != old_track_index)
    {
      old_track_index = track_index;
      switchTrack(track_index);
      track_switched = true;
    }

    // Read the engine selection knob
    // ADD: When engine changes, set default values for engine
    // I may have to add a flag to each engine to say if it's been modified or
    // not and do not load defaults if it has been modified
    if(! track_switched)
    {
      unsigned int new_engine_index = params[ENGINE_SELECT_KNOB].getValue();
      if(new_engine_index != old_engine_index)
      {
        old_engine_index = new_engine_index;
        switchEngine(new_engine_index);
        engine_switched = true;
      }
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
        // selectStep(i);
      }

      // Process step select triggers.
      if(step_select_triggers[step_number].process(params[STEP_SELECT_BUTTONS + step_number].getValue()))
      {
        toggleStep(step_number);
      }

      // Light up drum pads
      lights[DRUM_PAD_LIGHTS + step_number].setBrightness(selected_track->getValue(step_number));

      // Light up step selection light
      lights[STEP_SELECT_BUTTON_LIGHTS + step_number].setBrightness(selected_step == step_number);

      // Show location
      lights[STEP_LOCATION_LIGHTS + step_number].setBrightness(playback_step == step_number);
    }


    //
    // Read all of the parameter knobs and assign them to the selected Track/Step

    // Be careful.  this->selected step can change within the rendering thread
    // and cause issues within this loop, therefore it's necessary to capture
    // the value in a local variable and is guaranteed not to change.
    //
    // Anothe option is to set a flag on mouse-over in the LCD code and don't
    // process this code if it's being modulated.  This might be necessary.


    unsigned int local_selected_step = this->selected_step;

    for(unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETERS; parameter_number++)
    {
      // get the knob value
      float new_parameter_value = params[ENGINE_PARAMS + parameter_number].getValue();

      // get the value for the parameter at the selected step,
      float current_parameter_value = selected_track->getParameter(local_selected_step, parameter_number);

      if(new_parameter_value != current_parameter_value)
      {
        selected_track->setParameter(local_selected_step, parameter_number, new_parameter_value);

        // BUG: THis is being called when a paramter (knob) has changed.  However,
        // swiping the parameter display should only change a single parameter's
        // value, so new_parameter_value == current_parameter_value should be true for all parameters except that being modified (see above)
        if(! track_switched && ! engine_switched) selectParameter(parameter_number); // This is triggering on parameter page swipe
        selectLCDFunctionOnParameterFocus(parameter_number);
      }
    }



    //
    // compute output
    // The return result from tne engine should be -5v to 5v
    left_output = 0;
    right_output = 0;

    for(unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      std::tie(track_left_output, track_right_output) = tracks[i].process();
      left_output += track_left_output;
      right_output += track_right_output;
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

    // step all sample players
    sample_bank.step(args.sampleRate);
  }
};
