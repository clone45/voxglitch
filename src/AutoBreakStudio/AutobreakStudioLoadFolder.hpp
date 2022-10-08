struct AutobreakStudioLoadFolder : MenuItem
{
    AutobreakStudio *module;
    std::string root_dir;

    void onAction(const event::Action &e) override
    {
        const std::string dir = root_dir.empty() ? "" : root_dir;
#ifdef USING_CARDINAL_NOT_RACK
        AutobreakStudio *module = this->module;
        async_dialog_filebrowser(false, NULL, dir.c_str(), text.c_str(), [module](char *path)
        {
			if (path) 
            {
				if (char *rpath = strrchr(path, CARDINAL_OS_SEP)) *rpath = '\0';
				pathSelected(module, path);
			} 
        });
#else
        char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);
        pathSelected(module, path);
#endif
    }

    static void pathSelected(AutobreakStudio *module, char *path)
    {
        if (path)
        {
            std::vector<std::string> dirList = system::getEntries(path);
            unsigned int i = 0;

            for (auto filename : dirList)
            {
                if (
                    // Something happened in Rack 2 where the extension started to include
                    // the ".", so I decided to check for both versions, just in case.
                    (rack::string::lowercase(system::getExtension(filename)) == "wav") ||
                    (rack::string::lowercase(system::getExtension(filename)) == ".wav"))
                {
                    if (i < 8)
                    {
                        module->samples[i].load(filename);
                        module->loaded_filenames[i] = module->samples[i].filename;
                        module->setRoot(filename);
                        i++;
                    }
                }
            }

            free(path);
        }
    }
};
