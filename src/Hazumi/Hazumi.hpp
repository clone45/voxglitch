//

struct Hazumi : Module
{
  HazumiSequencer hazumi_sequencer;

  dsp::SchmittTrigger stepTrigger;

  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Hazumi()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // configParams go here
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
    // There's nothing to save and load yet.
	}

	void process(const ProcessArgs &args) override
	{
    // Process Step Input
    if(stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      hazumi_sequencer.step();

      /*
      for(unsigned int i=0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
      {
        if(trigger_results[i]) gateOutputPulseGenerators[i].trigger(0.01f);
      }
      */
    }
  }
};
