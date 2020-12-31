struct ByteBeat : Module
{
  float left_audio = 0;
  float right_audio = 0;

  uint32_t t;  // t is our time counter used in the equations
  uint32_t p1; // p1-p3 are variables used (optionally) in equations
  uint32_t p2;
  uint32_t p3;
  uint8_t w;   // w is the output of the equation
  uint8_t clock_division_counter = 0;
  uint8_t clock_division = 2;

  std::string math_equation;

  enum ParamIds {
    CLOCK_DIVISION_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
    PARAM_INPUT_1,
    PARAM_INPUT_2,
    PARAM_INPUT_3,
		NUM_INPUTS
	};
	enum OutputIds {
    AUDIO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ByteBeat()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(CLOCK_DIVISION_KNOB, 0.0f, 16.0f, 0.0f, "ClockDivisionKnob");
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
    clock_division_counter ++;
    if(clock_division_counter >= clock_division)
    {
      t = t + 1;
      clock_division_counter = 0;
    }

    p1 = (inputs[PARAM_INPUT_1].getVoltage() / 10.0) * 4096.0;
    p2 = (inputs[PARAM_INPUT_2].getVoltage() / 10.0) * 4096.0;
    p3 = (inputs[PARAM_INPUT_3].getVoltage() / 10.0) * 4096.0;

    clock_division = params[CLOCK_DIVISION_KNOB].getValue(); // float to int conversion happening here

    // float output = compute(t, p1, p2, p3);

    // Test out calcuator
    int result = calculator::eval("(0 + ~(255 & 1000)*3) / -2");

    // int result = 1;

    outputs[AUDIO_OUTPUT].setVoltage(result);
  }

  float compute(uint32_t t, uint32_t p1, uint32_t p2, uint32_t p3)
  {
    // >> has precidence over mod

    // w = (uint8_t) ((t>>(13 + p1)&t)*(t>>(p2)));
    // w = div(((t^(p1>>3)-456)*(p2+1)),(((t>>(p3>>3))%14)+1))+(t*((182>>(t>>15)%16))&1);

    // Alpha
    // w = div(((t^(p1>>3)-456)*(p2+1)),(((t>>(p3>>3))%14)+1))+(t*((182>>(t>>15)%16))&1);

    // Omega
    // w= (((t>>5)|(t<<((p3>>4)+1))>>4)*p1-(div(t,((1853>>(t>>(p2>>4))%15)))&1)>>(t>>12)%6)>>4;

    // Widerange
    // w = (((p1^(t>>(p2>>3)))-(t>>(p3>>2))-mod(t,(t&p2))));

    // Defender
    // w = ( mod((w-482+(w^(3456-p1))), (p2*t>>5&(1030-(p3<<2)))) );

    // Maginfied
    // w = (mod((p1 & t), p3)) ^ div(t,p2);

    // Light Reactor
    w = mod((p1+t>>p2),12)|((  mod(w,(p1+t>>p1%4)))+11+p3^t)>>(p3>>12);

    return(w / 256.0);
  }

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
