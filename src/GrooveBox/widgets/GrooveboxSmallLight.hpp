struct StepLightWidget : SvgWidget
{

  bool *state_bool_ptr = NULL;
  std::vector<std::shared_ptr<window::Svg>> frames;

  StepLightWidget(bool *state)
  {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_led.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_led_lit.svg")));

    this->state_bool_ptr = state;
  }

  void addFrame(std::shared_ptr<Svg> svg)
  {
    this->frames.push_back(svg);

    // If this is our first frame, automatically set SVG and size
    if (!this->svg)
    {
      this->setSvg(svg);
      box.size = svg->getSize();
    }
  };

  void step() override
  {
    SvgWidget::step();

    if (state_bool_ptr != NULL)
    {
      this->setSvg(frames[(*state_bool_ptr) ? 1 : 0]);
    }
    else
    {
      this->setSvg(frames[0]);
    }
  }
};
