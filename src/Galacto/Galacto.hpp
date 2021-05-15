//
// NEXT:
// freeze buffer input
  /* 1024 sin array */


struct Galacto : Module
{
  uint8_t w = 0;     // w is the output of the equations
  int offset = 0;
  uint8_t previous = 0;
  unsigned int t = 0;
  unsigned int selected_effect = 0;
  float param_1_input = 0.0;
  float param_2_input = 0.0;

  GalactoStereoAudioBuffer audio_buffer;

  dsp::SchmittTrigger purge_button_schmitt_trigger;

  float left_audio = 0.0;
  float right_audio = 0.0;

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
    AUDIO_INPUT_RIGHT,
    EFFECT_INPUT,
    BUFFER_SIZE_INPUT,
    FEEDBACK_INPUT,
    PARAM_1_INPUT,
    PARAM_2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    AUDIO_OUTPUT_LEFT,
    AUDIO_OUTPUT_RIGHT,
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
    configParam(EFFECT_KNOB, 0, 9, 0, "EffectKnob");

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

    float audio_input_left = inputs[AUDIO_INPUT_LEFT].getVoltage();
    float audio_input_right = inputs[AUDIO_INPUT_RIGHT].getVoltage();
    // float output = 0.0;

    audio_buffer.push(audio_input_left, audio_input_right);

    t += 1;

    switch(selected_effect) {

      case 0:
        std::tie(left_audio, right_audio) = fx_two_direction.process(this, t, param_1_input, param_2_input);
        break;
      case 1:
        std::tie(left_audio, right_audio) = fx_delays.process(this, t, param_1_input, param_2_input);
        break;
      case 2:
        std::tie(left_audio, right_audio) = fx_bytebeat_1.process(this,t, param_1_input, param_2_input);
        break;
      case 3:
        std::tie(left_audio, right_audio) = fx_bytebeat_2.process(this,t, param_1_input, param_2_input);
        break;
      case 4:
        std::tie(left_audio, right_audio) = fx_bytebeat_3.process(this,t, param_1_input, param_2_input);
        break;
      case 5:
        std::tie(left_audio, right_audio) = fx_bytebeat_4.process(this,t, param_1_input, param_2_input);
        break;
      case 6:
        std::tie(left_audio, right_audio) = fx_bytebeat_5.process(this,t, param_1_input, param_2_input);
        break;
      case 7:
        std::tie(left_audio, right_audio) = fx_slice_repeat.process(this, t, param_1_input, param_2_input);
        break;
      case 8:
        std::tie(left_audio, right_audio) = fx_dizzy.process(this, t, param_1_input, param_2_input);
        break;
      case 9:
        std::tie(left_audio, right_audio) = fx_wave_packing.process(this, t, param_1_input, param_2_input);
        break;
    }

    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_audio);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_audio);

    // For testing, just output the raw input audio
    // outputs[AUDIO_OUTPUT_LEFT].setVoltage(audio_input_left);
    // outputs[AUDIO_OUTPUT_RIGHT].setVoltage(audio_input_right);
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

    std::pair<float,float> mix(const std::pair<float,float> &a, const std::pair<float,float> &b)
    {
      return(std::make_pair(a.first + b.first, a.second + b.second));
    }

    std::pair<float,float> subtract(const std::pair<float,float> &a, const std::pair<float,float> &b)
    {
      return(std::make_pair((a.first - b.first) * 2, a.second - b.second));
    }
  };

  struct FXTwoDirection : Effect
  {
    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 8.0;
      if(vp1 == 0) vp1 = 1;

      return(mix(galacto->audio_buffer.valueAt((galacto->buffer_size / 2) - t), galacto->audio_buffer.valueAt(t)));
    }
  } fx_two_direction;

  struct FXDelays : Effect
  {
    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      return(
        mix(
          mix(galacto->audio_buffer.valueAt(t), galacto->audio_buffer.valueAt(t + (galacto->buffer_size / 2))),
          galacto->audio_buffer.valueAt(t + (galacto->buffer_size / 4))
        )
      );
    }
  } fx_delays;

  struct FXBytebeat1 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp2 = p2 * 16.0;

      offset = ((t*7)&div(t,vp2)) * .1;

      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_1;

  // Very similar to the one above, but with different ranges?
  struct FXBytebeat2 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = ((t*vp1)&div(t,vp2)) * .1;
      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_2;


  struct FXBytebeat3 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = ((t >> vp1) & t) * (t>>vp2);
      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_3;

  // CONSIDER REPLACING THIS ONE
  struct FXBytebeat4 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = mod((( mod(t,((76 - (t>>vp2)) % 11))) * (t>>1)), (47-(t>>(4+vp1)) % 41)) * .7;
      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_4;


  struct FXBytebeat5 : Effect
  {
    int previous_offset = 0;
    int next_offset = 0;
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      next_offset = ( t * (( t>>4| t>>vp1 ) & vp2)) & (vp2+5);
      offset = (previous_offset + next_offset) / 2;
      next_offset = offset;

      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_5;

  struct FXDizzy : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 5000;
      uint32_t vp2 = p2 * 6000;

      offset = sin(float(t/8.0) / vp1) * galacto->buffer_size;
      offset += sin(float(t/32.0) / vp2) * galacto->buffer_size;

      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_dizzy;

  struct FXSliceRepeat : Effect
  {
    int divisor = 4;
    int window_size;
    int offset = -1;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      divisor = int(p1 * 16.0);
      if(divisor <= 1) divisor = 2;

      window_size = galacto->buffer_size / divisor;
      if(++offset >= 0) offset = (-1 * window_size);

      return(mix(galacto->audio_buffer.valueAt(offset), galacto->audio_buffer.valueAt(t)));
    }

  } fx_slice_repeat;


  struct FXWavePacking : Effect
  {
    unsigned int divisor = 4;
    unsigned int phase = 2;
    int offset = -1;

    int period = 64;
    int sin_index = 0;
    float sin_amplitude = 0;

    bool sin_is_playing = false;
    std::pair<float, float> stereo_audio = { 0, 0 };

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      // HOW about flutter?  (sequenced muting)
      divisor = int(p1 * 4096.0);
      if(divisor < 12) divisor = 12;

      phase = float(divisor) * p2;
      if(phase < 1) phase = 1;

      if(t%divisor < phase)
      {
        stereo_audio = galacto->audio_buffer.valueAt(t);
      }
      else
      {
        stereo_audio = { 0, 0 };
      }

      return(stereo_audio);
    }

  } fx_wave_packing;

