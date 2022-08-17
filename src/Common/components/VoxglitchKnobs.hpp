struct VoxglitchKnob : SvgKnob {

  // Copied from Count Modula:
  std::string svgFile = "";
	float orientation = 0.0;

	VoxglitchKnob() {
		svgFile = "";
		orientation = 0.0;
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
    this->shadow->opacity = 0;
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
struct TLargeKnob : TBase
{
  ImageWidget* bg;
	TLargeKnob() {

		this->svgFile = "large_knob_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));

    bg = new ImageWidget(asset::plugin(pluginInstance, "res/components/png/Big-Knob.png"), 22.2, 24.2);
    this->addChildBottom(bg);

    // Offset background to align with overlay
    bg->setPosition(Vec(-2.8, -2.25));
	}
};
typedef TLargeKnob<> VoxglitchLargeKnob;

// Medium Knob
template <typename TBase = VoxglitchKnob>
struct TMediumKnob : TBase {
  ImageWidget* bg;
	TMediumKnob() {
		this->svgFile = "medium_knob_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));

    bg = new ImageWidget(asset::plugin(pluginInstance, "res/components/png/knob_medium_light.png"), 12.4, 12.4);
    this->addChildBottom(bg);
	}
};
typedef TMediumKnob<> VoxglitchMediumKnob;


// Attenuator
template <typename TBase = VoxglitchKnob>
struct TAttenuator : TBase {
  ImageWidget* bg;
	TAttenuator() {
		this->svgFile = "attenuator_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));

    bg = new ImageWidget(asset::plugin(pluginInstance, "res/components/png/knob_small.png"), 6.0, 6.0);
    this->addChildBottom(bg);

    // TEMPORARY, for quick adjustments ========================================
    /*
    json_t *json_root;
    json_error_t error;

    std::string config_file_path = asset::plugin(pluginInstance, "res/voxglitch_config.json");

    // Load theme selection, either "light" or "dark"
    json_root = json_load_file(config_file_path.c_str(), 0, &error);
    float offset_x = json_real_value(json_object_get(json_root, "x"));
    float offset_y = json_real_value(json_object_get(json_root, "y"));
    */
    // =========================================================================

    bg->setPosition(Vec(0.2, 0.2));
	}
};
typedef TAttenuator<> VoxglitchAttenuator;

template <typename TBase = VoxglitchKnob>
struct TValve : TBase {
	TValve() {
		this->svgFile = "valve.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));
	}
};
typedef TValve<> VoxglitchValve;


template <typename TBase = VoxglitchKnob>
struct TPainfulMediumKnob : TBase {
	TPainfulMediumKnob() {
		this->svgFile = "painful_medium_knob.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));
	}
};
typedef TPainfulMediumKnob<> PainfulMediumKnob;
