struct ByteBeat : Module
{
  float left_audio = 0;
  float right_audio = 0;

  uint32_t t;  // t is our time counter used in the equations
  uint32_t p1; // p1-p3 are variables used (optionally) in equations
  uint32_t p2;
  uint32_t p3;
  uint32_t e1; // e1-e3 are sub-expressions to give equations more variance
  uint32_t e2;
  uint32_t e3;

  uint8_t w = 0;   // w is the output of the equation

  uint8_t clock_division_counter = 0;
  uint8_t clock_division = 2;

  enum ParamIds {
    CLOCK_DIVISION_KNOB,
    EQUATION_KNOB,
    EXPRESSION_KNOB_1,
    EXPRESSION_KNOB_2,
    EXPRESSION_KNOB_3,
    PARAM_KNOB_1,
    PARAM_KNOB_2,
    PARAM_KNOB_3,
		NUM_PARAMS
	};
	enum InputIds {
    PARAM_INPUT_1,
    PARAM_INPUT_2,
    PARAM_INPUT_3,
    EXPRESSION_INPUT_1,
    EXPRESSION_INPUT_2,
    EXPRESSION_INPUT_3,
		NUM_INPUTS
	};
	enum OutputIds {
    AUDIO_OUTPUT,
    DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ByteBeat()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(EQUATION_KNOB, 0.0f, 1.0f, 0.0f, "EquationKnob");

    configParam(EXPRESSION_KNOB_1, 0.0f, 1.0f, 0.0f, "ExpressionKnob1");
    configParam(EXPRESSION_KNOB_2, 0.0f, 1.0f, 0.0f, "ExpressionKnob2");
    configParam(EXPRESSION_KNOB_3, 0.0f, 1.0f, 0.0f, "ExpressionKnob3");

    configParam(CLOCK_DIVISION_KNOB, 0.0f, 256.0f, 0.0f, "ClockDivisionKnob");  // 256 gives us the entire range.  Anything after that wraps
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

  float calculate_inputs(int input_index, int knob_index, float maximum_value)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0;
    float knob_value = params[knob_index].getValue();
    float output;

    if(inputs[input_index].isConnected())
    {
      input_value = clamp(input_value, 0.0, 1.0);

      // NUMBER_OF_EXPRESSIONS is defined in defines.h
      output = clamp((input_value * maximum_value) + (knob_value * maximum_value), 0.0, maximum_value);
    }
    else
    {
      // NUMBER_OF_EXPRESSIONS is defined in defines.h
      output = clamp(knob_value * maximum_value, 0.0, maximum_value);
    }
    return(output);
  }


	void process(const ProcessArgs &args) override
	{
    //
    // TODO: slave to clock and provide external clock
    // TODO: Allow override of "t" using external input
    //

    clock_division_counter ++;
    if(clock_division_counter >= clock_division)
    {
      t = t + 1;
      clock_division_counter = 0;
    }

    // FYI: float to uint32_t conversions are happening when assigning the input values to the p,e varaibles

    uint32_t equation = params[EQUATION_KNOB].getValue() * NUMBER_OF_EQUATIONS;

    p1 = calculate_inputs(PARAM_INPUT_1, PARAM_KNOB_1, 4096.0);
    p2 = calculate_inputs(PARAM_INPUT_2, PARAM_KNOB_2, 4096.0);
    p3 = calculate_inputs(PARAM_INPUT_3, PARAM_KNOB_3, 4096.0);

    e1 = calculate_inputs(EXPRESSION_INPUT_1, EXPRESSION_KNOB_1, (float) NUMBER_OF_EXPRESSIONS); // float to uint32_t happening here
    e2 = calculate_inputs(EXPRESSION_INPUT_2, EXPRESSION_KNOB_2, (float) NUMBER_OF_EXPRESSIONS);
    e3 = calculate_inputs(EXPRESSION_INPUT_3, EXPRESSION_KNOB_3, (float) NUMBER_OF_EXPRESSIONS);

    outputs[AUDIO_OUTPUT].setVoltage(compute(equation, t, p1, p2, p3, e1, e2, e3));
    outputs[DEBUG_OUTPUT].setVoltage(e1);

    clock_division = params[CLOCK_DIVISION_KNOB].getValue(); // float to int conversion happening here
  }


