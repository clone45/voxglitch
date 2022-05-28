struct LooperLoadSample : MenuItem
{
	Looper *module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
#ifdef USING_CARDINAL_NOT_RACK
		Looper *module = this->module;
		async_dialog_filebrowser(false, dir.c_str(), module->loaded_filename.c_str(), [module](char* path) {
			pathSelected(module, path);
		});
#else
		// char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));
    char *path = module->selectFileVCV(dir);
		pathSelected(module, path);
#endif
	}

	static void pathSelected(Looper *module, char *path)
	{
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
