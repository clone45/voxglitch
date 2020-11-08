struct ByteBeat : Module
{
  float left_audio = 0;
  float right_audio = 0;

  uint32_t t; // 32 bits
  uint8_t w;

  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
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

	ByteBeat()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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

    t = t + 1;

    float output = compute(t, 0, 0, 0);

    outputs[AUDIO_OUTPUT_LEFT].setVoltage(output);
    // outputs[AUDIO_MIX_OUTPUT_RIGHT].setVoltage(summed_output_right);
  }

  float compute(uint32_t t, uint32_t p1, uint32_t p2, uint32_t p3)
  {
    // w = (((t/(p3+1)*t)%p1)&(t-t/p2*632));
    // Try: (t>>13&t)*(t>>8)
    w = (uint8_t) ((t>>13&t)*(t>>8));
    return(w / 256.0);
  }
};
