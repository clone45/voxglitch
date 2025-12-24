#pragma once

struct MenuItemLoadWords : MenuItem
{
	Maya *module;

	void onAction(const event::Action &e) override
	{
#ifdef USING_CARDINAL_NOT_RACK
		Maya *module = this->module;
		async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load words folder", [module](char *path) {
			if (path)
			{
				if (char *rpath = strrchr(path, CARDINAL_OS_SEP))
					*rpath = '\0';
				pathSelected(module, path);
				free(path);
			}
		});
#else
		pathSelected(module, module->selectPathVCV());
#endif
	}

	static void pathSelected(Maya *module, std::string path)
	{
		if (path != "")
		{
			module->load_samples_from_path(path);
			module->path = path;
			module->setRoot(path);
		}
	}
};
