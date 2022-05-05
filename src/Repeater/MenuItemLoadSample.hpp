struct MenuItemLoadSample : MenuItem
{
	Repeater *module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
#ifdef USING_CARDINAL_NOT_RACK
		Repeater *module = this->module;
		unsigned int sample_number = this->sample_number;
		async_dialog_filebrowser(false, dir.c_str(), text.c_str(), [module, sample_number](char* path) {
			pathSelected(module, sample_number, path);
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));
		pathSelected(module, sample_number, path);
#endif
	}

	static void pathSelected(Repeater *module, unsigned int sample_number, char *path)
	{
		if (path)
		{
			module->any_sample_has_been_loaded = true;
			module->samples[sample_number].load(path);
			module->root_dir = std::string(path);
			module->loaded_filenames[sample_number] = module->samples[sample_number].filename;
			free(path);
		}
	}
};
