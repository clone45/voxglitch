struct GrainEngineMK2Expander : Module
{
  dsp::SchmittTrigger record_start_input_trigger;
  dsp::SchmittTrigger record_stop_input_trigger;
  dsp::SchmittTrigger record_start_button_trigger;
  dsp::SchmittTrigger record_stop_button_trigger;
  bool recording = false;
  Sample sample;

  enum ParamIds {
    RECORD_START_BUTTON_PARAM,
    RECORD_STOP_BUTTON_PARAM,
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
    RECORDING_LIGHT,
    STOPPED_LIGHT,
    NUM_LIGHTS
  };

  GrainEngineMK2Expander()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(RECORD_START_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordStartButtonParam");
    configParam(RECORD_STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordEndButtonParam");
  }

	void process(const ProcessArgs &args) override {

		if (rightExpander.module && rightExpander.module->model == modelGrainEngineMK2)
    {

      float left = inputs[AUDIO_IN_LEFT].getVoltage();
      float right = inputs[AUDIO_IN_RIGHT].getVoltage();
      unsigned int sample_slot = inputs[SAMPLE_SLOT_INPUT].getVoltage() / 2;

      sample_slot = clamp(sample_slot, 0, 4);

      bool start_recording = record_start_button_trigger.process(params[RECORD_START_BUTTON_PARAM].getValue()) || record_start_input_trigger.process(inputs[RECORD_START_INPUT].getVoltage());

      if(start_recording)
      {
        sample.initialize_recording();
        recording = true;
      }

      if(recording)
      {
        sample.record_audio(left, right);
      }

      bool stop_recording = record_stop_button_trigger.process(params[RECORD_STOP_BUTTON_PARAM].getValue()) || record_stop_input_trigger.process(inputs[RECORD_STOP_INPUT].getVoltage());

      if(stop_recording)
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

      lights[RECORDING_LIGHT].setBrightness(recording == true);
      lights[STOPPED_LIGHT].setBrightness(recording == false);
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
