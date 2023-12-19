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
};

// Large Knob
template <typename TBase = VoxglitchKnob>
struct TLargeKnob : TBase
{
  ImageWidget* bg;
  ImageWidget* shadow;
	TLargeKnob() {

    // Draw Knob Overlay
		this->svgFile = "large_knob_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));

    // Add the knob's PNG background to the bottom
    bg = new ImageWidget("res/components/png/Big-Knob.png", 22.2, 24.2);
    this->addChildBottom(bg);
    bg->setPosition(Vec(-2.8, -2.25));

    // Add the shadow below everything
    shadow = new ImageWidget("res/themes/default/round_shadow.png", 32.0, 32.0);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-16.0, 0.0));
	}
};
typedef TLargeKnob<> VoxglitchLargeKnob;

// Medium Knob
template <typename TBase = VoxglitchKnob>
struct TMediumKnob : TBase {
  ImageWidget* bg;
  ImageWidget* shadow;
	TMediumKnob() {
		this->svgFile = "medium_knob_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));

    bg = new ImageWidget("res/components/png/knob_medium_light.png", 12.4, 12.4);
    this->addChildBottom(bg);

    shadow = new ImageWidget("res/themes/default/round_shadow.png", 16.4, 16.4);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-5.0, 3.0));

	}
};
typedef TMediumKnob<> VoxglitchMediumKnob;


// Medium Knob black
template <typename TBase = VoxglitchKnob>
struct TMediumBlackKnob : TBase {
  ImageWidget* bg;
  ImageWidget* shadow;
	TMediumBlackKnob() {
		this->svgFile = "medium_knob_black_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));

    bg = new ImageWidget("res/components/png/knob_medium_black.png", 12.4, 12.4);
    this->addChildBottom(bg);

    shadow = new ImageWidget("res/themes/default/round_shadow.png", 16.4, 16.4);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-5.0, 3.0));

	}
};
typedef TMediumBlackKnob<> VoxglitchMediumBlackKnob;

// Attenuator
template <typename TBase = VoxglitchKnob>
struct TAttenuator : TBase {
  ImageWidget* bg;
  ImageWidget* shadow;
	TAttenuator() {
		this->svgFile = "attenuator_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));

    bg = new ImageWidget("res/components/png/knob_small.png", 6.0, 6.0);
    this->addChildBottom(bg);
    bg->setPosition(Vec(0.2, 0.2));

    shadow = new ImageWidget("res/themes/default/round_shadow.png", 8.4, 8.4);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-3.0, 1.2));
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


template <typename TBase = VoxglitchKnob>
struct TEpicKnob : TBase {
	TEpicKnob() {
    this->minAngle = -0.76*M_PI;
		this->maxAngle = 0.76*M_PI;
		this->svgFile = "epic_knob_overlay.svg";
		this->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/" + this->svgFile)));
	}
};
typedef TEpicKnob<> VoxglitchEpicKnob;
