struct GrainEngineLoadSample : MenuItem
{
	GrainEngine *module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = "";
#ifdef USING_CARDINAL_NOT_RACK
		GrainEngine *module = this->module;
		async_dialog_filebrowser(false, NULL, dir.c_str(), text.c_str(), [module](char* path) {
			pathSelected(module, path);
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));
    // char *path = module->selectFileVCV(dir);
		pathSelected(module, path);
#endif
	}

	static void pathSelected(GrainEngine *module, char *path)
	{
		if (path)
		{
			module->sample.load(path);
			module->root_dir = std::string(path);
			module->loaded_filename = module->sample.filename;
			free(path);
		}
	}
};
