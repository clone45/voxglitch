//
// TODO: add outputs!
// call process in engine
// Listen to some sounds
// Add per-step properties

struct Scalar110 : Module
{
  dsp::SchmittTrigger drum_button_triggers[NUMBER_OF_STEPS];
  dsp::SchmittTrigger stepTrigger;
  Track tracks[NUMBER_OF_TRACKS];
  Track *selected_track;
  float parameters[6];
  unsigned int playback_step = 0;
  unsigned int selected_step = 0;
  StepParams step_parameters;
  float left_output;
  float right_output;

  enum ParamIds {
    ENUMS(DRUM_BUTTONS, NUMBER_OF_STEPS),
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
    ENUMS(DRUM_BUTTON_LIGHTS, NUMBER_OF_STEPS),
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
      configParam(ENGINE_PARAMS + i, 0.0, 1.0, 1.0, "Parameter_" + std::to_string(i));
    }

    tracks[0].engine = new Foo();
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
    // Process drum step selection
    for(unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
    {
      if(drum_button_triggers[i].process(params[DRUM_BUTTONS + i].getValue()))
      {
        selected_track->toggleStep(i);
      }

      lights[DRUM_BUTTON_LIGHTS + i].setBrightness(selected_track->getValue(i));
      lights[STEP_LOCATION_LIGHTS + i].setBrightness(playback_step == i);
    }

    //
    // Read all of the parameter knobs and assign them to the select
    // Track/Step
    // TODO: figure out how to select a step
    for(unsigned int i = 0; i < NUMBER_OF_PARAMETERS; i++)
    {
      step_parameters.p[i] = params[ENGINE_PARAMS + i].getValue();
    }
    selected_track->setParameters(selected_step, &step_parameters);

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
    }
  }
};
