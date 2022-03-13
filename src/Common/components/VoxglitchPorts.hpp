struct VoxglitchPort : SvgPort {

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
	VoxglitchInputPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/input-port-simplified2.svg")));
	}
};

struct VoxglitchOutputPort : VoxglitchPort {
	VoxglitchOutputPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/output-port-simplified.svg")));
	}
};
