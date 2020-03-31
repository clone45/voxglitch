struct XY : Module
{
  Vec drag_position;
  std::vector<Vec> recording_memory;
  unsigned int mode = MODE_PLAYBACK;
  unsigned int playback_index = 0;
  dsp::SchmittTrigger clkTrigger;
  dsp::SchmittTrigger reset_trigger;
  bool tablet_mode = false;

  enum ParamIds {
    RETRIGGER_SWITCH,
    PUNCH_SWITCH,
    NUM_PARAMS
  };
  enum InputIds {
    CLK_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    X_OUTPUT,
    Y_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  //
  // Constructor
  //
  XY()
  {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(RETRIGGER_SWITCH, 0.f, 1.f, 1.f, "Retrigger");
    configParam(PUNCH_SWITCH, 0.f, 1.f, 0.f, "Punch");
  }

  json_t *dataToJson() override
  {
    json_t *root = json_object();

    json_t *recording_memory_json_array = json_array();

    for(Vec position : recording_memory)
    {
      json_t *xy_vec = json_array();
      json_array_append_new(xy_vec, json_real(position.x));
      json_array_append_new(xy_vec, json_real(position.y));

      json_array_append_new(recording_memory_json_array, xy_vec);
    }

    json_object_set(root, "recording_memory_data", recording_memory_json_array);
    json_decref(recording_memory_json_array);

    //
    // Save tablet mode
    //
    json_object_set_new(root, "tablet_mode", json_integer(tablet_mode));

    return root;
  }

  void dataFromJson(json_t *root) override
  {
    json_t *recording_memory_data = json_object_get(root, "recording_memory_data");

    if(recording_memory_data)
    {
      recording_memory.clear();
      size_t i;
      json_t *json_array_pair_xy;

      json_array_foreach(recording_memory_data, i, json_array_pair_xy)
      {
        float x = json_real_value(json_array_get(json_array_pair_xy, 0));
        float y = json_real_value(json_array_get(json_array_pair_xy, 1));
        recording_memory.push_back(Vec(x,y));
      }
    }

    json_t* tablet_mode_json = json_object_get(root, "tablet_mode");
    if (tablet_mode_json) tablet_mode = json_integer_value(tablet_mode_json);
  }

  void process(const ProcessArgs &args) override
  {
    if (reset_trigger.process(inputs[RESET_INPUT].getVoltage()))
    {
      playback_index = 0;
    }

    if (inputs[CLK_INPUT].isConnected())
    {
      if (clkTrigger.process(inputs[CLK_INPUT].getVoltage()))
      {
        if(mode == MODE_PUNCH_RECORDING)
        {
          if(recording_memory.size() > 0)
          {
            if(params[RETRIGGER_SWITCH].getValue())
            {
              // Restart playback if we've reached the end
              if(playback_index >= recording_memory.size()) playback_index = 0;
            }

            if(playback_index < recording_memory.size())
            {
              // This will cause the XYDisplay to animate
              // this->drag_position = recording_memory[playback_index];
              recording_memory[playback_index] = drag_position;
              // this->drag_position = recording_memory[playback_index];

              // Output the voltages
              outputs[X_OUTPUT].setVoltage((drag_position.x / DRAW_AREA_WIDTH_PT) * 10.0f);
              outputs[Y_OUTPUT].setVoltage(((DRAW_AREA_HEIGHT_PT - drag_position.y) / DRAW_AREA_HEIGHT_PT) * 10.0f);

              // Step to the next recorded x,y position
              playback_index += 1;
            }
          }
        }

        if(mode == MODE_RECORDING)
        {

          // Output the voltages
          outputs[X_OUTPUT].setVoltage((drag_position.x / DRAW_AREA_WIDTH_PT) * 10.0f);
          outputs[Y_OUTPUT].setVoltage(((DRAW_AREA_HEIGHT_PT - drag_position.y) / DRAW_AREA_HEIGHT_PT) * 10.0f);

          // Store the mouse x,y position
          recording_memory.push_back(drag_position);

        }

        if(mode == MODE_PLAYBACK)
        {
          if(recording_memory.size() > 0)
          {
            if(params[RETRIGGER_SWITCH].getValue())
            {
              // Restart playback if we've reached the end
              if(playback_index >= recording_memory.size()) playback_index = 0;
            }

            if(playback_index < recording_memory.size())
            {
              // This will cause the XYDisplay to animate
              this->drag_position = recording_memory[playback_index];

              // Output the voltages
              outputs[X_OUTPUT].setVoltage((drag_position.x / DRAW_AREA_WIDTH_PT) * 10.0f);
              outputs[Y_OUTPUT].setVoltage(((DRAW_AREA_HEIGHT_PT - drag_position.y) / DRAW_AREA_HEIGHT_PT) * 10.0f);

              // Step to the next recorded x,y position
              playback_index += 1;
            }
          }
        }
      }
    }
    else // CLK input is not connected
    {
      outputs[X_OUTPUT].setVoltage((drag_position.x / DRAW_AREA_WIDTH_PT) * 10.0f);
      outputs[Y_OUTPUT].setVoltage(((DRAW_AREA_HEIGHT_PT - drag_position.y) / DRAW_AREA_HEIGHT_PT) * 10.0f);
    }
  }

  void start_punch_recording()
  {
    mode = MODE_PUNCH_RECORDING;
  }

  void start_recording()
  {
    recording_memory.clear();
    mode = MODE_RECORDING;
  }

  void start_playback()
  {
    playback_index = 0;
    mode = MODE_PLAYBACK;
  }

  void continue_playback()
  {
    mode = MODE_PLAYBACK;
  }

  float get_punch_switch_value()
  {
    return(params[PUNCH_SWITCH].getValue());
  }
};
