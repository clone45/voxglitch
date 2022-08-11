
struct VoxglitchModuleWidget : ModuleWidget
{

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
