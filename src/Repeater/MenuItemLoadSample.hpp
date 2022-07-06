struct MenuItemLoadSample : MenuItem
{
	Repeater *module;
	unsigned int sample_number = 0;

  void onAction(const event::Action &e) override
	{
#ifdef USING_CARDINAL_NOT_RACK
		async_dialog_filebrowser(false, module->samples_root_dir.c_str(), NULL, [module, sample_number](char* filename) {
      if(filename)
      {
  			fileSelected(module, sample_number, std::string(filename));
        free filename;
      }
		});
#else
		fileSelected(module, this->sample_number, module->selectFileVCV());
#endif
	}

  static void fileSelected(Repeater *module, unsigned int sample_number, std::string filename)
	{
		if (filename != "")
		{
			module->any_sample_has_been_loaded = true;
			module->samples[sample_number].load(filename);
			module->loaded_filenames[sample_number] = module->samples[sample_number].filename;
      module->setRoot(filename);
		}
	}
};
