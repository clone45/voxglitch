//
// DualSlope

struct DualSlope : Module
{
  enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DualSlope()
	{
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

  //
	// Autosave module data.  VCV Rack decides when this should be called.

	json_t *dataToJson() override
	{
    json_t *json_root = json_object();
  	return json_root;
	}

	// Load module data
	void dataFromJson(json_t *json_root) override
	{
	}

	void process(const ProcessArgs &args) override
	{

  }
};
