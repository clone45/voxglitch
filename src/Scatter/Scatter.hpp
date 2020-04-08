struct Scatter : Module
{
	Scatter()
	{

	}

	// Autosave module data.  VCV Rack decides when this should be called.
	json_t *dataToJson() override
	{
		json_t *root = json_object();

		return root;
	}

	// Load module data
	void dataFromJson(json_t *root) override
	{

	}

	void process(const ProcessArgs &args) override
	{
  }
};
