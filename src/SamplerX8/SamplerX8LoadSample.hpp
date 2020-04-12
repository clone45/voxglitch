struct SamplerX8LoadSample : MenuItem
{
	SamplerX8 *module;
	unsigned int sample_number = 0;
  std::string root_dir;

	void onAction(const event::Action &e) override
	{
		const std::string dir = root_dir.empty() ? "" : root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
      root_dir = std::string(path);
			module->sample_players[sample_number].loadSample(std::string(path));
      module->loaded_filenames[sample_number] = module->sample_players[sample_number].getFilename();
			free(path);
		}
	}
};
