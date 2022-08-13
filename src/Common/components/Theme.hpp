struct Layer
{
  std::string type;
  std::string value;

  Layer(std::string type, std::string value)
  {
    this->type = type;
    this->value = value;
  }
};

struct Theme
{
  std::string name = "dark";
  json_t *json_root;
  std::vector<Layer*> layers;

  Theme()
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

  bool load(std::string slug)
  {
    json_error_t error;

    std::string config_file_path = asset::plugin(pluginInstance, "res/" + slug + "/themes/" + name + "/config.json");

    // Load theme selection
    json_root = json_load_file(config_file_path.c_str(), 0, &error);

    if(! json_root)
    {
      DEBUG("unable to load JSON file");
      DEBUG(config_file_path.c_str());
    }

    if(json_root)
    {

      //
      // Load layers into an array that can be consumed by the module
      //

      DEBUG("loading layers from Theme.hpp");

      /*

      Example JSON:

      {
        "panel_svg": "res/digital_sequencer/themes/default/panel.svg",
        "layers": {
          "png": "res/digital_sequencer/themes/default/baseplate.png" },
          "svg": "res/digital_sequencer/themes/default/typography.svg" }
        }
      }

      */

      /*
      json_t* layers_object = json_object_get(json_root, "layers");
      if (layers_object)
      {
        // size_t layer_number;
        // json_t *json_layer_array;

        DEBUG("got here! wof");

        const char *key;
        json_t *json_value;

        json_object_foreach(layers_object, key, json_value) {

          std::string layer_value = json_string_value(json_value);
          std::string layer_type(key);

          layers.push_back(new Layer(layer_type, layer_value));
        }
      }

      DEBUG("setup complete");
      */

      /*
      json_array_foreach(recording_memory_data, i, json_array_pair_xy)
      {
        float x = json_real_value(json_array_get(json_array_pair_xy, 0));
        float y = json_real_value(json_array_get(json_array_pair_xy, 1));
        recording_memory.push_back(Vec(x,y));
      }
      */


      return(true);
    }

    return(false);
  }

  json_t* getLayers()
  {
    json_t* array_json = json_object_get(json_root, "layers");
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
/*
  float getLayers()
  {

    return(???); // some iterator??
  }
  */
};
