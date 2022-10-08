struct Theme
{
  std::string name = "dark";
  json_t *json_root = NULL;
  json_t *widgets = NULL;
  bool show_screws = true;

  Theme()
  {
    json_t *json_root_local;
    json_error_t error;

    if(! rack::system::exists(asset::user("Voxglitch.json")))
    {
      rack::system::copy(asset::plugin(pluginInstance, "res/voxglitch_config.json"), asset::user("Voxglitch.json"));
    }

    std::string config_file_path = asset::user("Voxglitch.json");

    // Load theme selection, either "light" or "dark"
    json_root_local = json_load_file(config_file_path.c_str(), 0, &error);
    if(json_root_local)
    {
      json_t* theme_json = json_object_get(json_root_local, "theme");
      if (theme_json) name = json_string_value(theme_json);
      json_decref(json_root_local);
    }
  }

  ~Theme()
  {
    if (json_root)
      json_decref(json_root);
  }

  bool load(std::string slug)
  {
    json_error_t error;

    std::string config_file_path = asset::plugin(pluginInstance, "res/" + slug + "/themes/" + name + "/config.json");

    // Load theme selection
    json_root = json_load_file(config_file_path.c_str(), 0, &error);

    if(! json_root)
    {
      //DEBUG("unable to load JSON file");
      // DEBUG(config_file_path.c_str());
      return(false);
    }

    // Store this for quick access for later
    widgets = json_object_get(json_root, "widgets");

    // Optionally show or hide screws.  Screws are shown by default.
    json_t* show_screws_json = json_object_get(json_root, "show_screws");
    if (show_screws_json) show_screws = json_boolean_value(show_screws_json);

    return(true);
  }

  json_t* getLayers()
  {
    json_t* array_json = json_object_get(json_root, "layers");
    return(array_json);
  }

  json_t *getWidgets()
  {
    json_t* array_json = json_object_get(json_root, "widgets");
    return(array_json);
  }

  std::string getString(std::string key)
  {
    std::string value_string = "";

    json_t* value_json = json_object_get(json_root, key.c_str());
    if (value_json)
    {
      value_string = json_string_value(value_json);
    }
    else
    {
      // DEBUG("could not find key");
      // DEBUG(key.c_str());
    }

    return(value_string);
  }

  float getFloat(std::string key)
  {
    float value_float = 0.0;
    if(json_root)
    {
      json_t* value_json = json_object_get(json_root, key.c_str());
      if (value_json) value_float = json_real_value(value_json);
    }
    return(value_float);
  }

  bool showScrews()
  {
    return(this->show_screws);
  }

};
