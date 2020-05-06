struct PanelArt : TransparentWidget
{
	GrainBlender *module;
	std::shared_ptr<Font> font;
  unsigned int internal_activity_indicator = 20;

	PanelArt()
	{
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		if(module)
		{
      /*
			if(module->is_spawn_cable_connected == false)
			{
        internal_activity_indicator += 2;
        if(internal_activity_indicator >= 200) internal_activity_indicator = 0;

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, 184.00, 77.00, 13.0 + (float)internal_activity_indicator / 50.0);
        nvgFillColor(args.vg, nvgRGBA(200, 200, 200, 200));
        nvgFill(args.vg);
			}
      */
		}

		nvgRestore(args.vg);
	}

};
