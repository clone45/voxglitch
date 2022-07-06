struct VoxglitchSamplerModule : Module
{
  unsigned int interpolation = 1;
  float sample_rate = 44100;
  std::string samples_root_dir = "";

  void saveSamplerData(json_t *root)
  {
    json_object_set_new(root, "interpolation", json_integer(interpolation));
    json_object_set_new(root, "samples_root_dir", json_string(samples_root_dir.c_str()));
  }

  void loadSamplerData(json_t *root)
  {
    // Load Interpolation settings
    json_t *interpolation_json = json_object_get(root, ("interpolation"));
    if (interpolation_json) interpolation = json_integer_value(interpolation_json);

    // Load root directory
    json_t *samples_root_dir_json = json_object_get(root, ("samples_root_dir"));
    if (samples_root_dir_json) samples_root_dir = json_string_value(samples_root_dir_json);
  }

  // Pure virtual function that must be implemented in the derived classes
  // virtual void fileSelected(std::string filename) = 0;

  std::string selectFileVCV()
  {
    std::string filename_string = "";
    osdialog_filters* filters = osdialog_filters_parse("WAV:wav");
    char *filename = osdialog_file(OSDIALOG_OPEN, samples_root_dir.c_str(), NULL, filters);

    if(filename != NULL)
    {
      filename_string.assign(filename);
      osdialog_filters_free(filters);
      std::free(filename);
    }

    return(filename_string);
  }

  std::string selectPathVCV()
  {
    std::string path_string = "";
    char *path = osdialog_file(OSDIALOG_OPEN_DIR, samples_root_dir.c_str(), NULL, NULL);

    if(path != NULL)
    {
      path_string.assign(path);
      std::free(path);
    }

    return(path_string);

    /*
    osdialog_filters* filters = osdialog_filters_parse("WAV:wav");
    char *filename = osdialog_file(OSDIALOG_OPEN, samples_root_dir.c_str(), NULL, filters);
    osdialog_filters_free(filters);
    std::string filename_string(filename);
    return(filename_string);
    */
  }

  void setSamplesRootDirectory(std::string samples_root_directory)
  {
    this->samples_root_dir = samples_root_directory;
  }

  void setRoot(std::string filename)
  {
    this->setSamplesRootDirectory(extractDirectoryFromFilename(filename));
  }

  // Helper function that returns the directory given a full path
  std::string extractDirectoryFromFilename(const std::string& filename)
  {
    size_t pos = filename.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : filename.substr(0, pos);
  }
};
