struct GrainEngineMK2Expander : Module
{
  dsp::SchmittTrigger record_start_trigger;
  dsp::SchmittTrigger record_stop_trigger;
  bool recording = false;
  Sample sample;

  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    RECORD_START_INPUT,
    RECORD_STOP_INPUT,
    AUDIO_IN_LEFT,
    AUDIO_IN_RIGHT,
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  GrainEngineMK2Expander()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  }

	void process(const ProcessArgs &args) override {
		if (rightExpander.module && rightExpander.module->model == modelGrainEngineMK2)
    {
      // Send a message to the GrainEngineMK2 on the right
      /*
      float *buffer = (float*) rightExpander.module->leftExpander.producerMessage;
      buffer[0] = 1.f;
      */
      if(record_start_trigger.process(inputs[RECORD_START_INPUT].getVoltage()))
      {
        sample.initialize_recording();
        recording = true;
      }


      if(recording)
      {
        float left = inputs[AUDIO_IN_LEFT].getVoltage();
        float right = inputs[AUDIO_IN_RIGHT].getVoltage();
        sample.record_audio(left, right);
      }

      if(record_stop_trigger.process(inputs[RECORD_STOP_INPUT].getVoltage()))
      {
        sample.save_recorded_audio("test.wav");
        recording = false;
      }
		}
		else
    {
			// No Grain Engine MK2 to the right, so do nothing
		}
	}
};
