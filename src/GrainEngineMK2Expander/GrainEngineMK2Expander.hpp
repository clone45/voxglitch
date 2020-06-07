struct GrainEngineMK2Expander : Module
{
  dsp::SchmittTrigger record_start_input_trigger;
  dsp::SchmittTrigger record_stop_input_trigger;
  dsp::SchmittTrigger record_start_button_trigger;
  dsp::SchmittTrigger record_stop_button_trigger;
  std::string patch_uuid = "";
  bool recording = false;

  // TODO: make this a Sample *, then after the audio is recorded, pass the
  // Sample pointer to GrainEngineMK2.  GrainEngineMK2 will then free its
  // sample at that slot and point to this new one.

  // Step #1: expander module passes path, sample_slot, message_received flag, and Sample pointer to GEMK2
  // Step #2: GEMK2 ditches its current saple at sample_slot and points to the new one
  // Step #3: GEMK2 puts the sample path string in it's array of loaded paths so it can be displayed in the right-click menu
  // Step #4: expander module must write the sample contents out the the file so that it be loaded by GEMK2 on reboot
  //          ^ this may cause a short delay in the expander, but I don't see that as a big issue
  // Q: If the path is set, should it be reused?  Won't this start to create a bunch of temp files on the user's drive?
  //    ^ possible solution: The expander create a UUID for itself on startup if one doesn't already exist.
  //      The UUID is used in the sample path, like expander_[uuid]_s1.wav (for slot 1).  This should work?
  //
  // Sample passing:
  // - when a recording has been stopped, a pointer to the Sample holding the sample data is passed to the mother module
  // - in the expander, a new sample is initiated and the internal pointer is set to the new sample
  // - when the mother module receives a sample pointer, it assigns one of the sample sloat sample pointers to this new one and __deletes the old one__
  //   ^ this is to keep from ending up with the everybody pointing to the sample sample instance

  Sample *sample = new Sample();

  // Sample sample;

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

  ~GrainEngineMK2Expander()
  {
    delete sample;
    sample = NULL;
  }

  GrainEngineMK2Expander()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(RECORD_START_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordStartButtonParam");
    configParam(RECORD_STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordEndButtonParam");
    configParam(SAMPLE_SLOT_KNOB_PARAM, 0.f, 4.f, 0.f, "SampleSlotKnobParam");
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

		if (rightExpander.module && rightExpander.module->model == modelGrainEngineMK2)
    {

      float left = inputs[AUDIO_IN_LEFT].getVoltage();
      float right = inputs[AUDIO_IN_RIGHT].getVoltage();

      bool start_recording = record_start_button_trigger.process(params[RECORD_START_BUTTON_PARAM].getValue()) || record_start_input_trigger.process(inputs[RECORD_START_INPUT].getVoltage());
      bool stop_recording = record_stop_button_trigger.process(params[RECORD_STOP_BUTTON_PARAM].getValue()) || record_stop_input_trigger.process(inputs[RECORD_STOP_INPUT].getVoltage());

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
        unsigned int sample_slot = inputs[SAMPLE_SLOT_INPUT].getVoltage() / 2;
        sample_slot += params[SAMPLE_SLOT_KNOB_PARAM].getValue();
        sample_slot = clamp(sample_slot, 0, 4);

        if(patch_uuid == "") patch_uuid = random_string(12);
        sample->filename = "grain_engine_" + patch_uuid + "_s" + std::to_string(sample_slot) + ".wav";
        sample->path = "";


        // Send a message to the GrainEngineMK2 "mother" on the right to load the newly saved .wav file
        // Notice that no sample data is passed to the GrainEngineMK2.  That may change.
        GrainEngineExpanderMessage *message_to_mother = (GrainEngineExpanderMessage *) rightExpander.module->leftExpander.producerMessage;
        // message_to_mother->path = path;
        message_to_mother->sample_slot = sample_slot;
        message_to_mother->message_received = false;
        message_to_mother->sample = sample;

        // Save recorded audio.  Note that this is done after the sample information
        // is passed to the mother
        sample->save_recorded_audio(sample->path + sample->filename);
        sample = new Sample();  // the old sample will be deleted by the mother module

        recording = false;
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
