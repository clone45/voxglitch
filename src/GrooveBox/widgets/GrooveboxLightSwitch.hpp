struct GrooveboxLightSwitch : LightWidget
{
    GrooveBox *module;
    bool switch_position = false;

    bool momentary = false;

	// struct Internal;
	// Internal* internal;

	widget::FramebufferWidget* fb;
	CircularShadow* shadow;
	std::vector<std::shared_ptr<window::Svg>> frames;


    GrooveboxLightSwitch()
    {
        // From https://github.com/VCVRack/Rack/blob/05fa24a72bccf4023f5fb1b0fa7f1c26855c0926/src/app/SvgSwitch.cpp#L9-L17
	    fb = new widget::FramebufferWidget;
	    addChild(fb);

        shadow = new CircularShadow;
        fb->addChild(shadow);
        shadow->box.size = math::Vec();
    /*
        // sw = new widget::SvgWidget;
        // fb->addChild(sw);

        //
        // Custom code
        //
        // shadow->opacity = 0;
                                                                     
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button_lit.svg")));

        box.size = Vec(16.240, 16.240); // was 15.5   (19.28)
            */
    }


    void addFrame(std::shared_ptr<window::Svg> svg) {
        frames.push_back(svg);
        // If this is our first frame, automatically set SVG and size
        /*
        if (frames.size() == 1) {
            box.size = sw->box.size;
            fb->box.size = sw->box.size;
            // Move shadow downward by 10%
            shadow->box.size = sw->box.size;
            shadow->box.pos = math::Vec(0, sw->box.size.y * 0.10);
        }
        */
    }

    // Methods copied from SvgWidget
    // https://github.com/VCVRack/Rack/blob/05fa24a72bccf4023f5fb1b0fa7f1c26855c0926/src/widget/SvgWidget.cpp

    // Removed code.  I don't need to override the svgwidget class because I 
    // maintain a pointer to one called "sw"



    // Overriding the LightWidget draw methods from
    // https://github.com/VCVRack/Rack/blob/05fa24a72bccf4023f5fb1b0fa7f1c26855c0926/src/app/LightWidget.cpp#L10-L60C2


    void draw(const DrawArgs& args) override {
        if(switch_position == false) drawOffState(args);
        // Note: The "on" state is conditionally drawn by drawLayer (see below)
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer == 1 && (switch_position == true)) {
            // Use the formula `lightColor * (1 - dest) + dest` for blending
            nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
            drawOnState(args);
        }
        // Widget::drawLayer(args, layer);
    }

    void drawOffState(const DrawArgs& args) {
        window::svgDraw(args.vg, frames[0]->handle);
    }

    void drawOnState(const DrawArgs& args) {
        window::svgDraw(args.vg, frames[1]->handle);
    }   
};
