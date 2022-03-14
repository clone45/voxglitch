struct EngineKnob : RoundBlackKnob
{
  EngineKnob()
  {
    minAngle = -0.83*M_PI;
    maxAngle = -0.50*M_PI;
  }

  void onButton(const event::Button &e) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

    if(module)
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT)
      {
        if(e.action == GLFW_PRESS) module->setLCDPage(LCD_PAGE_ENGINE);
        if(e.action == GLFW_RELEASE) module->selectLCDFunctionSelectedParam();
      }

    }
    RoundBlackKnob::onButton(e);
  }

  void onLeave(const LeaveEvent &e) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

    if(module)
    {
      module->selectLCDFunctionSelectedParam();
    }
  }

  void onHover(const event::Hover& e) override
  {
		RoundBlackKnob::onHover(e);
		e.consume(this);
	}
};

struct ParameterKnob : RoundBlackKnob
{
  unsigned int lcd_function = LCD_PAGE_PARAMETER_VALUES; // Default
  unsigned int parameter_number = 0; // This gets set when the widget is created

  void onButton(const event::Button &e) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);

    if(module)
    {
      if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        module->selectParameter(parameter_number);
        module->selectLCDFunctionOnParameterFocus(parameter_number);
      }
    }
    RoundBlackKnob::onButton(e);
  }
};
