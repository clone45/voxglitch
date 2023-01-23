struct GrainEngineMK2Expander : Module
{
  dsp::SchmittTrigger record_start_input_trigger;
  dsp::SchmittTrigger record_stop_input_trigger;
  dsp::SchmittTrigger record_start_button_trigger;
  dsp::SchmittTrigger record_stop_button_trigger;
  std::string patch_uuid = "";
  bool recording = false;

  Sample *sample = new Sample();

  enum ParamIds {
    RECORD_START_BUTTON_PARAM,
    RECORD_STOP_BUTTON_PARAM,
    SAMPLE_SLOT_KNOB_PARAM,
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

  // Constructor
  GrainEngineMK2Expander()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(RECORD_START_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordStartButtonParam");
    configParam(RECORD_STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordEndButtonParam");
    configParam(SAMPLE_SLOT_KNOB_PARAM, 0.f, 4.f, 0.f, "SampleSlotKnobParam");

    // Make sure that the folder where recorded audio is saved exists.  If not, create it.
    std::string path = asset::user(WAV_FOLDER_NAME);

    if(! system::isDirectory(path))
    {
      system::createDirectory(path);
    }
  }

  // Destructor
  ~GrainEngineMK2Expander()
  {
    delete sample;
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();
    json_object_set_new(root, "patch_uuid", json_string(patch_uuid.c_str()));
    return root;
  }

  void dataFromJson(json_t *root) override
  {
    json_t* patch_uuid_json = json_object_get(root, "patch_uuid");
    if (patch_uuid_json) patch_uuid = json_string_value(patch_uuid_json);

    if(patch_uuid == "") patch_uuid = random_string(12);
  }

	void process(const ProcessArgs &args) override {

    // Send a message to the GrainEngineMK2 "mother" on the right to load the newly saved .wav file
    // Notice that no sample data is passed to the GrainEngineMK2.  That may change.

		// if (rightExpander.module && rightExpander.module->model == modelGrainEngineMK2)
    if (leftExpander.module && leftExpander.module->model == modelGrainEngineMK2)
    {
      float left = inputs[AUDIO_IN_LEFT].getVoltage();
      float right = inputs[AUDIO_IN_RIGHT].getVoltage();

      bool start_recording = record_start_button_trigger.process(params[RECORD_START_BUTTON_PARAM].getValue()) || record_start_input_trigger.process(inputs[RECORD_START_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);
      bool stop_recording = record_stop_button_trigger.process(params[RECORD_STOP_BUTTON_PARAM].getValue()) || record_stop_input_trigger.process(inputs[RECORD_STOP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger);

      if(recording && start_recording)
      {
        stop_recording = true;
      }

      if(! stop_recording)
      {
        if(start_recording)
        {
          sample->initialize_recording();
          recording = true;
        }

        if(recording)
        {
          sample->record_audio(left, right);
        }
      }

      if(stop_recording)
      {
        recording = false;

        // Get sample slot for where GEMK2 should store the sample
        unsigned int sample_slot = inputs[SAMPLE_SLOT_INPUT].getVoltage();
        sample_slot += params[SAMPLE_SLOT_KNOB_PARAM].getValue();
        sample_slot = clamp(sample_slot, 0, 4);

        // Build up a filename for the .wav file in the format grain_engine_[patch_uuid]_s[sample_slot].wave
        if(patch_uuid == "") patch_uuid = random_string(12);
        std::string path = asset::user(WAV_FOLDER_NAME);
        std::string filename = "grain_engine_" + patch_uuid + "_s" + std::to_string(sample_slot) + ".wav";

        // Prepare message for sending to Grain Engine MK2
        // GrainEngineExpanderMessage *message_to_grain_engine = (GrainEngineExpanderMessage *) rightExpander.module->leftExpander.producerMessage;
        GrainEngineExpanderMessage *message_to_grain_engine = (GrainEngineExpanderMessage *) leftExpander.module->rightExpander.producerMessage;
        message_to_grain_engine->sample_slot = sample_slot;
        message_to_grain_engine->path = path;
        message_to_grain_engine->filename = filename;

        // Save the recorded audio to disk
        sample->save_recorded_audio(path + "/" + filename);

        // Tell Grain Engine MK2 that the message is ready for receiving
        message_to_grain_engine->message_received = false;
      }

      outputs[PASSTHROUGH_LEFT].setVoltage(left);
      outputs[PASSTHROUGH_RIGHT].setVoltage(right);

      lights[RECORDING_LIGHT].setBrightness(recording == true);
      lights[STOPPED_LIGHT].setBrightness(recording == false);
		}
		else
    {
			// No Grain Engine MK2 to the right
      lights[RECORDING_LIGHT].setBrightness(false);
      lights[STOPPED_LIGHT].setBrightness(false);
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