/*
  struct FXBytebeat6 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      // offset = (vp1&t*(4|(7&t>>13))>>(1&-t>>vp2))+(12&t*(t>>11&t>>13)*(3&-t>>9));
      // offset =
      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_6;
*/

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


/* 256 sin array

0.000000, 0.024541, 0.049068, 0.073565, 0.098017, 0.122411, 0.146730, 0.170962, 0.195090, 0.219101, 0.242980, 0.266713, 0.290285, 0.313682,
 0.336890, 0.359895, 0.382683, 0.405241, 0.427555, 0.449611, 0.471397, 0.492898, 0.514103, 0.534998, 0.555570, 0.575808, 0.595699, 0.615232
, 0.634393, 0.653173, 0.671559, 0.689541, 0.707107, 0.724247, 0.740951, 0.757209, 0.773010, 0.788346, 0.803208, 0.817585, 0.831470, 0.84485
4, 0.857729, 0.870087, 0.881921, 0.893224, 0.903989, 0.914210, 0.923880, 0.932993, 0.941544, 0.949528, 0.956940, 0.963776, 0.970031, 0.9757
02, 0.980785, 0.985278, 0.989177, 0.992480, 0.995185, 0.997290, 0.998795, 0.999699, 1.000000, 0.999699, 0.998795, 0.997290, 0.995185, 0.992
480, 0.989177, 0.985278, 0.980785, 0.975702, 0.970031, 0.963776, 0.956940, 0.949528, 0.941544, 0.932993, 0.923880, 0.914210, 0.903989, 0.89
3224, 0.881921, 0.870087, 0.857729, 0.844854, 0.831470, 0.817585, 0.803208, 0.788346, 0.773010, 0.757209, 0.740951, 0.724247, 0.707107, 0.6
89541, 0.671559, 0.653173, 0.634393, 0.615232, 0.595699, 0.575808, 0.555570, 0.534998, 0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.
405241, 0.382683, 0.359895, 0.336890, 0.313682, 0.290285, 0.266713, 0.242980, 0.219101, 0.195090, 0.170962, 0.146730, 0.122411, 0.098017, 0
.073565, 0.049068, 0.024541, 0.000000, -0.024541, -0.049068, -0.073565, -0.098017, -0.122411, -0.146730, -0.170962, -0.195090, -0.219101, -
0.242980, -0.266713, -0.290285, -0.313682, -0.336890, -0.359895, -0.382683, -0.405241, -0.427555, -0.449611, -0.471397, -0.492898, -0.51410
3, -0.534998, -0.555570, -0.575808, -0.595699, -0.615232, -0.634393, -0.653173, -0.671559, -0.689541, -0.707107, -0.724247, -0.740951, -0.7
57209, -0.773010, -0.788346, -0.803208, -0.817585, -0.831470, -0.844854, -0.857729, -0.870087, -0.881921, -0.893224, -0.903989, -0.914210,
-0.923880, -0.932993, -0.941544, -0.949528, -0.956940, -0.963776, -0.970031, -0.975702, -0.980785, -0.985278, -0.989177, -0.992480, -0.9951
85, -0.997290, -0.998795, -0.999699, -1.000000, -0.999699, -0.998795, -0.997290, -0.995185, -0.992480, -0.989177, -0.985278, -0.980785, -0.
975702, -0.970031, -0.963776, -0.956940, -0.949528, -0.941544, -0.932993, -0.923880, -0.914210, -0.903989, -0.893224, -0.881921, -0.870087,
 -0.857729, -0.844854, -0.831470, -0.817585, -0.803208, -0.788346, -0.773010, -0.757209, -0.740951, -0.724247, -0.707107, -0.689541, -0.671
559, -0.653173, -0.634393, -0.615232, -0.595699, -0.575808, -0.555570, -0.534998, -0.514103, -0.492898, -0.471397, -0.449611, -0.427555, -0
.405241, -0.382683, -0.359895, -0.336890, -0.313682, -0.290285, -0.266713, -0.242980, -0.219101, -0.195090, -0.170962, -0.146730, -0.122411
, -0.098017, -0.073565, -0.049068, -0.024541,

*/
