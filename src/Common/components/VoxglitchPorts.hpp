struct VoxglitchPort : SvgPort {

VoxglitchPort()
{
  this->shadow->opacity = 0;
}

//
// When in "DEV_MODE", which is set in plugin.hpp, hover over components
// and use the adsw keys to nudge the components into place.
//

#ifdef DEV_MODE
  void onHoverKey(const event::HoverKey &e) override
  {
    bool shift_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
    float shift_amount = .05;

    if(shift_key) shift_amount = shift_amount * 100;

    if(e.action == GLFW_PRESS)
    {
      if(e.key == GLFW_KEY_A) this->box.pos.x -= shift_amount;
      if(e.key == GLFW_KEY_D) this->box.pos.x += shift_amount;
      if(e.key == GLFW_KEY_W) this->box.pos.y -= shift_amount;
      if(e.key == GLFW_KEY_S) this->box.pos.y += shift_amount;

      // get center point of location
      float panel_x = this->box.pos.x + (this->box.size.x / 2);
      float panel_y = this->box.pos.y + (this->box.size.y / 2);

      std::string debug_string = "New box.pos: " + std::to_string(panel_x) + "," + std::to_string(panel_y);
      DEBUG(debug_string.c_str());
    }
  }
#endif
};

struct BlankPort : VoxglitchPort {
	BlankPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/blank-port.svg")));
	}
};

struct VoxglitchInputPort : VoxglitchPort {
  ImageWidget* shadow;
	VoxglitchInputPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_input_port.svg")));

    shadow = new ImageWidget(asset::plugin(pluginInstance, "res/themes/default/round_shadow.png"), 12.0, 12.0, 0.7);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-5.0, 0.0));
	}
};

struct VoxglitchOutputPort : VoxglitchPort {
  ImageWidget* shadow;
	VoxglitchOutputPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_output_port.svg")));

    shadow = new ImageWidget(asset::plugin(pluginInstance, "res/themes/default/round_shadow.png"), 12.0, 12.0, 0.7);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-5.0, 0.0));
	}
};
