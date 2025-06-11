struct CueResearchLoadSample : MenuItem
{
    CueResearch *module;

    void onAction(const event::Action &e) override
    {
#ifdef USING_CARDINAL_NOT_RACK
        CueResearch *module = this->module;
        async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load sample", [module](char *filename)
                                 {
        if(filename)
        {
          fileSelected(module, std::string(filename));
          free(filename);
        } });
#else
        fileSelected(module, module->selectFileVCV());
#endif
    }

    static void fileSelected(CueResearch *module, std::string filename)
    {
        if (filename != "")
        {
            module->sample.load(filename);
            module->loaded_filename = module->sample.getFilename();
            module->setRoot(filename);
            if (module->clear_markers_on_sample_load)
            {
                module->clearMarkers();
            }
            // Notify track model and waveform model of sample change
            module->track_model.onSampleChanged();
            // module->waveform_model.onSampleChanged();
        }
    }
};
