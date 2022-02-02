struct ByteBender : Module
{
  uint8_t w = 0;     // w is the output of the equations
  float output = 0;  // output is the audio output
  uint32_t t;  // t is the time counter used in the equations

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
    process_m1();
    process_m2();
  }

  uint32_t readInput(unsigned int input_index)
  {
    uint32_t low_value = 0;
    uint32_t high_value = 0;
    uint32_t value = 0;

    low_value = (uint32_t) inputs[input_index].getVoltage(0);

    if(inputs[input_index].getChannels() > 1)
    {
      high_value = (uint32_t) inputs[input_index].getVoltage(1);
      value = low_value | (high_value << 16);
    }
    else
    {
      value = low_value;
    }

    return value;
  }

  void process_m1()
  {
    uint32_t a = readInput(INPUT_1A);
    uint32_t b = readInput(INPUT_1B);
    w = a >> b;
    if (w>0) outputs[OUTPUT_1].setVoltage(w);
  }

  void process_m2()
  {
    uint32_t a = readInput(INPUT_2A);
    uint32_t b = readInput(INPUT_2B);
    w = mod(a,b);
    outputs[OUTPUT_2].setVoltage(w);
  }

  void outputClocks(uint32_t t)
  {
    uint16_t low = t & 65535;// isolate the first 16 bits
    uint16_t high = t >> 16;

    outputs[T1_OUTPUT].setVoltage(low, 0);
    outputs[T1_OUTPUT].setVoltage(high, 1);
    outputs[T1_OUTPUT].setChannels(2);
  }

  //
  // These are safe versions of / and %  that avoid division by 0 which crash VCV Rack
  //

  uint32_t div(uint32_t a, uint32_t b)
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
