struct Sampler16PLoadFolder : MenuItem
{
	Sampler16P *module;
	unsigned int sample_number = 0;
	std::string root_dir;

	void onAction(const event::Action &e) override
	{
		const std::string dir = root_dir.empty() ? "" : root_dir;
#ifdef USING_CARDINAL_NOT_RACK
		Sampler16P *module = this->module;
		unsigned int sample_number = this->sample_number;
		async_dialog_filebrowser(false, NULL, dir.c_str(), text.c_str(), [module, sample_number](char *path) {
			if (path) {
				if (char *rpath = strrchr(path, CARDINAL_OS_SEP))
					*rpath = '\0';
				pathSelected(module, sample_number, path);
				free(path);
			}
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);
		pathSelected(module, sample_number, path);
#endif
	}

	static void pathSelected(Sampler16P *module, unsigned int sample_number, char *path)
	{
		if (path)
		{
			std::vector<std::string> dirList = system::getEntries(path);
			unsigned int i = 0;

			// Now supports WAV, AIFF, FLAC, MP3, and ALAC formats via FFmpeg
			const std::vector<std::string> supportedExtensions = {
				"wav", ".wav",
				"aiff", ".aiff", "aif", ".aif",
				"flac", ".flac",
				"mp3", ".mp3",
				"m4a", ".m4a", "alac", ".alac"
			};

			for (auto entry : dirList)
			{
				std::string ext = rack::string::lowercase(system::getExtension(entry));

				// Check if extension is in supported list
				bool isSupported = std::find(
					supportedExtensions.begin(),
					supportedExtensions.end(),
					ext
				) != supportedExtensions.end();

				if (isSupported)
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
