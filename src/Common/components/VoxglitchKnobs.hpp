struct VoxglitchKnob : SvgKnob {

  // Copied from Count Modula:
  std::string svgFile = "";
	float orientation = 0.0;

	VoxglitchKnob() {
		svgFile = "";
		orientation = 0.0;
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
	}
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


// Attenuator
template <typename TBase = VoxglitchKnob>
struct TAttenuator : TBase {
	TAttenuator() {
		this->svgFile = "attenuator_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));
	}
};
typedef TAttenuator<> VoxglitchAttenuator;
