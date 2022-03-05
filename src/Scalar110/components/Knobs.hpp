struct EngineKnob : RoundBlackKnob
{
  EngineKnob(unsigned int foo)
  {

  }

  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      DEBUG("I was clicked");
      e.consume(this);
    }
  }
};
