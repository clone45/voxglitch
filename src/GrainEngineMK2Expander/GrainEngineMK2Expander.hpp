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
    SAMPLE_SLOT_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    PASSTHROUGH_LEFT,
    PASSTHROUGH_RIGHT,
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

      float left = inputs[AUDIO_IN_LEFT].getVoltage();
      float right = inputs[AUDIO_IN_RIGHT].getVoltage();
      unsigned int sample_slot = inputs[SAMPLE_SLOT_INPUT].getVoltage() / 2;

      sample_slot = clamp(sample_slot, 0, 4);

      if(record_start_trigger.process(inputs[RECORD_START_INPUT].getVoltage()))
      {
        sample.initialize_recording();
        recording = true;
      }

      if(recording)
      {
        sample.record_audio(left, right);
      }

      if(record_stop_trigger.process(inputs[RECORD_STOP_INPUT].getVoltage()))
      {
        std::string path = "grain_engine_" + random_string(12) + ".wav";
        sample.save_recorded_audio(path);
        recording = false;

        // Send a message to the GrainEngineMK2 on the right to load the newly saved .wav file.
        GrainEngineExpanderMessage *buffer = (GrainEngineExpanderMessage *) rightExpander.module->leftExpander.producerMessage;
        buffer->path = path;
        buffer->sample_slot = sample_slot;
        buffer->message_received = false;
      }

      outputs[PASSTHROUGH_LEFT].setVoltage(left);
      outputs[PASSTHROUGH_RIGHT].setVoltage(right);
		}
		else
    {
			// No Grain Engine MK2 to the right, so do nothing
		}
	}

  std::string random_string( size_t length )
  {
      auto randchar = []() -> char
      {
          const char charset[] =
          "0123456789"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "abcdefghijklmnopqrstuvwxyz";
          const size_t max_index = (sizeof(charset) - 1);
          return charset[ rand() % max_index ];
      };
      std::string str(length,0);
      std::generate_n( str.begin(), length, randchar );
      return str;
  }
};
