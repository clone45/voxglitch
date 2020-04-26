struct PanelArt : TransparentWidget
{
	GrainBlender *module;
	std::shared_ptr<Font> font;
  unsigned int warning_fade = 20;

	PanelArt()
	{
	}

	void draw(const DrawArgs &args) override
	{
		nvgSave(args.vg);

		if(module)
		{
			if(module->is_spawn_cable_connected == false)
			{
        warning_fade += 2;
        if(warning_fade >= 200) warning_fade = 0;

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, 184.00, 77.00, 15);
        nvgFillColor(args.vg, nvgRGBA(255, 30, 30, warning_fade));
        nvgFill(args.vg);
			}
		}

		nvgRestore(args.vg);
	}

};
