//
// Hazumi - Tenori-on style bouncing ball sequencer

struct Hazumi : Module
{
  HazumiSequencer hazumi_sequencer;

  dsp::SchmittTrigger stepTrigger;
  dsp::SchmittTrigger resetTrigger;
  dsp::PulseGenerator gateOutputPulseGenerators[SEQUENCER_COLUMNS];
  bool trigger_results[SEQUENCER_COLUMNS];
  unsigned int gate_outputs[SEQUENCER_COLUMNS];

  std::string trigger_options_names[3] = { "Bottom", "Top", "Both" };

  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
    STEP_INPUT,
    RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
    ENUMS(GATE_OUTPUTS, SEQUENCER_COLUMNS),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Hazumi()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

  //
	// Autosave module data.  VCV Rack decides when this should be called.
  //
  // My saving and loading code could be more compact by saving arrays of
  // "ball" data tidily packaged up.  I'll do that if this code ever goes
  // through another iteration.
  //

	json_t *dataToJson() override
	{
    json_t *json_root = json_object();

    //
    // Save ball locations
    //

    json_t *ball_locations_json_array = json_array();
    for(int column_number=0; column_number < SEQUENCER_COLUMNS; column_number++)
    {
      json_array_append_new(ball_locations_json_array, json_integer(this->hazumi_sequencer.ball_locations[column_number]));
    }
    json_object_set(json_root, "ball_locations", ball_locations_json_array);
    json_decref(ball_locations_json_array);

    //
    // Save ball directions
    //

    json_t *ball_directions_json_array = json_array();
    for(int column_number=0; column_number < SEQUENCER_COLUMNS; column_number++)
    {
      json_array_append_new(ball_directions_json_array, json_integer(this->hazumi_sequencer.ball_directions[column_number]));
    }
    json_object_set(json_root, "ball_directions", ball_directions_json_array);
    json_decref(ball_directions_json_array);

    //
    // Save column heights
    //

    json_t *column_heights_json_array = json_array();
    for(int column_number=0; column_number < SEQUENCER_COLUMNS; column_number++)
    {
      json_array_append_new(column_heights_json_array, json_integer(this->hazumi_sequencer.column_heights[column_number]));
    }
    json_object_set(json_root, "column_heights", column_heights_json_array);
    json_decref(column_heights_json_array);

    //
    // Save trigger options
    //

    json_t *trigger_options_json_array = json_array();
    for(int column_number=0; column_number < SEQUENCER_COLUMNS; column_number++)
    {
      json_array_append_new(trigger_options_json_array, json_integer(this->hazumi_sequencer.trigger_options[column_number]));
    }
    json_object_set(json_root, "trigger_options", trigger_options_json_array);
    json_decref(trigger_options_json_array);

  	return json_root;
	}

	// Load module data
	void dataFromJson(json_t *json_root) override
	{
    // Load ball locations
    json_t *ball_locations_data = json_object_get(json_root, "ball_locations");
    if(ball_locations_data)
    {
      size_t i;
      json_t *location_json = NULL;
      json_array_foreach(ball_locations_data, i, location_json)
      {
        this->hazumi_sequencer.ball_locations[i] = json_integer_value(location_json);
      }
      //if(location_json) json_decref(location_json);
    }

    // Load ball directions
    json_t *ball_directions_data = json_object_get(json_root, "ball_directions");
    if(ball_directions_data)
    {
      size_t i;
      json_t *direction_json = NULL;
      json_array_foreach(ball_directions_data, i, direction_json)
      {
        this->hazumi_sequencer.ball_directions[i] = json_integer_value(direction_json);
      }
      //if(direction_json) json_decref(direction_json);
    }

    // Load column_heights
    json_t *column_heights_data = json_object_get(json_root, "column_heights");
    if(column_heights_data)
    {
      size_t i;
      json_t *height_json = NULL;
      json_array_foreach(column_heights_data, i, height_json)
      {
        this->hazumi_sequencer.column_heights[i] = json_integer_value(height_json);
      }
      //if (height_json) json_decref(height_json);
    }

    // Load trigger options data
    json_t *trigger_options_data = json_object_get(json_root, "trigger_options");
    if(trigger_options_data)
    {
      size_t i;
      json_t *trigger_option_json = NULL;
      json_array_foreach(trigger_options_data, i, trigger_option_json)
      {
        this->hazumi_sequencer.trigger_options[i] = json_integer_value(trigger_option_json);
      }
      //if (trigger_options_data) json_decref(trigger_options_data);
    }
	}

	void process(const ProcessArgs &args) override
	{

    // Process reset input
    if(resetTrigger.process(inputs[RESET_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
    {
      hazumi_sequencer.reset();
    }

    // Process Step Input
    if(stepTrigger.process(inputs[STEP_INPUT].getVoltage(), constants::gate_low_trigger, constants::gate_high_trigger))
    {
      hazumi_sequencer.step(trigger_results);

      for(unsigned int i=0; i < 8; i++)
      {
        if(trigger_results[i]) gateOutputPulseGenerators[i].trigger(0.01f);
      }
    }

    // Output gates
    for(unsigned int i=0; i < 8; i++)
    {
      bool trigger_output_pulse = gateOutputPulseGenerators[i].process(args.sampleTime);
      outputs[GATE_OUTPUTS + i].setVoltage((trigger_output_pulse ? 10.0f : 0.0f));
    }
  }
};
