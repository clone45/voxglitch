//
// Next: add engine select knob to allow for seleciton of low drums
// See: https://github.com/VCVRack/Fundamental/blob/v1/src/Merge.cpp#L71
// and: https://community.vcvrack.com/t/getting-started-with-rack-menu/8608
// then test the kick drum


struct Scalar110 : Module
{
  dsp::SchmittTrigger drum_pad_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger step_select_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger stepTrigger;
  Track tracks[NUMBER_OF_TRACKS];
  Track *selected_track;
  unsigned int playback_step = 0;
  unsigned int selected_step = 0;
  unsigned int engine = 0;
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

    configParam(TRACK_SELECT_KNOB, 0.0, NUMBER_OF_TRACKS, 0.0, "Track");
    configParam(ENGINE_SELECT_KNOB, 0.0, 2.0, 0.0, "Engine");

    selected_track = &tracks[0];
	}

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

    // Read the engine selection knob

    engine = params[ENGINE_SELECT_KNOB].getValue() * (NUMBER_OF_ENGINES - 1);

    switch(engine) {
      case 0:
        selected_track->setEngine(new Foo());
        break;
      case 1: //
        selected_track->setEngine(new LowDrums());
        break;
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
