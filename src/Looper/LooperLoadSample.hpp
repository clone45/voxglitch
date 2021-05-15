struct LooperLoadSample : MenuItem
{
	Looper *module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
      module->root_dir = std::string(path);
			module->sample_player.loadSample(std::string(path));
      module->loaded_filename = module->sample_player.getFilename();
			free(path);

      // module->sample_player.loadSample(std::string(path));
		}
	}
};
