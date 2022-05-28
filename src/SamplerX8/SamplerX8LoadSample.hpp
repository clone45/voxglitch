struct SamplerX8LoadSample : MenuItem
{
	SamplerX8 *module;
	unsigned int sample_number = 0;
	std::string root_dir;

	void onAction(const event::Action &e) override
	{
		const std::string dir = root_dir.empty() ? "" : root_dir;
#ifdef USING_CARDINAL_NOT_RACK
		SamplerX8 *module = this->module;
		unsigned int sample_number = this->sample_number;
		async_dialog_filebrowser(false, dir.c_str(), text.c_str(), [module, sample_number](char* path) {
			pathSelected(module, sample_number, path);
		});
#else
		// char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));
    char *path = module->selectFileVCV(dir);
		pathSelected(module, sample_number, path);
#endif
	}

	static void pathSelected(SamplerX8 *module, unsigned int sample_number, char *path)
	{
		if (path)
		{
			module->sample_players[sample_number].loadSample(std::string(path));
			module->loaded_filenames[sample_number] = module->sample_players[sample_number].getFilename();
			free(path);
		}
	}
};
