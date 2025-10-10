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
    // voxglitch_shadow = new ImageWidget("res/themes/default/square_shadow.png", 15.0, 15.0, 0.4);
    // this->addChildBottom(voxglitch_shadow);
    // voxglitch_shadow->setPosition(Vec(-11, -8));

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

// TempestVS1 Switches

// VS1 Push Button (for Mode, Preset, Clear buttons)
struct VS1PushButton : SvgSwitch {
	VS1PushButton() {
		momentary = true;
		shadow->opacity = 0;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/vs1_pushbutton_off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/vs1_pushbutton_on.svg")));
	}
};

// NKK Two-Position Switch (uses VCV Rack's built-in component)
struct NKKTwoPosition : SvgSwitch {
	NKKTwoPosition() {
		shadow->opacity = 0;
		// Only load the down (0) and up (2) frames, skipping middle (1)
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/NKK_0.svg")));
		addFrame(APP->window->loadSvg(asset::system("res/ComponentLibrary/NKK_2.svg")));
	}
};
