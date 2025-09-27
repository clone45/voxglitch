struct MenuItemLoadConfig : MenuItem
{
  Netrunner *module;

  void onAction(const event::Action &e) override
  {
    #ifdef USING_CARDINAL_NOT_RACK
        Netrunner *module = this->module;
        async_dialog_filebrowser(false, NULL, NULL, "Load Config", [module](char *path) {
          if (path) {
            pathSelected(module, path);
            free(path);
          }
        });
    #else
        std::string path = module->selectFileVCV();
        pathSelected(module, path);
    #endif
  }

  static void pathSelected(Netrunner *module, std::string path)
  {
    if (path != "")
    {
      module->load_samples_from_config(path);
      module->path = path;
    }
  }
};