struct EngineKnob : RoundBlackKnob
{
  void onButton(const event::Button &e) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

    if(module)
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        module->setLCDFocus(LCD_ENGINE_DISPLAY);
      }
    }
    RoundBlackKnob::onButton(e);
  }
};

struct ParameterKnob : RoundBlackKnob
{
  unsigned int lcd_focus = LCD_NO_FOCUS_CHANGE;
  unsigned int parameter_number = 0;

  void onButton(const event::Button &e) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

    if(module)
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        module->selectParameter(parameter_number);
      }
    }
    RoundBlackKnob::onButton(e);
  }
  /* Nope
  void setLCDFocus(unsigned int new_lcd_focus)
  {
    lcd_focus = new_lcd_focus;
  }
  */
};
