struct VoxglitchScrew : Widget {

	float orientation = 0.0;

	VoxglitchScrew() {
		orientation = 0.0;
    // this->shadow->opacity = 0;
	}

};


template <typename TBase = VoxglitchScrew>
struct TScrewHexBlack : TBase
{
  ImageWidget* bg;
  ImageWidget* shadow;
  TScrewHexBlack() {

    bg = new ImageWidget(asset::plugin(pluginInstance, "res/components/png/screw_light.png"), 5.0, 5.0);
    this->addChild(bg);

    shadow = new ImageWidget(asset::plugin(pluginInstance, "res/themes/default/round_shadow.png"), 6.0, 6.0);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-1.5, 1.2));
  }
};
typedef TScrewHexBlack<> ScrewHexBlack;
