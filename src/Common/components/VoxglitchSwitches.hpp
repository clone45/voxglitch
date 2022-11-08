// When adding a new switch
// 1. ensure box size is correct before anything else.  It should be the same
//    size as the SVG measurement in px
// 2. To fix issues with exported SVGs, you may need to create a brand new
//    SVG in inkscape, ungroup the elements, then copy them over and align them
//    to the page.

struct VoxglitchSwitch : SvgSwitch {

  VoxglitchSwitch()
  {

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

struct VoxglitchRoundLampSwitch : VoxglitchSwitch {

  ImageWidget* bg;
  ImageWidget* voxglitch_shadow;

  VoxglitchRoundLampSwitch()
  {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/round_light_off.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/round_light_on.svg")));

    // Add the shadow below everything
    voxglitch_shadow = new ImageWidget("res/themes/default/round_shadow.png", 10.0, 10.0, 0.8);
    this->addChildBottom(voxglitch_shadow);
    voxglitch_shadow->setPosition(Vec(-5.5, -3.3));

    // Set box size.  This affects the center position
    box.size = Vec(15.5,15.5);

    // Turn off Rack shadow
    this->shadow->opacity = 0;
  }

  void draw(const DrawArgs& args) override
  {
    SvgSwitch::draw(args);

    if(module)
    {
      //
      // Draw glow effect
      //
      ParamQuantity *param_quantity = getParamQuantity();

      if(! (param_quantity->getValue() == param_quantity->getMinValue()))
      {
        math::Vec c = box.size.div(2);

        c.x += 1.46; // offset a little bit to correct for small box size
        c.y += 1.46;

        float radius = std::min(box.size.x, box.size.y) / 2.0;
        float oradius = radius + std::min(radius * 1.5f, 4.5f);  // was 2.5 f max

        nvgBeginPath(args.vg);
        nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

        NVGcolor icol = nvgRGBA(255, 208, 183, 60);
        NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
        NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
        nvgFillPaint(args.vg, paint);
        nvgFill(args.vg);
      }
    }
  }

};

struct VoxglitchRoundToggleLampSwitch : VoxglitchRoundLampSwitch {
  VoxglitchRoundToggleLampSwitch() {
    momentary = false;
  }
};

struct VoxglitchRoundMomentaryLampSwitch : VoxglitchRoundLampSwitch {
  VoxglitchRoundMomentaryLampSwitch() {
    momentary = true;
  }
};

struct squareToggle : VoxglitchSwitch {

  ImageWidget* voxglitch_shadow;

  squareToggle() 
  {
    momentary = false;
    shadow->opacity = 0;

    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/square_light_off.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/square_light_on.svg")));

    // Add the shadow below everything
    voxglitch_shadow = new ImageWidget("res/themes/default/square_shadow.png", 15.0, 15.0, 0.4);
    this->addChildBottom(voxglitch_shadow);
    voxglitch_shadow->setPosition(Vec(-11, -8));

    box.size = Vec(22.0, 22.0); // was 15.5   (19.28)
  }

  void draw(const DrawArgs& args) override
  {
    // Draw background shadow
    nvgBeginPath(args.vg);
    nvgRect(args.vg, -1, -1, box.size.x + 3.0, box.size.y + 3.0);
    nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 60));
    nvgFill(args.vg);

    SvgSwitch::draw(args);

    //
    // Draw glow effect
    //

    if(module)
    {
      ParamQuantity *param_quantity = getParamQuantity();

      if(! (param_quantity->getValue() == param_quantity->getMinValue()))
      {
        math::Vec c = box.size.div(2);
        float radius = std::min(box.size.x, box.size.y) / 2.0;
        float oradius = radius + std::min(radius * 2.f, 8.f);

        nvgBeginPath(args.vg);
        nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

        NVGcolor icol = nvgRGBA(255, 208, 183, 60);
        NVGcolor ocol = nvgRGBA(0, 0, 0, 0);
        NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
        nvgFillPaint(args.vg, paint);
        nvgFill(args.vg);
      }
    }
  }

};
