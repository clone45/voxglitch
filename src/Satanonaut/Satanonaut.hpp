struct Satanonaut : Module
{
  uint8_t w = 0;     // w is the output of the equations
  int offset = 0;
  uint8_t previous = 0;
  unsigned int t = 0;
  unsigned int selected_effect = 0;
  float param_1_input = 0.0;
  float param_2_input = 0.0;

  SatanonautStereoAudioBuffer audio_buffer;

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

  // Satanonaut Contructor
	Satanonaut()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(PARAM_1_KNOB, 0.0f, 1.0f, 0.0f, "ParamKnob1");
    configParam(PARAM_2_KNOB, 0.0f, 1.0f, 0.0f, "ParamKnob2");

    configParam(BUFFER_SIZE_KNOB, 0.0f, 1.0f, 1.0f, "BufferSizeKnob");
    configParam(FEEDBACK_KNOB, 0.0f, 1.0f, 0.0f, "FeedbackKnob");
    configParam(EFFECT_KNOB, 0, NUMBER_OF_EFFECTS, 0, "EffectKnob");
    // configParam(EFFECT_KNOB, 0.0f, 1.0f, 0.0f, "EffectKnob");
    configParam(DRIVE_KNOB, 1, 60, 1, "DriveKnob");

    rack::random::init();
    audio_buffer.purge();

		configBypass(AUDIO_INPUT_LEFT, AUDIO_OUTPUT_LEFT);
		configBypass(AUDIO_INPUT_RIGHT, AUDIO_OUTPUT_RIGHT);
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

  float standard_attenuverter_input(int input_index, int knob_index)
  {
    float output = 0.0;
    float input_value = inputs[input_index].getVoltage() / 10.0; // -1 to 1, normally
    float knob_value = (params[knob_index].getValue() * 2.0) - 1; // -1 to 1

    output = clamp(input_value + knob_value, 0.0, 1.0);

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
      case 13:
        std::tie(left_audio, right_audio) = fx_long_play.process(this, t, param_1_input, param_2_input);
        break;
    }

    outputs[AUDIO_OUTPUT_LEFT].setVoltage(left_audio * drive);
    outputs[AUDIO_OUTPUT_RIGHT].setVoltage(right_audio * drive);
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

  //
  // EFFECT #0
  //

  struct FXTwoDirection : Effect
  {
    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = (p1 * 15.0) + 1;

      float vp2 = 1.0;
      if(p2 > .1) vp2 = (p2 * 8.0) - 4.0;

      return(mix(satanonaut->audio_buffer.valueAt((satanonaut->buffer_size / vp1) - t), satanonaut->audio_buffer.valueAt(t * vp2)));
    }
  } fx_two_direction;

  //
  // EFFECT #1
  //

  struct FXDelays : Effect
  {
    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = (p1 * 6) + 1;
      uint32_t vp2 = (p2 * 6) + 1;

      return(
        mix(
          mix(satanonaut->audio_buffer.valueAt(t), satanonaut->audio_buffer.valueAt(t + (satanonaut->buffer_size / vp1))),
          satanonaut->audio_buffer.valueAt(t + (satanonaut->buffer_size / vp2))
        )
      );
    }
  } fx_delays;

  //
  // EFFECT #2
  //

  struct FXBytebeat1 : Effect
  {
    unsigned int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = (p1 * 14.0) + 1;
      uint32_t vp2 = (p2 * 4) + 3;

      // offset = ((t*vp1)&div(t,vp2)) * .1;
      offset = (((t>>2)|(t>>vp2)) - ((t<<7)|(t/vp1)) + ((t>>4)+(t<<2))%(offset + 1));
      offset = offset * (p1 * .01);

      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_1;

  //
  // EFFECT #3
  //
  // This doesn't seem to do anything
  struct FXBytebeat2 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 10991.0;
      uint32_t vp2 = (p2 * 22000) + 1;

      // offset = (((vp1&t)^mod((t>>2), vp2))) * .1;
      // offset = ((t>>6) & div((t<<3),mod( (t*(t>>vp1)),(vp2+ mod((t>>16),vp2) ))));

      offset = (vp1*(t/32)%vp2);

      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_2;

  //
  // EFFECT #4
  //

  struct FXBytebeat3 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 16.0;
      uint32_t vp2 = p2 * 32.0;

      offset = ((t >> vp1) & t) * (t>>vp2);
      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_3;

  //
  // EFFECT #5
  //

  // CONSIDER REPLACING THIS ONE
  struct FXBytebeat4 : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      offset = mod((( mod(t,((76 - (t>>vp2)) % 11))) * (t>>1)), (47-(t>>(4+vp1)) % 41)) * .7;
      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_4;

  //
  // EFFECT #6
  //

  struct FXBytebeat5 : Effect
  {
    int previous_offset = 0;
    int next_offset = 0;
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 32.0;
      uint32_t vp2 = p2 * 32.0;

      next_offset = ( t * (( t>>4| t>>vp1 ) & vp2)) & (vp2+5);
      offset = (previous_offset + next_offset) / 2;
      next_offset = offset;

      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_5;

  //
  // EFFECT #7
  //

  struct FXDizzy : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 5000;
      uint32_t vp2 = p2 * 6000;

      offset = sin(float(t/8.0) / vp1) * satanonaut->buffer_size;
      offset += sin(float(t/32.0) / vp2) * satanonaut->buffer_size;

      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_dizzy;

  //
  // EFFECT #8
  //

  struct FXSliceRepeat : Effect
  {
    int divisor = 4;
    int window_size;
    int offset = -1;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      divisor = int(p1 * 16.0);
      if(divisor <= 1) divisor = 2;

      window_size = satanonaut->buffer_size / divisor;
      if(++offset >= 0) offset = (-1 * window_size);

      return(mix(satanonaut->audio_buffer.valueAt(offset), satanonaut->audio_buffer.valueAt(t)));
    }

  } fx_slice_repeat;

  //
  // EFFECT #9
  //

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

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      // HOW about flutter?  (sequenced muting)
      divisor = int(p1 * 4096.0);
      if(divisor < 12) divisor = 12;

      phase = float(divisor) * p2;
      if(phase < 1) phase = 1;

      if(t%divisor < phase)
      {
        stereo_audio = satanonaut->audio_buffer.valueAt(t);
      }
      else
      {
        stereo_audio = { 0, 0 };
      }

      return(stereo_audio);
    }

  } fx_wave_packing;

  //
  // EFFECT #10
  //

  struct FXSmooth : Effect
  {
    int divisor = 32;
    unsigned int window_size = 44010;
    int offset = -1;
    std::pair<float, float> stereo_audio = {0,0};

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {

      // For this effect, the parameters need to be positive numbers
      if(p1 < 0) p1 = 0;
      if(p2 < 0) p2 = 0;

      if(satanonaut->buffer_size > 16)
      {
        stereo_audio = satanonaut->audio_buffer.valueAt(t);
        window_size = satanonaut->buffer_size / (int(p1 * 64)+1);
        if(window_size < 1) window_size = 1;

        divisor = int(p2*60) + 1;
        if(divisor < 1) divisor = 1;

        if((window_size/divisor) >= 1)
        {
          for(unsigned int i=1; i<window_size; i+=(window_size/divisor))
          {
            offset = (t-i) % satanonaut->buffer_size;
            stereo_audio = mix(stereo_audio, satanonaut->audio_buffer.valueAt(offset));
          }
        }

        stereo_audio = divide(stereo_audio, 8);
        return(stereo_audio);
      }
      else
      {
        return(satanonaut->audio_buffer.valueAt(t));
      }

    }
  } fx_smooth;

  //
  // EFFECT #11
  //
  struct FXFold : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      int vp1 = (p1 * 50.0) - 25;
      int vp2 = (p2 * 30.0) - 15;

      // if(vp1 == 0) vp1 = 1;

      offset = (t-t+t*vp1)|(t&(t/44100))|div(t,vp2);

      // offset = ((t/vp1)|t|((t>>1)&(t+(t>>(t*vp2)))))-(t/44100);

      return(satanonaut->audio_buffer.valueAt(offset));
    }
  } fx_fold;

  //
  // EFFECT #12
  //

  struct FXByteBeatAnxous : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = p1 * 50.0;
      uint32_t vp2 = p2 * 300.0;

      if(vp1 == 0) vp1 = 1;

      offset = ((t/vp1)|t|((t>>1)&(t+(t>>(t*vp2)))))-(t/44100);

      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_bytebeat_anxious;

  //
  // EFFECT #13
  //

  struct FXByteLongPlay : Effect
  {
    int offset = 0;

    std::pair<float, float> process(Satanonaut *satanonaut, unsigned int t, float p1, float p2)
    {
      uint32_t vp1 = (p1 * 22.0) + 1;
      p2 = p2 * .1;

      // offset = ((t>>2)|(t>>2)) - ((t<<7)|(t/22)) + ((t>>4)+(t<<2))%101;
      offset = (((t>>2)|(t>>2)) - ((t<<7)|(t/vp1)) + ((t>>4)+(t<<2))%101) * p2;

      return(satanonaut->audio_buffer.valueAt(t + offset));
    }
  } fx_long_play;

};
