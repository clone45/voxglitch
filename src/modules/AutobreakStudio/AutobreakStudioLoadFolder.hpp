struct AutobreakStudioLoadFolder : MenuItem
{
    AutobreakStudio *module;
    std::string root_dir;

    void onAction(const event::Action &e) override
    {
        const std::string dir = root_dir.empty() ? "" : root_dir;
#ifdef USING_CARDINAL_NOT_RACK
        AutobreakStudio *module = this->module;
        async_dialog_filebrowser(false, NULL, dir.c_str(), text.c_str(), [module](char *path) {
            if (path) {
                if (char *rpath = strrchr(path, CARDINAL_OS_SEP))
                    *rpath = '\0';
                pathSelected(module, path);
                free(path);
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

            // Now supports WAV, AIFF, FLAC, MP3, and ALAC formats via FFmpeg
            const std::vector<std::string> supportedExtensions = {
                "wav", ".wav",
                "aiff", ".aiff", "aif", ".aif",
                "flac", ".flac",
                "mp3", ".mp3",
                "m4a", ".m4a", "alac", ".alac"
            };

            for (auto filename : dirList)
            {
                std::string ext = rack::string::lowercase(system::getExtension(filename));

                // Check if extension is in supported list
                bool isSupported = std::find(
                    supportedExtensions.begin(),
                    supportedExtensions.end(),
                    ext
                ) != supportedExtensions.end();

                if (isSupported)
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