  float compute(uint32_t equation, uint32_t t, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t e1, uint32_t e2, uint32_t e3)
  {
    // >> has precidence over mod

    // w = matrix(e2,matrix(e1,t,p1),t>>p2) - matrix(e3,p3,t+w);
    // w = matrix(e1,t,p1) + (matrix(e2,p3,(div(t,4))) * p2) - matrix(e3,p3,t);

    // w = sin((float) ( (float)t / 4294967296.1)) * 4096.0;

    switch(equation) {

      case 0: // Triangle Dash (rating: 8/10)
        w = t << (t>>  matrix(e1,(p1>>6), (3^t))  >> matrix(e2,(p2>>6), 3)) | matrix(e3,t>>(p3>>6), t>>3);
        break;

      case 1: // Digital Trash (rating: 9/10)
        w = matrix(e1,t+w,p1) + (matrix(e2,p3,(div(t,w))) * p2) - matrix(e3,p3,t);
        break;

      case 2: // Alpha (no rating yet)
        w = matrix(e1, ((t^(p1>>3)-456)*(p2+1)),(((t>>(p3>>3))%14)+1))+(t*((matrix(e1,182>>(t>>15),16)))&1);
        break;

      case 3: // Omega (no rating yet)
        w = (matrix(e3,(matrix(e1,t>>5,t<<((p3>>4)+1))>>4),p1-(matrix(e2,t,((1853>>(t>>(p2>>4))%15)))&1))>>(t>>12)%6)>>4;
        break;

      default:
        w = 0;
    }

      // Unsorted experimental equations
      //

    // w = ((p1^matrix(e1,t,(p2>>3))))-(t>>(matrix(e2,p3,2)))-matrix(e3,t,(t&p2));

    // Widerange
    // w = ((p1^matrix(e1,t,(p2>>3))))-(t>>(matrix(e2,p3,2)))-matrix(e3,t,(t&p2));

    // Defender
    // w = ( mod((w-482+(w^(3456-p1))), (p2*t>>5&(1030-(p3<<2)))) );

    // Maginfied
    // w = (mod((p1 & t), p3)) ^ div(t,p2);

    // Light Reactor
    // w = mod((p1+t>>p2),12)|((mod(w,(p1+t>>p1%4)))+11+p3^t)>>(p3>>12);

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

  uint32_t matrix(uint32_t expression_number, uint32_t a, uint32_t b)
  {
    switch(expression_number) {
      case 1: return expression_1(a,b);
      case 2: return expression_2(a,b);
      case 3: return expression_3(a,b);
      case 4: return expression_4(a,b);
      case 5: return expression_5(a,b);
      case 6: return expression_6(a,b);
      case 7: return expression_7(a,b);
      case 8: return expression_8(a,b);
      case 9: return expression_9(a,b);
      case 10: return expression_10(a,b);
      case 11: return expression_11(a,b);
      case 12: return expression_12(a,b);
      // case 13: return expression_13(a,b);
    }
    return 0;
  }

  uint32_t expression_1(uint32_t a, uint32_t b)
  {
    return(a >> b);
  }

  uint32_t expression_2(uint32_t a, uint32_t b)
  {
    return(a << b);
  }

  uint32_t expression_3(uint32_t a, uint32_t b)
  {
    return(a + b);
  }

  uint32_t expression_4(uint32_t a, uint32_t b)
  {
    return(a - b);
  }

  uint32_t expression_5(uint32_t a, uint32_t b)
  {
    return(mod(a,b));
  }

  uint32_t expression_6(uint32_t a, uint32_t b)
  {
    return(a * b);
  }

  uint32_t expression_7(uint32_t a, uint32_t b)
  {
    return(div(a,b));
  }

  uint32_t expression_8(uint32_t a, uint32_t b)
  {
    return(a^b);
  }

  uint32_t expression_9(uint32_t a, uint32_t b)
  {
    return(a & b);
  }

  uint32_t expression_10(uint32_t a, uint32_t b)
  {
    return(a | b);
  }

  uint32_t expression_11(uint32_t a, uint32_t b)
  {
    return(a + w); // test this
  }

  uint32_t expression_12(uint32_t a, uint32_t b)
  {
    return((a + b) / 2);
  }

};
