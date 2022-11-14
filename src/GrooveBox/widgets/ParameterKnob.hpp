struct ParameterKnob : SvgKnob
{
  widget::SvgWidget *bg;
  GrooveBox *module;
  unsigned int parameter_index = 0;
  unsigned int step = 0;

  ParameterKnob()
  {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;

    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_TrimpotMedium.svg")));
    bg = new widget::SvgWidget;
    bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_TrimpotMedium_bg.svg")));
    fb->addChildBelow(bg, tw);
  }

  void onDoubleClick(const DoubleClickEvent &e) override
  {
    float value = 0.0;

    value = default_parameter_values[module->selected_function];

    if (module->shift_key)
    {
      // set _all_ knob values to the default
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
      {
        module->params[module->STEP_KNOBS + i].setValue(value);
      }
    }
    else
    {
      // set _this_ knob's values to the default
      module->params[parameter_index].setValue(value);
    }
  }

  void onButton(const event::Button &e) override
  {
    if (module->selected_function == FUNCTION_SAMPLE_START || module->selected_function == FUNCTION_SAMPLE_END)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        if (module->lcd_screen_mode != module->SAMPLE)
        {
          module->lcd_screen_mode = module->SAMPLE;
          module->visualizer_step = step;
        }
      }
    }

    if (module->selected_function == FUNCTION_RATCHET)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        if (module->lcd_screen_mode != module->RATCHET)
        {
          module->lcd_screen_mode = module->RATCHET;
          module->visualizer_step = step;
        }
      }
    }

    SvgKnob::onButton(e);
  }

  void onDragEnd(const DragEndEvent &e) override
  {

    if (module->selected_function == FUNCTION_SAMPLE_START || module->selected_function == FUNCTION_SAMPLE_END)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT)
        module->lcd_screen_mode = module->TRACK;
    }

    if (module->selected_function == FUNCTION_RATCHET)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT)
        module->lcd_screen_mode = module->TRACK;
    }

    SvgKnob::onDragEnd(e);
  }

  struct RandomizeParamMenuItem : MenuItem
  {
    GrooveBox *module;

    void onAction(const event::Action &e) override
    {
      module->randomizeSelectedParameter();
    }
  };

  struct ResetParamMenuItem : MenuItem
  {
    GrooveBox *module;

    void onAction(const event::Action &e) override
    {
      module->resetSelectedParameter();
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
    assert(module);

    menu->addChild(new MenuSeparator);

    // Randomize parameters
    RandomizeParamMenuItem *randomize_param_menu_item = createMenuItem<RandomizeParamMenuItem>("Randomize Knobs");
    randomize_param_menu_item->module = module;
    menu->addChild(randomize_param_menu_item);

    // Reset all knobs
    ResetParamMenuItem *reset_param_menu_item = createMenuItem<ResetParamMenuItem>("Reset Knobs");
    reset_param_menu_item->module = module;
    menu->addChild(reset_param_menu_item);    
  }
};