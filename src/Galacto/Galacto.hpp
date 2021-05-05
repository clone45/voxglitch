//
// NEXT:
// freeze buffer input

struct Galacto : Module
{
  uint8_t w = 0;     // w is the output of the equations
  int offset = 0;
  uint8_t previous = 0;
  int t = 0;
  unsigned int selected_effect = 0;
  float param_1_input = 0.0;
  float param_2_input = 0.0;

  GalactoAudioBuffer audio_buffer;

  dsp::SchmittTrigger purge_button_schmitt_trigger;

  float output = 0;  // output is the audio output

  uint32_t buffer_size = 0;
  float feedback = 0.0;

  enum ParamIds {
    CLOCK_DIVISION_KNOB,
    BUFFER_SIZE_KNOB,
    FEEDBACK_KNOB,
    EFFECT_KNOB,
    PARAM_1_KNOB,
    PARAM_2_KNOB,
    PURGE_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
    AUDIO_INPUT_LEFT,
    AUDIO_INPUT,
    EFFECT_INPUT,
    BUFFER_SIZE_INPUT,
    FEEDBACK_INPUT,
    PARAM_1_INPUT,
    PARAM_2_INPUT,
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

  // Galacto Contructor
	Galacto()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(PARAM_1_KNOB, 0.0f, 1.0f, 0.0f, "ParamKnob1");
    configParam(PARAM_2_KNOB, 0.0f, 1.0f, 0.0f, "ParamKnob2");

    configParam(BUFFER_SIZE_KNOB, 0.0f, 1.0f, 0.0f, "BufferSizeKnob");
    configParam(FEEDBACK_KNOB, 0.0f, 1.0f, 0.0f, "FeedbackKnob");
    configParam(EFFECT_KNOB, 0, 7, 0, "EffectKnob");

    audio_buffer.purge();
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


  float calculate_attenuverter_input(int input_index, int knob_index)
  {
    float input_value = inputs[input_index].getVoltage() / 10.0; // -1 to 1, normally
    float knob_value = params[knob_index].getValue(); // 0 to 1

    return(knob_value + input_value);
  }

	void process(const ProcessArgs &args) override
	{
    bool purge_button_is_triggered = purge_button_schmitt_trigger.process(params[PURGE_BUTTON].getValue());
    if(purge_button_is_triggered) audio_buffer.purge();

    //
    // Read knobs and inputs
    //
    param_1_input = calculate_attenuverter_input(PARAM_1_INPUT, PARAM_1_KNOB); // ranges from 0 to 1
    param_2_input = calculate_attenuverter_input(PARAM_2_INPUT, PARAM_2_KNOB); // ranges from 0 to 1
    buffer_size = clamp((int) (calculate_attenuverter_input(BUFFER_SIZE_INPUT, BUFFER_SIZE_KNOB) * (float) MAX_BUFFER_SIZE), MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
    feedback = clamp(calculate_attenuverter_input(FEEDBACK_INPUT, FEEDBACK_KNOB), 0.0, 1.0);

    // selected_effect = clamp((int) (( /* params[EFFECT_INPUT].getValue() * (float)NUMBER_OF_EFFECTS) +*/ params[EFFECT_KNOB].getValue()), 0, NUMBER_OF_EFFECTS);
    selected_effect = params[EFFECT_KNOB].getValue();


    // Set buffer attributes
    audio_buffer.setBufferSize(buffer_size);
    audio_buffer.setFeedback(feedback);

    float audio_input = inputs[AUDIO_INPUT].getVoltage();
    float output = 0.0;

    audio_buffer.push(audio_input);

    t += 1;

    switch(selected_effect) {
      case 0:
        output = fx_two_direction.process(this, t, param_1_input, param_2_input);
        // output = eye_1(param_1_input, param_2_input);
        break;
      case 1:
        output = fx_delays.process(this, t, param_1_input, param_2_input);
        break;
      case 2:
        output = fx_bytebeat_1.process(this,t, param_1_input, param_2_input);
        break;
        /*
      case 3:
        output = eye_4(param_1_input, param_2_input);
        break;
      case 4:
        output = eye_5(param_1_input, param_2_input);
        break;
      case 5:
        output = eye_6(param_1_input, param_2_input);
        break;
      case 6:
        output = eye_7(param_1_input, param_2_input);
        break;
        */
      case 7:
        output = fx_slice_repeat.process(this,t, param_1_input, param_2_input);
        break;

    }

    outputs[AUDIO_OUTPUT].setVoltage(output);
  }

  struct Effect
  {
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

  struct FXTwoDirection : Effect
  {
    int offset = 0;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 8.0;
      if(vp1 == 0) vp1 = 1;

      float output = galacto->audio_buffer.getOutput((galacto->buffer_size  / 2) - t) + galacto->audio_buffer.getOutput(t);

      return(output);
    }
  } fx_two_direction;

  struct FXDelays : Effect
  {
    float process(Galacto *galacto, int t, float p1, float p2)
    {
      float output = galacto->audio_buffer.getOutput(t) + galacto->audio_buffer.getOutput(t + (galacto->buffer_size / 2)) + galacto->audio_buffer.getOutput(t + (galacto->buffer_size / 4));
      return(output);
    }
  } fx_delays;

  struct FXBytebeat1 : Effect
  {
    int offset = 0;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      uint32_t vp2 = p2 * 16.0;

      offset = ((t*7)&div(t,vp2)) * .1;

      float output = galacto->audio_buffer.getOutput(t + offset);
      return(output);
    }
  } fx_bytebeat_1;

