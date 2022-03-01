struct FolderSelect : MenuItem
{
	Scalar110 *module;

	void onAction(const event::Action &e) override
	{
		const std::string dir = module->root_directory.empty() ? "" : module->root_directory;
    char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);

		if (path)
		{
      SampleBank& sample_bank = SampleBank::get_instance();
      sample_bank.loadSamplesFromPath(path);
		}

    free(path);
	}

};
