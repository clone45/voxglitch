struct GhostsLoadSample : MenuItem
{
	Ghosts *module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = "";
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if(path)
		{
			module->sample.load(path, false);
			module->root_dir = std::string(path);
			module->loaded_filename = module->sample.filename;
			free(path);
		}
	}
};
