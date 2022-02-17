//
// TODO: add outputs!
// call process in engine
// Listen to some sounds
// Add per-step properties

struct Scalar110 : Module
{
  dsp::SchmittTrigger drum_pad_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger step_select_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger stepTrigger;
  Track tracks[NUMBER_OF_TRACKS];
  Track *selected_track;
  // float parameters[6];
  unsigned int playback_step = 0;
  unsigned int selected_step = 0;
  StepParams step_parameters;
  float left_output;
  float right_output;

  enum ParamIds {
    ENUMS(DRUM_PADS, NUMBER_OF_STEPS),
    ENUMS(STEP_SELECT_BUTTONS, NUMBER_OF_STEPS),
    ENUMS(ENGINE_PARAMS, NUMBER_OF_PARAMETERS),
    TRACK_SELECT_KNOB,
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

    for(unsigned int i=0; i<NUMBER_OF_PARAMETERS; i++)
    {
      // TODO: It's going to be important to figure out the defaults for the
      // parameters per step and load them instead of setting them all to 0
      configParam(DRUM_PADS + i, 0.0, 1.0, 0.0, "drum_button_" + std::to_string(i));
      configParam(STEP_SELECT_BUTTONS + i, 0.0, 1.0, 0.0, "selected_step_" + std::to_string(i));
      configParam(ENGINE_PARAMS + i, 0.0, 1.0, 1.0, "Parameter_" + std::to_string(i));
    }

    tracks[0].engine = new Foo();
    selected_track = &tracks[0];
	}

  /*
  void setParameters(unsigned int selected_step, Track *selected_track)
  {
    // TODO: INstead of copying the parameters into the local structure, how
    // about just having a pointer to track_parameters that always points
    // to one of the track's parameter structures, or something like that?
    StepParams *track_parameters = selected_track->getParameters(selected_step);

    for(unsigned int i=0; i<NUMBER_OF_PARAMETERS; i++)
    {
      parameters[i] = track_parameters->p[i];
    }
  }
  */

	// Autosave module data.  VCV Rack decides when this should be called.
	json_t *dataToJson() override
	{
		json_t *root = json_object();
		return root;
	}

	// Load module data
	void dataFromJson(json_t *root) override
	{
	}

	void process(const ProcessArgs &args) override
	{
    //
    // Handle drum pads, drum location, and drum selection interactions and
    // lights.
    //

    for(unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
    {
      // Process drum pads
      if(drum_pad_triggers[i].process(params[DRUM_PADS + i].getValue())) selected_track->toggleStep(i);

      // Process step select triggers.
      if(step_select_triggers[i].process(params[STEP_SELECT_BUTTONS + i].getValue()))
      {
        selected_step = i;
        for(unsigned int parameter_number = 0; parameter_number < NUMBER_OF_PARAMETERS; parameter_number++)
        {
          // set the knob positions for the selected step
          params[ENGINE_PARAMS + parameter_number].setValue(selected_track->step_parameters[selected_step].p[parameter_number]);
        }
      }

      // Light up drum pads
      lights[DRUM_PAD_LIGHTS + i].setBrightness(selected_track->getValue(i));

      // Light up step selection light
      lights[STEP_SELECT_BUTTON_LIGHTS + i].setBrightness(selected_step == i);

      // Show location
      lights[STEP_LOCATION_LIGHTS + i].setBrightness(playback_step == i);
    }


    // Set all of the parameter knobs to the values for the current step
    // TODO: Maybe just do this if the selected track has changed
    // setParameters(selected_step, selected_track);

    //   selected_track->getParameters(selected_step, &step_parameters);
    //   params[ENGINE_PARAMS + i].setValue(step_parameters.p[i]);

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
    //
    std::tie(left_output, right_output) = selected_track->process();

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
