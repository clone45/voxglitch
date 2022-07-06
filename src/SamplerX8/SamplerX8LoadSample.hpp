struct SamplerX8LoadSample : MenuItem
{
	SamplerX8 *module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
    #ifdef USING_CARDINAL_NOT_RACK
    		async_dialog_filebrowser(false, module->sample_root_dir.c_str(), NULL, [module, sample_number](char* filename) {
          if(filename)
          {
            fileSelected(module, sample_number, std::string(filename));
            free filename;
          }
    		});
    #else
    		fileSelected(module, this->sample_number, module->selectFileVCV());
    #endif

    DEBUG("onAction completed");
	}

  static void fileSelected(SamplerX8 *module, unsigned int sample_number, std::string filename)
	{
		if (filename != "")
		{
      if(module->sample_players[sample_number].loadSample(filename))
      {
  			module->loaded_filenames[sample_number] = module->sample_players[sample_number].getFilename();
        module->setRoot(filename);
      }
		}
    DEBUG("Finished file selected");
	}
};
