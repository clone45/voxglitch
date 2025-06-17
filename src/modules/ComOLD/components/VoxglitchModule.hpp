struct VoxglitchModule : Module
{
  // std::string theme_name = "dark";

  template<typename T=rack::SwitchQuantity>
  
  T *configOnOff(int paramId, float defaultValue, std::string name)
  {
      return configSwitch<T>(paramId, 0, 1, defaultValue, name, {"Off", "On"});
  }

  VoxglitchModule()
  {
    //
    // Load config file
    //
    // TODO: Move this into a singleton class to cut down on file access because
    // right now, each module runs this code
    //

    /*
    json_t *json_root;
    json_error_t error;

    std::string config_file_path = asset::plugin(pluginInstance, "res/voxglitch_config.json");

    // Load theme selection, either "light" or "dark"
    json_root = json_load_file(config_file_path.c_str(), 0, &error);
    if(json_root)
    {
      json_t* theme_json = json_object_get(json_root, "theme");
      if (theme_json)
      {
        theme.name = json_string_value(theme_json);
      }
    }
    */
  }

};
