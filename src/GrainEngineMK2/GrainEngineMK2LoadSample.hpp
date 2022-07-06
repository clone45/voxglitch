struct GrainEngineMK2LoadSample : MenuItem
{
	GrainEngineMK2 *module;
	unsigned int sample_number = 0;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_dir.empty() ? "" : module->root_dir;
#ifdef USING_CARDINAL_NOT_RACK
		async_dialog_filebrowser(false, dir.c_str(), NULL, [module, sample_number](char* filename) {
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

	static void fileSelected(GrainEngineMK2 *module, unsigned int sample_number, std::string filename)
	{
		if (filename != "")
		{
			module->load_queue.queue_sample_for_loading(filename, sample_number);
			module->fade_out_on_load.trigger();
		}
	}
};
