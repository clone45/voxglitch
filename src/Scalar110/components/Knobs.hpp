struct EngineKnob : RoundBlackKnob
{
  Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      module->setLCDFocus(LCD_ENGINE_DISPLAY);
      e.consume(this);
    }
  }
};
