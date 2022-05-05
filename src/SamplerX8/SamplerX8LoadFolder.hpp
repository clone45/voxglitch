struct SamplerX8LoadFolder : MenuItem
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
		async_dialog_filebrowser(false, dir.c_str(), text.c_str(), [module, sample_number](char *path) {
			if (path) {
				if (char *rpath = strrchr(path, CARDINAL_OS_SEP))
					*rpath = '\0';
				pathSelected(module, sample_number, path);
			}
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);
		pathSelected(module, sample_number, path);
#endif
	}

	static void pathSelected(SamplerX8 *module, unsigned int sample_number, char *path)
	{
		if (path)
		{
			std::vector<std::string> dirList = system::getEntries(path);
			unsigned int i = 0;

			for (auto entry : dirList)
			{
				if (
					// Something happened in Rack 2 where the extension started to include
					// the ".", so I decided to check for both versions, just in case.
					(rack::string::lowercase(system::getExtension(entry)) == "wav") ||
					(rack::string::lowercase(system::getExtension(entry)) == ".wav")
				   )
				{
					if (i < 8)
					{
						module->sample_players[i].loadSample(std::string(entry));
						module->loaded_filenames[i] = module->sample_players[i].getFilename();
						i++;
					}
				}
			}

			free(path);
		}
	}
};
