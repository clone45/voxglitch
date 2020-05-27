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
        std::string path = "test.wav";
        sample.save_recorded_audio(path);
        recording = false;

        // Send a message to the GrainEngineMK2 on the right to load the
        // newly saved .wav file.
        GrainEngineExpanderMessage *buffer = (GrainEngineExpanderMessage *) rightExpander.module->leftExpander.producerMessage;
        buffer->path = path;
        buffer->sample_slot = 0;
        buffer->message_received = false;
      }
		}
		else
    {
			// No Grain Engine MK2 to the right, so do nothing
		}
	}
};
