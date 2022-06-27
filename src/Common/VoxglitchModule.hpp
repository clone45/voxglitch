/*

//
//
// This gets more complicated than I want to consider right now
//
//

struct VoxglitchModule : Module
{
  std::string version = "";
  bool show_release_notes = false;

  void readVoxglitchSettings()
  {
    json_error_t error;

    json_t *json_root = json_load_file(asset::user("voxglitch.json").c_str(), 0, &error);
    if(json_root)
    {
      json_t *version_json = json_object_get(json_root, "version");
      this->version = json_string_value(version_json);

      if(this->version != )
      {
        show_release_notes = true;

        // Also write new version to voxglitch.json
      }
    }
    else
    {
      createVoxglitchSettings();
    }
  }

  void createVoxglitchSettings()
  {
    // Save current version
    json_t *json_root = json_object();
    json_object_set(track_json_object, "sample_filename", json_string(filename.c_str()));
  }

};
*/
