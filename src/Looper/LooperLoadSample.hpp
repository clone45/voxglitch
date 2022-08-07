struct LooperLoadSample : MenuItem
{
	Looper *module;

	void onAction(const event::Action &e) override
	{
    #ifdef USING_CARDINAL_NOT_RACK
		Looper *module = this->module;
    	async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load sample", [module](char* filename) {
        if(filename)
        {
          fileSelected(module, std::string(filename));
          free(filename);
        }
    	});
    #else
      fileSelected(module, module->selectFileVCV());
    #endif
	}

	static void fileSelected(Looper *module, std::string filename)
	{
    if (filename != "")
		{
			module->sample_player.loadSample(filename);
      module->sample_player.trigger();
			module->loaded_filename = module->sample_player.getFilename();
			module->setRoot(filename);
		}
	}
};
