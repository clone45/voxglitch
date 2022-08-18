//
//
//
//  Note: Using a singleton for the theme won't work without a bit more
//  thinking, because the widgets are associated with different modules
//
//
//
//

struct Theme
{
  std::string name = "default";
  json_t *json_root;
  json_t *widgets;
  bool loaded = false;

  static Theme& getInstance()
  {
    static Theme theme_instance;
    return theme_instance;
  }

  // Singleton structure and comments by Andy Prowl
  // https://stackoverflow.com/a/15187314/690143

  // The copy constructor is deleted, to prevent client code from creating new
  // instances of this class by copying the instance returned by get_instance()
  Theme(Theme const&) = delete;

  // The move constructor is deleted, to prevent client code from moving from
  // the object returned by get_instance(), which could result in other clients
  // retrieving a reference to an object with unspecified state.
  Theme(Theme&&) = delete;

  bool load(std::string slug)
  {
    if(! loaded)
    {

      // Load up theme name and other theme-specific stuff
      this->loadConfig();

      // Load up actual theme data based on theme slug (default, dark, etc.)
      json_error_t error;
      std::string config_file_path = asset::plugin(pluginInstance, "res/" + slug + "/themes/" + name + "/config.json");

      // Load theme selection
      json_root = json_load_file(config_file_path.c_str(), 0, &error);

      if(! json_root)
      {
        // DEBUG("unable to load JSON file");
        // DEBUG(config_file_path.c_str());
        return(false);
      }

      // Store this for quick access for later
      widgets = json_object_get(json_root, "widgets");
    }

    loaded = true;

    return(true);
  }

  void loadConfig()
  {
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
        name = json_string_value(theme_json);
      }
    }
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
    json_t* value_json = json_object_get(json_root, key.c_str());
    if (value_json) value_float = json_real_value(value_json);
    return(value_float);
  }

private:

  // Default-constructor is private, to prevent client code from creating new
  // instances of this class. The only instance shall be retrieved through the
  // get_instance() function.

  Theme() { }

};
