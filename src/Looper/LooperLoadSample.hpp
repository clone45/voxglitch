struct LooperLoadSample : MenuItem
{
	Looper *module;
  std::string root_dir;

	void onAction(const event::Action &e) override
	{
		const std::string dir = root_dir.empty() ? "" : root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
      root_dir = std::string(path);
			module->sample_player.loadSample(std::string(path));
      module->loaded_filename = module->sample_player.getFilename();
			free(path);

      module->sample_player.trigger();
		}
	}
};
