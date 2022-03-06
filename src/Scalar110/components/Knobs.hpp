struct EngineKnob : RoundBlackKnob
{
  Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

  void onButton(const event::Button &e) override
  {
    if(module)
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        module->setLCDFocus(LCD_ENGINE_DISPLAY);
      }
    }
    RoundBlackKnob::onButton(e);
    e.consume(this);
  }

};

struct QuantumKnob : RoundBlackKnob
{
  Scalar110 *module = dynamic_cast<Scalar110*>(this->module);
  unsigned int lcd_focus = LCD_NO_FOCUS_CHANGE;

  void onButton(const event::Button &e) override
  {
    if(module)
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        if(lcd_focus != LCD_NO_FOCUS_CHANGE) module->setLCDFocus(lcd_focus);
      }
    }
    RoundBlackKnob::onButton(e);
    e.consume(this);
  }

};

struct ParameterKnob : RoundBlackKnob
{
  unsigned int lcd_focus = LCD_NO_FOCUS_CHANGE;
  unsigned int parameter_number = 0;

  void onButton(const event::Button &e) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

    DEBUG("button clicked");
    if(module)
    {
      DEBUG("module is defined");
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        module->selectParameter(parameter_number);
        DEBUG("parameter selected");
      }
    }
    RoundBlackKnob::onButton(e);
  }
};
