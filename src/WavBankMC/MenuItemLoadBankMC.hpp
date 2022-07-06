struct MenuItemLoadBankMC : MenuItem
{
	WavBankMC *module;

	void onAction(const event::Action &e) override
	{
    #ifdef USING_CARDINAL_NOT_RACK
    		async_dialog_filebrowser(false, dir.c_str(), text.c_str(), [module](char* path) {
    			if (path) {
    				if (char *rpath = strrchr(path, CARDINAL_OS_SEP))
    					*rpath = '\0';
    				pathSelected(module, path);
    			}
    		});
    #else
    		pathSelected(module, module->selectPathVCV());
    #endif
	}

	static void pathSelected(WavBankMC *module, std::string path)
	{
	if (path != "")
		{
			module->load_samples_from_path(path);
			module->path = path;
			module->selected_sample_slot = 0;
      module->setRoot(path);
		}
	}
};
