struct StepLightWidget : SvgWidget
{

  bool *state_bool_ptr = NULL;
  std::vector<std::shared_ptr<window::Svg>> frames;
  NVGcolor halo_color = nvgRGBA(202, 16, 21, 255);

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

  void draw(const DrawArgs &args) override
  {
      // For some reason, it's important to draw this background even if the
      // drawLayer function draws on top of it.  Somthing about the composit
      // blend function?  If you don't draw this beneath drawLayer, the active
      // light is too light colored.
      SvgWidget::draw(args);
  }

  void drawLayer(const DrawArgs &args, int layer) override
  {
    // If layer == 1, and the state_bool_ptr not null, and it's false, then...
    if (layer == 1 && state_bool_ptr && *state_bool_ptr)
    {
      nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
      SvgWidget::draw(args);
      drawHalo(args);
    }

    Widget::drawLayer(args, layer);
  }

  //
  // copied from https://github.com/VCVRack/Rack/blob/05fa24a72bccf4023f5fb1b0fa7f1c26855c0926/src/app/LightWidget.cpp#LL63-L63C51
  //
  void drawHalo(const DrawArgs& args) {
      // Don't draw halo if rendering in a framebuffer, e.g. screenshots or Module Browser
      if (args.fb)
          return;

      const float halo = settings::haloBrightness;
      if (halo == 0.f)
          return;

      // If light is off, rendering the halo gives no effect.
      if (halo_color.r == 0.f && halo_color.g == 0.f && halo_color.b == 0.f)
          return;

      math::Vec c = box.size.div(2);
      float radius = std::min(box.size.x, box.size.y) / 2.0;
      float oradius = radius + std::min(radius * 4.f, 15.f);

      nvgBeginPath(args.vg);
      nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

      NVGcolor icol = color::mult(halo_color, halo);
      NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
      NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
      nvgFillPaint(args.vg, paint);
      nvgFill(args.vg);
  }  
};
