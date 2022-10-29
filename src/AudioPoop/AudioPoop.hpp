struct AudioPoop : Module
{
  dsp::SchmittTrigger record_start_input_trigger;
  dsp::SchmittTrigger record_stop_input_trigger;
  dsp::SchmittTrigger record_start_button_trigger;
  dsp::SchmittTrigger record_stop_button_trigger;
  bool recording = false;

  Sample *sample = new Sample();
  std::string readBuffer;

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
  AudioPoop()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(RECORD_START_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordStartButtonParam");
    configParam(RECORD_STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "RecordEndButtonParam");
 
    // Make sure that the folder where recorded audio is saved exists.  If not, create it.
    std::string path = asset::user(WAV_FOLDER_NAME);

    if(! system::isDirectory(path))
    {
      system::createDirectory(path);
    }
    else
    {
      // DEBUG("Using path: ");
      // DEBUG(path.c_str());
    }
   
  }

  // Destructor
  ~AudioPoop()
  {
    curl_global_cleanup();
    delete sample;
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();
    return root;
  }

  void dataFromJson(json_t *root) override
  {
  }

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
  {
    DEBUG("result from WriteCallback: ");
    DEBUG((char*)contents);

    std::string filePath = "";
    std::string fileUrl = "";

    std::string escaped_filePath = "";
    std::string escaped_fileUrl = "";    
    // std::string accountId = "";

    json_error_t error;
    json_t *json_contents = json_loads((char*)contents, 0, &error);

    if(json_contents)
    {
      json_t* filePath_json = json_object_get(json_contents, "filePath");
      if (filePath_json) filePath = json_string_value(filePath_json);

      json_t* fileUrl_json = json_object_get(json_contents, "fileUrl");
      if (fileUrl_json) fileUrl = json_string_value(fileUrl_json);

      // json_t* accountId_json = json_object_get(json_contents, "accountId");
      // if (accountId_json) accountId = json_string_value(accountId_json);

      json_decref(json_contents);

      // TODO
      // Here's where I need to make a call to Xano to store the sample information
      // API endpoint:
      //   https://x8ki-letl-twmt.n7.xano.io/api:3ak3rVjk/sample
      // 
      // Method: POST
      // Payload
      // {
      //    "url": "string",
      //    "path": "string",
      //    "user_id": 0
      // }

      CURL *curl = curl_easy_init();

      if(curl) 
      {
        // Forgive me....
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        DEBUG("fileUrl=");
        DEBUG(fileUrl.c_str());

        escaped_fileUrl = curl_easy_escape(NULL, fileUrl.c_str(), 0);
        escaped_filePath = curl_easy_escape(NULL, filePath.c_str(), 0);

        DEBUG("escaped_fileUrl=");
        DEBUG(escaped_fileUrl.c_str());




        std::string post_parameters = std::string("url=") + escaped_fileUrl + std::string("&path=") + escaped_filePath;

        DEBUG("posting parameters");
        DEBUG(post_parameters.c_str());

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_parameters);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, "https://x8ki-letl-twmt.n7.xano.io/api:3ak3rVjk/sample");

        CURLcode res = curl_easy_perform(curl);
        
        /* Check for errors */
        if(res != CURLE_OK)
        {
          DEBUG("second CURL failed");
          DEBUG(curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
      }
    }

    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
  }

	void process(const ProcessArgs &args) override {

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
      recording = false;

      // Build up a filename for the .wav file in the format grain_engine_[patch_uuid]_s[sample_slot].wav
      std::string sample_uuid = random_string(12);
      std::string path = asset::user(WAV_FOLDER_NAME);
      std::string filename = "recording_" + sample_uuid + ".wav";
      std::string full_path = path + "/" + filename;

      DEBUG("full path:");
      DEBUG(full_path.c_str());

      // Save the recorded audio to disk
      sample->save_recorded_audio(full_path);

      CURL *curl = curl_easy_init();

      // Write sample to the cloud
      if(curl) 
      {
        CURLcode res;

        // Forgive me....
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        
        // Set content type
        struct curl_slist *headers=NULL;
        headers = curl_slist_append(headers, "Content-Type: audio/wav");
        headers = curl_slist_append(headers, "Authorization: Bearer public_W142hWDBoENDV9LwxiviYp8BSRKF");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        //
        // Read raw .wav file into char *
        //
        std::string contents;
        std::ifstream in(full_path, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
        }
        else
        {
          DEBUG("Unable to read in file to send");
        }

        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, contents.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, contents.data());
        curl_easy_setopt(curl, CURLOPT_POST,1);

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.upload.io/v2/accounts/W142hWD/uploads/binary");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK)
        {
          DEBUG("curl_easy_perform() failed");
          DEBUG(curl_easy_strerror(res));
          curl_easy_cleanup(curl);
        } 
      } 
      else
      {
        DEBUG("curl not defined");
      }
    }

    outputs[PASSTHROUGH_LEFT].setVoltage(left);
    outputs[PASSTHROUGH_RIGHT].setVoltage(right);

    lights[RECORDING_LIGHT].setBrightness(recording == true);
    lights[STOPPED_LIGHT].setBrightness(recording == false);
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
