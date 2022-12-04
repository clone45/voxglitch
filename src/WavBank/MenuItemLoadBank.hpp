struct MenuItemLoadBank : MenuItem
{
  WavBank *module;

  void onAction(const event::Action &e) override
  {
    #ifdef USING_CARDINAL_NOT_RACK
        WavBank *module = this->module;
        async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load sample", [module](char *path)
                                {
          if (path) {
              if (char *rpath = strrchr(path, CARDINAL_OS_SEP))
                  *rpath = '\0';
              pathSelected(module, std::string(path));
              free(path);
          } });
    #else
        pathSelected(module, module->selectPathVCV());
    #endif
  }

  static void pathSelected(WavBank *module, std::string path)
  {
    if (path != "")
    {
      module->load_samples_from_path(path);
      module->path = path;
      module->setRoot(path);
    }
  }
};
