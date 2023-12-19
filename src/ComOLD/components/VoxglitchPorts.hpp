struct VoxglitchPort : SvgPort
{
  VoxglitchPort()
  {
    this->shadow->opacity = 0;
  }
};

struct BlankPort : VoxglitchPort
{
  BlankPort()
  {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/blank-port.svg")));
  }
};

struct VoxglitchInputPort : VoxglitchPort
{
  ImageWidget *shadow;
  VoxglitchInputPort()
  {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_input_port.svg")));

    shadow = new ImageWidget("res/themes/default/round_shadow.png", 12.0, 12.0, 0.7);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-5.0, 0.0));
  }
};

struct VoxglitchOutputPort : VoxglitchPort
{
  ImageWidget *shadow;
  VoxglitchOutputPort()
  {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_output_port.svg")));

    shadow = new ImageWidget("res/themes/default/round_shadow.png", 12.0, 12.0, 0.7);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-5.0, 0.0));
  }
};

struct VoxglitchPolyPort : VoxglitchPort
{
  ImageWidget *shadow;
  VoxglitchPolyPort()
  {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/voxglitch_poly_port.svg")));

    shadow = new ImageWidget("res/themes/default/round_shadow.png", 12.0, 12.0, 0.7);
    this->addChildBottom(shadow);
    shadow->setPosition(Vec(-5.0, 0.0));
  }
};
