struct FourTrackLoadSample : MenuItem
{
    FourTrack *module;

    void onAction(const event::Action &e) override
    {
#ifdef USING_CARDINAL_NOT_RACK
        FourTrack *module = this->module;
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

    static void fileSelected(FourTrack *module, std::string filename)
    {
        if (filename != "")
        {
            module->sample.load(filename);
            module->loaded_filename = module->sample.getFilename();
            module->setRoot(filename);

            module->clearMarkers();
        }
    }
};