  // Very similar to the one above, but with different ranges?
  struct FXBytebeat2 : Effect
  {
    int offset = 0;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = ((t*vp1)&div(t,vp2)) * .1;
      return(galacto->audio_buffer.getOutput(t + offset));
    }
  } fx_bytebeat_2;


  struct FXBytebeat3 : Effect
  {
    int offset = 0;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = ((t >> vp1) & t) * (t>>vp2);
      return(galacto->audio_buffer.getOutput(t + offset));
    }
  } fx_bytebeat_3;

  struct FXBytebeat4 : Effect
  {
    int offset = 0;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = mod((( mod(t,((76 - (t>>vp2)) % 11))) * (t>>1)), (47-(t>>(4+vp1)) % 41)) * .7;
      return(galacto->audio_buffer.getOutput(t + offset));
    }
  } fx_bytebeat_4;

  struct FXBytebeat5 : Effect
  {
    int offset = 0;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = (vp1&t*(4|(7&t>>13))>>(1&-t>>vp2))+(127&t*(t>>11&t>>13)*(3&-t>>9));
      return(galacto->audio_buffer.getOutput(t + offset));
    }
  } fx_bytebeat_5;

  struct FXDizzy : Effect
  {
    int offset = 0;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 5000;
      uint32_t vp2 = p2 * 6000;

      offset = sin(float(t/8.0) / vp1) * galacto->buffer_size;
      offset += sin(float(t/32.0) / vp2) * galacto->buffer_size;

      return(galacto->audio_buffer.getOutput(t + offset));
    }
  } fx_dizzy;

  struct FXSliceRepeat : Effect
  {
    int divisor = 4;
    int window_size;
    int offset = -1;

    float process(Galacto *galacto, int t, float p1, float p2)
    {
      divisor = int(p1 * 32.0);
      if(divisor <= 1) divisor = 2;

      window_size = galacto->buffer_size / divisor;

      if(++offset >= 0) offset = (-1 * window_size);

      return(galacto->audio_buffer.getOutput(offset) + galacto->audio_buffer.getOutput(t));
    }
  } fx_slice_repeat;

  /*



  struct eye_9
  {
    int offset = 0;

    float process(Galacto *g, float p1, float p2)
    {
      uint32_t vp1 = p1 * 5000;
      uint32_t vp2 = p2 * 6000;

      offset = sin(float(g->t/8.0) / vp1) * g->buffer_size;
      offset += sin(float(g->t/32.0) / vp2) * g->buffer_size;


      float output = g->audio_buffer.getOutput(offset);

      // return(output);
      return(output);
    }
  };
  */

  /*

  eye_9

  play from a sample position to sample position + 22040 or something.
  then step forward that amount
  then repeat
  poor man's granular synthesis, or short repeater


  */


  //
  // These are safe versions of / and %  that avoid division by 0 which crash VCV Rack
  //

};
