struct ScatterLoadSample : MenuItem
{
	Scatter *module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));

		if (path)
		{
			module->sample_players[sample_number].loadSample(std::string(path));
			module->root_dir = std::string(path);
			// module->loaded_filenames[sample_number] = module->samples[sample_number].filename;
      module->loaded_filenames[sample_number] = module->sample_players[sample_number].getFilename();
			free(path);
		}
	}
};
