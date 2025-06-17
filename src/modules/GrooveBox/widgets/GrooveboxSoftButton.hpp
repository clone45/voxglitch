struct GrooveboxSoftButton : SvgSwitch
{
    GrooveBox *module;
	NVGcolor halo_color = nvgRGBA(255, 204, 143, 255);

    GrooveboxSoftButton()
    {
        momentary = false;
        // shadow->opacity = 0;
                                                                     
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/modules/groovebox/groove_box_soft_button.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/modules/groovebox/groove_box_soft_button_lit.svg")));

        box.size = Vec(16.240, 16.240); // was 15.5   (19.28)
    }
   
    void drawLayer(const DrawArgs& args, int layer) override 
    {
        if (layer == 1)
        {
            ParamQuantity *param_quantity = this->getParamQuantity();

            if (!param_quantity || module == nullptr)
            {
                // We're in the library view or don't have a valid param_quantity
                // Draw the default state (unlit button)
                SvgSwitch::draw(args);
            }
            else if (momentary)
            {
                // For momentary buttons, check the light value
                if (module->lights[paramId].getBrightness() > 0.f)
                {
                    nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
                    SvgSwitch::draw(args);
                    drawHalo(args);
                }
                else
                {
                    SvgSwitch::draw(args);
                }
            }
            else if (param_quantity->getValue()) 
            {
                nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
                SvgSwitch::draw(args);
                drawHalo(args);
            }
        }
        
        Widget::drawLayer(args, layer);
    }    

    void draw(const DrawArgs& args) override 
    {
        ParamQuantity *param_quantity = this->getParamQuantity();

        if (!param_quantity || module == nullptr || momentary) 
        {
            // We're in the library view, don't have a valid param_quantity, or it's a one-shot button
            // Draw the default state (unlit button)
            SvgSwitch::draw(args);
        }
        else if (param_quantity->getValue() == false) 
        {
            SvgSwitch::draw(args);
        }
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
