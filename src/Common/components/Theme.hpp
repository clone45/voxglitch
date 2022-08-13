struct Theme
{
  std::string name = "dark";
  json_t *json_root;

  Theme()
  {

  }

  bool load(std::string slug)
  {
    json_error_t error;

    std::string config_file_path = asset::plugin(pluginInstance, "res/" + slug + "/themes/" + name + "/config.json");

    DEBUG("config_file_path is");
    DEBUG(config_file_path.c_str());

    // Load theme selection, either "light" or "dark"
    json_root = json_load_file(config_file_path.c_str(), 0, &error);
    if(json_root) return(true);

    return(false);
  }

  std::string getString(std::string key)
  {
    std::string value_string = "";

    json_t* value_json = json_object_get(json_root, key.c_str());
    if (value_json)
    {
      value_string = json_string_value(value_json);
      DEBUG("found key OK");
      DEBUG(key.c_str());
    }
    else
    {
      DEBUG("could not find key");
      DEBUG(key.c_str());
    }

    return(value_string);
  }

  float getFloat(std::string key)
  {
    float value_float = 0.0;
    json_t* value_json = json_object_get(json_root, key.c_str());
    if (value_json) value_float = json_real_value(value_json);
    return(value_float);
  }
};
