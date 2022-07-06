struct GhostsLoadSample : MenuItem
{
	Ghosts *module;

  void onAction(const event::Action &e) override
	{
    #ifdef USING_CARDINAL_NOT_RACK
    	async_dialog_filebrowser(false, module->samples_root_dir.c_str(), NULL, [module](char* filename) {
        if(filename)
        {
          fileSelected(module, std::string(filename));
          free filename;
        }
    	});
    #else
      fileSelected(module, module->selectFileVCV());
    #endif
	}

	static void fileSelected(Ghosts *module, std::string filename)
	{
    if (filename != "")
		{
			module->sample.load(filename);
			module->loaded_filename = module->sample.filename;
			module->setRoot(filename);
		}
	}

  /*
	void onAction(const event::Action &e) override
	{
		const std::string dir = "";
#ifdef USING_CARDINAL_NOT_RACK
		Ghosts *module = this->module;
		async_dialog_filebrowser(false, dir.c_str(), text.c_str(), [module](char* path) {
			pathSelected(module, path);
		});
#else
		// char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("Wav:wav"));
    char *path = module->selectFileVCV(dir);
		pathSelected(module, path);
#endif

	}

	static void pathSelected(Ghosts *module, char *path)
	{

		if (path)
		{
			module->sample.load(path);
			module->root_dir = std::string(path);
			module->loaded_filename = module->sample.filename;
			free(path);
		}
	}
  */
};
