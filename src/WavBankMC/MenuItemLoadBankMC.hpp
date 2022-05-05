struct MenuItemLoadBankMC : MenuItem
{
	WavBankMC *wav_bank_mc_module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = wav_bank_mc_module->rootDir;
#ifdef USING_CARDINAL_NOT_RACK
		WavBankMC *wav_bank_mc_module = this->wav_bank_mc_module;
		async_dialog_filebrowser(false, dir.c_str(), text.c_str(), [wav_bank_mc_module](char* path) {
			if (path) {
				if (char *rpath = strrchr(path, CARDINAL_OS_SEP))
					*rpath = '\0';
				pathSelected(wav_bank_mc_module, path);
			}
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);
		pathSelected(wav_bank_mc_module, path);
#endif
	}

	static void pathSelected(WavBankMC *wav_bank_mc_module, char *path)
	{
		if (path)
		{
			wav_bank_mc_module->load_samples_from_path(path);
			wav_bank_mc_module->path = path;
			wav_bank_mc_module->selected_sample_slot = 0;
			free(path);
		}
	}
};
