//
// Stringed

struct Stringed : Module
{
  dsp::PulseGenerator gateOutputPulseGenerators[3];
  unsigned int gate_outputs[3];
  bool triggered_string[3] = {false, false, false};

  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
    GATE_OUTPUT_1,
    GATE_OUTPUT_2,
    GATE_OUTPUT_3,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Stringed()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    gate_outputs[0] = GATE_OUTPUT_1;
    gate_outputs[1] = GATE_OUTPUT_2;
    gate_outputs[2] = GATE_OUTPUT_3;
	}

  //
	// Autosave module data.  VCV Rack decides when this should be called.
  //
  // My saving and loading code could be more compact by saving arrays of
  // "ball" data tidily packaged up.  I'll do that if this code ever goes
  // through another iteration.
  //

	json_t *dataToJson() override
	{
    json_t *json_root = json_object();
  	return json_root;
	}

	// Load module data
	void dataFromJson(json_t *json_root) override
	{
	}

	void process(const ProcessArgs &args) override
	{

    for(unsigned int i=0; i < 3; i++)
    {
      if(triggered_string[i] == true)
      {
        gateOutputPulseGenerators[i].trigger(0.01f);
        triggered_string[i] = false;
      }

      bool trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }

    // Output gates
    /*
    for(unsigned int i=0; i < 8; i++)
    {
      bool trigger_output_pulse = gateOutputPulseGenerators[i].process(1.0 / args.sampleRate);
      outputs[gate_outputs[i]].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }
    */
  }
};
