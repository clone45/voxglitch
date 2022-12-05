struct KodamaLoadSample : MenuItem
{
    Kodama *module;

    void onAction(const event::Action &e) override
    {
#ifdef USING_CARDINAL_NOT_RACK
        SamplerX8 *module = this->module;
        unsigned int sample_number = this->sample_number;
        async_dialog_filebrowser(false, NULL, NULL, "Load sample", [module, sample_number](char *filename)
                                 {
          if(filename)
          {
            fileSelected(module, sample_number, std::string(filename));
            free(filename);
          } });
#else
        fileSelected(module, module->selectFileVCV());
#endif
    }

    static void fileSelected(Kodama *module, std::string filename)
    {
        if (filename != "")
        {
            for(unsigned int i=0; i<NUMBER_OF_SLIDERS; i++)
            {
                if (module->sample_players[i].loadSample(filename))
                {
                    // module->loaded_filenames[i] = module->sample_players[i].getFilename();
                    module->setRoot(filename);
                    module->sample_players[i].trigger();
                }
            }

        }
    }
};