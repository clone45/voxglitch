//
// Hello!
//
// Welcome to the main ByteBeat code.  I'm glad you're here!  Let me give you
// a quick overview of where things are:
//
// This document contains
// - All the parameter, input, and output declarations.  If you want to add a
//   new knob, input, or output, you'll need to make updates in this document
//   as well as in ByteBeatWidget.hpp
// - The main "process" loop, whic reads all the inputs and outputs audio
// - A "compute" function which contains all of the bytebeat equations
// - A number of "expression" functions which provide more variance for the
//   equations.
// - A few helper functions
//

// TODO: slave to clock and provide external clock
// TODO: Allow override of "t" using external input
// TODO: change knobs so that they snap into position instead of being continuous.
// TODO: Add reset input

struct ByteBeat : Module
{
  uint8_t w = 0;     // w is the output of the equations
  float output = 0;  // output is the audio output

  uint32_t t;  // t is the time counter used in the equations

  // p1-p3 are variables used in equations.  These are values that the user
  // can manipulate to alter the sounds coming from the equations.
  uint32_t p1;
  uint32_t p2;
  uint32_t p3;

  // e1-e3 are sub-expressions to give equations more variance
  uint32_t e1;
  uint32_t e2;
  uint32_t e3;

  // I want to add better pitch control before this module is released.  But
  // in the short term, I'm simply skipping frames in the compute() function
  // to feign sample rate.  This won't work for the final version because
  // different users may have different sample rate settings.

  uint8_t clock_division_counter = 0;
  uint8_t clock_division = 2;

  enum ParamIds {
    CLOCK_DIVISION_KNOB,
    EQUATION_KNOB,
    PARAM_KNOB_1,
    PARAM_KNOB_2,
    PARAM_KNOB_3,
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
    DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

  // Bytebeat Contructor
  //
  // I usually narmalize all of my parameters to range from 0 to 1, then later
  // map them to the correct values in my "calculate_inputs" helper functions

	ByteBeat()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(EQUATION_KNOB, 0.0f, NUMBER_OF_EQUATIONS - 1, 0.0f, "EquationKnob");

    configParam(PARAM_KNOB_1, 0.0f, 1.0f, 0.0f, "ParamKnob1");
    configParam(PARAM_KNOB_2, 0.0f, 1.0f, 0.0f, "ParamKnob2");
    configParam(PARAM_KNOB_3, 0.0f, 1.0f, 0.0f, "ParamKnob3");

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
    // There's nothing to save and load yet.
	}

  // This is a helper function for reading inputs with attenuators
  //
  // input_index: The index of the input in InputIds.
  // knob_index: The index of the parameter in ParamIds
  // maxiumum value: The output of calculation_inputs will range from 0 to maximum_value
  //
  // Example call:
  //
  // float value = calculate_inputs(MY_INPUT, MY_ATTENUATOR_KNOB, 1200.0);
  //

  float calculate_inputs(int input_index, int knob_index, float maximum_value)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0;
    float knob_value = params[knob_index].getValue();

    if(inputs[input_index].isConnected())
    {
      input_value = clamp(input_value, 0.0, 1.0);
      output = clamp((input_value * maximum_value) + (knob_value * maximum_value), 0.0, maximum_value);
    }
    else
    {
      output = clamp(knob_value * maximum_value, 0.0, maximum_value);
    }
    return(output);
  }


	void process(const ProcessArgs &args) override
	{
    // Use clock divider to control how quickly t is incremented.
    // As I mentioned above, this is a temporary hack and will be replaced with
    // more intelligent control over sample rate (pitch)

    clock_division_counter ++;
    if(clock_division_counter >= clock_division)
    {
      t = t + 1;
      clock_division_counter = 0;
    }

    //
    // Read equation, parameter, and expression inputs.
    // Lots of implicit float to int conversion happening here

    uint32_t equation = params[EQUATION_KNOB].getValue();

    p1 = calculate_inputs(PARAM_INPUT_1, PARAM_KNOB_1, 4096.0);
    p2 = calculate_inputs(PARAM_INPUT_2, PARAM_KNOB_2, 4096.0);
    p3 = calculate_inputs(PARAM_INPUT_3, PARAM_KNOB_3, 4096.0);

    // Send the equation, parameters, and expression selections to the "compute"
    // function.  The output of the "compute" function will be a float representing
    // the audio generated by one of the bytebeat equations.
    outputs[AUDIO_OUTPUT].setVoltage(compute(equation, t, p1, p2, p3));

    // outputs[DEBUG_OUTPUT].setVoltage(e1); // << for difficult debugging
    clock_division = params[CLOCK_DIVISION_KNOB].getValue(); // float to int conversion happening here
  }

  //
  // compute(...)
  //
  // This equation takes in an equation number and a bunch of parameters and outputs
  // audio in the form of a floating point number.
  //
  // * The equation number is used  in a switch statement for deciding which equation to evaluate.
  // * Parameters p1, p2, and p3 are used as variables in the equation.  P1, p3, and p3 have
  //   both a knob and CV input on the front panel to allow the user to jazz up the equation.
  //
  // I'm using a rating system while this module is in development.  Equations
  // are rated from 1 to 10 based on how much I like them.  This'll help me later
  // to decide which equations should be included in the final release.
  //
  // If you want to add a new equation, you'll need to:
  //  1. Add the equation in the switch statement in the compute function below
  //  2. Increment the constant NUMBER_OF_EQUATIONS in defines.h
  //
  // Why is "w" defined as a class variable instead of inside of this function?
  // It's because you may want to use the previous value for "w" when calcuating the next "w".  :-)
  //

  float compute(uint32_t equation_number, uint32_t t, uint32_t p1, uint32_t p2, uint32_t p3)
  {
    switch(equation_number) {

      case 0: // Exploratorium
        w = ((mod(t,(p1+(mod(t,p2)))))^(t>>(p3>>5)))*2;
        break;

      case 1: // Toner
        w = ((t>>( mod((t>>12), (p3>>4)) ))+( mod((p1|t),p2)))<<2;
        break;

      case 2: // widerange
        w = (((p1^(t>>(p2>>3)))-(t>>(p3>>2))-mod(t,(t&p2))));
        break;

      case 3: // Landing gear
        w = ((p1&t^mod((t>>2), p2))&w+1393+p3);
        break;

      case 4: // rampcode (https://github.com/gabochi/rampcode/blob/master/tutorial)
        w = div((t*((t>>10&p1)+1)),((-t>>12&p2)+1))<<(t*p3>>(t>>14&3)&7|3);
        break;

      case 5:
        w = t << (t>> (0xb1a7529>>(t>>p1&7)*4&15) &7) & t>>(p3>>(t>>p2&3)*4&15);
        break;

      case 6: // Silent treatment
        w = (t-t+t*p1)|(t&(p3+1))|div(t,p2);
        break;

      case 7: // BitWiz Transplant
        w = (t-((t&p1)*p2-1668899)*(mod((t>>15),15)*t))>>( mod(t>>12,16))>>(p3%15);
        break;

      // Add next equation here.  Don't forget to increment NUMBER_OF_EQUATIONS in defines.h
      case 8: //
        // w =
        break;
    }

    // w is a 8-bit unsigned integer that ranges from 0 to 256. But
    // this funtion is supposed to return a float between 0 and 1, so we divide
    // w by 256.0.
    return(w / 256.0);
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
