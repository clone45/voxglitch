struct VoxglitchSamplerModule : Module
{
  unsigned int interpolation = 1;
  float sample_rate = 44100;

  void saveSamplerData(json_t *root)
  {
    json_object_set_new(root, "interpolation", json_integer(interpolation));
  }

  void loadSamplerData(json_t *root)
  {
    json_t *interpolation_json = json_object_get(root, ("interpolation"));
    if (interpolation_json) interpolation = json_integer_value(interpolation_json);
  }

  char *selectFileVCV(std::string dir)
  {
    osdialog_filters* filters = osdialog_filters_parse("WAV:wav");
    char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, filters);
    osdialog_filters_free(filters);
    return path;
  }

  // pathSelected(module, track_number, path);

};
