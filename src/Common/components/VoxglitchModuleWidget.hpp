
struct VoxglitchModuleWidget : ModuleWidget
{
  Theme theme;
  json_t *widgets_json;

  void addSVGLayer(std::string svg_path)
  {
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, svg_path));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);
  }

  Vec themePos(std::string widget_name)
  {
    json_t* widget_object = json_object_get(widgets_json, widget_name.c_str());
    json_t* x_object = json_object_get(widget_object, "x");
    json_t* y_object = json_object_get(widget_object, "y");
    return(Vec(json_real_value(x_object), json_real_value(y_object)));
  }

  void applyTheme()
  {
    json_t* layers_array = theme.getLayers();

    if (layers_array)
    {
      size_t index;
      json_t *value;

      json_array_foreach(layers_array, index, value)
      {
        //
        // Get the "type" from the object
        json_t *json_type = json_object_get(value, "type");
        if(json_type)
        {
          // Convert the "type" to a string
          std::string layer_type = json_string_value(json_type);

          // The type should be either svg, png, or rect.  Depending on the
          // type, the other variables might differ, so we need to process
          // each thing depending on the type.

          if(layer_type == "svg")
          {
            // For the svg type, there's only one other property, and that's
            // the path to the svg.  Get that path, convert it to a string,
            // and use it to load the svg and add it to the panel.
            std::string svg_path = json_string_value(json_object_get(value, "path"));
            addSVGLayer(svg_path);
          }

          if(layer_type == "image" || layer_type == "png")
          {
            std::string path = json_string_value(json_object_get(value, "path"));
            float module_width = json_real_value(json_object_get(value, "width"));
            float module_height = json_real_value(json_object_get(value, "height"));
            float zoom = json_real_value(json_object_get(value, "zoom"));

            float src_panel_offset_x = 0.0;
            float src_panel_offset_y = 0.0;

            PanelBackground *panel_background = new PanelBackground(path, module_width, module_height, src_panel_offset_x, src_panel_offset_y, zoom);
            addChild(panel_background);
          }
        }
      }
    }

    // Set the SVG.  This must always be called panel_svg in the theme JSON
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, theme.getString("panel_svg")))); // Set panel SVG

    // Precompile widgets_json and store it in this structure
    this->widgets_json = theme.getWidgets();
  }

#ifdef DEV_MODE
  void onHoverKey(const event::HoverKey &e) override
  {
    if(e.action == GLFW_PRESS && e.key == GLFW_KEY_P)
    {
      std::string debug_string = "mouse at: " + std::to_string(e.pos.x) + "," + std::to_string(e.pos.y);
      DEBUG(debug_string.c_str());
    }
    ModuleWidget::onHoverKey(e);
  }
#endif

};
