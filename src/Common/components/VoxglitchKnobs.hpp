struct VoxglitchKnob : SvgKnob {

  // Copied from Count Modula:
  std::string svgFile = "";
	float orientation = 0.0;

	VoxglitchKnob() {
		svgFile = "";
		orientation = 0.0;
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
    shadow->opacity = 0;
	}

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

// Large Knob
template <typename TBase = VoxglitchKnob>
struct TLargeKnob : TBase {
	TLargeKnob() {
		this->svgFile = "large_knob_overlay2.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));
	}
};
typedef TLargeKnob<> VoxglitchLargeKnob;

// Medium Knob
template <typename TBase = VoxglitchKnob>
struct TMediumKnob : TBase {
	TMediumKnob() {
		this->svgFile = "medium_knob.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));
	}
};
typedef TMediumKnob<> VoxglitchMediumKnob;

// Attenuator
template <typename TBase = VoxglitchKnob>
struct TAttenuator : TBase {
	TAttenuator() {
		this->svgFile = "attenuator_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));
	}
};
typedef TAttenuator<> VoxglitchAttenuator;
