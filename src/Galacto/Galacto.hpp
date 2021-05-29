//
// TODO:
// * get sample modulation working
// * look at how other modulation works right now.  Double check that it's working OK
// * Param 1,2 to first few effects


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
  float drive = 1;

  enum ParamIds {
    CLOCK_DIVISION_KNOB,
    BUFFER_SIZE_KNOB,
    FEEDBACK_KNOB,
    EFFECT_KNOB,
    PARAM_1_KNOB,
    PARAM_2_KNOB,
    PURGE_BUTTON,
    DRIVE_KNOB,
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
    configParam(EFFECT_KNOB, 0, NUMBER_OF_EFFECTS, 0, "EffectKnob");
    // configParam(EFFECT_KNOB, 0.0f, 1.0f, 0.0f, "EffectKnob");
    configParam(DRIVE_KNOB, 1, 60, 1, "DriveKnob");

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


  float attenuverter_input(int input_index, int knob_index)
  {
    float output = 0.0;
    float input_value = inputs[input_index].getVoltage() / 10.0; // -1 to 1, normally
    float knob_value = params[knob_index].getValue(); // 0 to 1

    if(inputs[input_index].isConnected())
    {
      output = clamp(input_value * knob_value, 0.0, 1.0);
    }
    else
    {
      output = knob_value;
    }

    return(output);
  }

  int snapped_attenuverter_input(int input_index, int knob_index, int min_value, int max_value)
  {
    // float input_value = inputs[input_index].getVoltage() / 10.0; // -1 to 1, normally
    // float knob_value = params[knob_index].getValue(); // 0 to 1

    int output = 0;

    if(inputs[input_index].isConnected())
    {
      output = params[knob_index].getValue() * (inputs[input_index].getVoltage() / 10.0);
    }
    else
    {
      output = params[knob_index].getValue();
    }

    output = clamp(output, min_value, max_value);

    return(output);
  }

	void process(const ProcessArgs &args) override
	{
    bool purge_button_is_triggered = purge_button_schmitt_trigger.process(params[PURGE_BUTTON].getValue());
    if(purge_button_is_triggered) audio_buffer.purge();

    //
    // Read knobs and inputs
    //
    selected_effect = snapped_attenuverter_input(EFFECT_INPUT, EFFECT_KNOB, 0, NUMBER_OF_EFFECTS);
    param_1_input = attenuverter_input(PARAM_1_INPUT, PARAM_1_KNOB); // ranges from 0 to 1
    param_2_input = attenuverter_input(PARAM_2_INPUT, PARAM_2_KNOB); // ranges from 0 to 1
    buffer_size = clamp((int) (attenuverter_input(BUFFER_SIZE_INPUT, BUFFER_SIZE_KNOB) * (float) MAX_BUFFER_SIZE), MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
    feedback = clamp(attenuverter_input(FEEDBACK_INPUT, FEEDBACK_KNOB), 0.0, 1.0);
    drive = params[DRIVE_KNOB].getValue();

    // selected_effect = clamp((int) (( /* params[EFFECT_INPUT].getValue() * (float)NUMBER_OF_EFFECTS) +*/ params[EFFECT_KNOB].getValue()), 0, NUMBER_OF_EFFECTS);



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
      case 10:
        std::tie(left_audio, right_audio) = fx_smooth.process(this, t, param_1_input, param_2_input);
        break;
      case 11:
        std::tie(left_audio, right_audio) = fx_fold.process(this, t, param_1_input, param_2_input);
        break;
      case 12:
        std::tie(left_audio, right_audio) = fx_bytebeat_anxious.process(this, t, param_1_input, param_2_input);
        break;
    }

    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_audio * drive);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_audio * drive);

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

    std::pair<float,float> divide(const std::pair<float,float> &a, unsigned int divisor)
    {
      if(divisor == 0) divisor = 1;
      return(std::make_pair(a.first / divisor, a.second / divisor));
    }

    // Folder code from Squinky Labs: https://github.com/squinkylabs/SquinkyVCV/blob/3a5fbaae4956737c77d0494b69149747c25726af/dsp/utils/AudioMath.h#L162
    float fold(float x, float bounds)
    {
        float fold;
        const float bias = (x < 0) ? (-1 * bounds) : bounds;
        int phase = int((x + bias) / 2.f);
        bool isEven = !(phase & 1);
        if (isEven) {
            fold = x - 2.f * phase;
        } else {
            fold = -x + 2.f * phase;
        }
        return fold;
    }
  };

  struct FXTwoDirection : Effect
  {
    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = (p1 * 15.0) + 1;

      float vp2 = 1.0;
      if(p2 > .1) vp2 = (p2 * 8.0) - 4.0;

      return(mix(galacto->audio_buffer.valueAt((galacto->buffer_size / vp1) - t), galacto->audio_buffer.valueAt(t * vp2)));
    }
  } fx_two_direction;

  struct FXDelays : Effect
  {
    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = (p1 * 6) + 1;
      uint32_t vp2 = (p2 * 6) + 1;

      return(
        mix(
          mix(galacto->audio_buffer.valueAt(t), galacto->audio_buffer.valueAt(t + (galacto->buffer_size / vp1))),
          galacto->audio_buffer.valueAt(t + (galacto->buffer_size / vp2))
        )
      );
    }
  } fx_delays;

  struct FXBytebeat1 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = (p1 * 7.0) + 3;
      uint32_t vp2 = p2 * 16.0;

      offset = ((t*vp1)&div(t,vp2)) * .1;

      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_1;

  struct FXBytebeat2 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = ((vp1&t^mod((t>>2), vp2))) * .1;
      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_2;


  struct FXBytebeat3 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 16.0;
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

  struct FXSmooth : Effect
  {
    int divisor = 32;
    unsigned int window_size = 44010;
    int offset = -1;
    std::pair<float, float> stereo_audio = {0,0};

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {

      // For this effect, the parameters need to be positive numbers
      if(p1 < 0) p1 = 0;
      if(p2 < 0) p2 = 0;

      if(galacto->buffer_size > 16)
      {
        stereo_audio = galacto->audio_buffer.valueAt(t);
        window_size = galacto->buffer_size / (int(p1 * 64)+1);
        if(window_size < 1) window_size = 1;

        divisor = int(p2*60) + 1;
        if(divisor < 1) divisor = 1;

        if((window_size/divisor) >= 1)
        {
          for(unsigned int i=1; i<window_size; i+=(window_size/divisor))
          {
            offset = (t-i) % galacto->buffer_size;
            stereo_audio = mix(stereo_audio, galacto->audio_buffer.valueAt(offset));
          }
        }

        stereo_audio = divide(stereo_audio, 8);
        return(stereo_audio);
      }
      else
      {
        return(galacto->audio_buffer.valueAt(t));
      }

    }
  } fx_smooth;


  struct FXFold : Effect
  {
    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      std::pair<float, float> stereo_audio = {0,0};

      float gain = (p1 * 20.0) + 1;
      float bounds = p2 + 0.5; // ranges from .5 to 1.5
      stereo_audio.first = fold(galacto->audio_buffer.valueAt(t).first * gain, bounds) / 2.0;
      stereo_audio.second = fold(galacto->audio_buffer.valueAt(t).second * gain, bounds) / 2.0;

      return(stereo_audio);
    }
  } fx_fold;


  struct FXByteBeatAnxous : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 50.0;
      uint32_t vp2 = p2 * 300.0;

      if(vp1 == 0) vp1 = 1;

      offset = ((t/vp1)|t|((t>>1)&(t+(t>>(t*vp2)))))-(t/44100);

      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_anxious;


  struct FXByteLongPlay : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Galacto *galacto, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 50.0;
      uint32_t vp2 = p2 * 300.0;

      if(vp1 == 0) vp1 = 1;

      offset = ((t>>2)|(t>>2)) - ((t<<7)|(t/22)) + ((t>>4)+(t<<2))%101;

      return(galacto->audio_buffer.valueAt(t + offset));
    }
  } fx_long_play;

/*
((t/43)|t|(t>>1)&t+(t>>(t)))-(t/44100)

((t/43)|t|(t>>1)&t+(t>>(t*323.0)))-(t/44100)

((t/43)|t|(t>>1)&t+(t>>(t+45)))+(t/44100)

((t/43)|t|(t>>3)&t+(t>>(t+3)))+((t+66)/44100)


((t/43)|t|(t>>3)&t-(t>>(t+3)))+((t+66)/44100)
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
