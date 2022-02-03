struct ByteBender : Module
{
  float output = 0;  // output is the audio output
  uint16_t t;  // t is the time counter used in the equations
  float w;

  float a;
  float b;

  uint8_t clock_division_counter = 0;
  uint8_t clock_division = 2;

  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    INPUT_1A,
    INPUT_1B,
    INPUT_2A,
    INPUT_2B,
    INPUT_3A,
    INPUT_3B,
		NUM_INPUTS
	};
	enum OutputIds {
    OUTPUT_1,
    OUTPUT_2,
    OUTPUT_3,
    T1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ByteBender()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    // configParam(PARAM_KNOB_1, 0.0f, 128, 0.0f, "ParamKnob1");
    // configParam(PARAM_KNOB_2, 0.0f, 128, 0.0f, "ParamKnob2");
    // configParam(PARAM_KNOB_3, 0.0f, 128, 0.0f, "ParamKnob3");

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
    clock_division_counter ++;
    if(clock_division_counter >= clock_division)
    {
      t = t + 1;
      clock_division_counter = 0;
    }

    outputClocks(t);

    // Don't forget to add this!!
    outputs[OUTPUT_1].setVoltage(process_m1()); // output is in the -5/+5v range
    outputs[OUTPUT_2].setVoltage(process_m2()); // output is in the -5/+5v range
    outputs[OUTPUT_3].setVoltage(process_m3()); // output is in the -5/+5v range
  }

  float process_m1()
  {
    a = inputs[INPUT_1A].getVoltage();
    b = inputs[INPUT_1B].getVoltage();
    return(sin(a + b));
  }

  float process_m2()
  {
    a = inputs[INPUT_2A].getVoltage();
    b = inputs[INPUT_2B].getVoltage();
    return(a*b);
  }

  float process_m3()
  {
    a = inputs[INPUT_3A].getVoltage();
    b = inputs[INPUT_3B].getVoltage();
    return(div(a,b));
  }

  void outputClocks(uint16_t t)
  {
    outputs[T1_OUTPUT].setVoltage(t);
  }

  //
  // These are safe versions of / and %  that avoid division by 0 which crash VCV Rack
  //

  float div(float a, float b)
  {
    if(b == 0) return(0);
    return(a / b);
  }

  uint32_t mod(uint32_t a, uint32_t b)
  {
    if(b == 0) return(0);
    return(a % b);
  }

};
