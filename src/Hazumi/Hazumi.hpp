//

struct Hazumi : Module
{
  HazumiSequencer hazumi_sequencer;

  dsp::SchmittTrigger stepTrigger;
  dsp::PulseGenerator gateOutputPulseGenerators[8];
  bool trigger_results[8];
  unsigned int gate_outputs[8];

  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    GATE_OUTPUT_1,
    GATE_OUTPUT_2,
    GATE_OUTPUT_3,
    GATE_OUTPUT_4,
    GATE_OUTPUT_5,
    GATE_OUTPUT_6,
    GATE_OUTPUT_7,
    GATE_OUTPUT_8,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Hazumi()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    gate_outputs[0] = GATE_OUTPUT_1;
    gate_outputs[1] = GATE_OUTPUT_2;
    gate_outputs[2] = GATE_OUTPUT_3;
    gate_outputs[3] = GATE_OUTPUT_4;
    gate_outputs[4] = GATE_OUTPUT_5;
    gate_outputs[5] = GATE_OUTPUT_6;
    gate_outputs[6] = GATE_OUTPUT_7;
    gate_outputs[7] = GATE_OUTPUT_8;
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
    bool trigger_output_pulse = false;

    // Process Step Input
    if(stepTrigger.process(rescale(inputs[STEP_INPUT].getVoltage(), 0.0f, 10.0f, 0.f, 1.f)))
    {
      hazumi_sequencer.step(trigger_results);

      for(unsigned int i=0; i < 8; i++)
      {
        if(trigger_results[i]) gateOutputPulseGenerators[i].trigger(0.01f);
      }
    }

    // Output gates
    for(unsigned int i=0; i < 8; i++)
    {
      trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }

  }
};
