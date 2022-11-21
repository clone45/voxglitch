struct GrooveboxSoftButton : SvgSwitch
{
    GrooveBox *module;

    GrooveboxSoftButton()
    {
        momentary = false;
        shadow->opacity = 0;
                                                                     
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button_lit.svg")));

        box.size = Vec(16.240, 16.240); // was 15.5   (19.28)
    }

    /*
    void draw(const DrawArgs &args) override
    {
        if(this->getParamQuantity()->getValue() == false)
        {
            SvgSwitch::draw(args);
        }
    }  
    */

    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer == 1 && (this->getParamQuantity()->getValue())) {
            // Use the formula `lightColor * (1 - dest) + dest` for blending
            nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
            // drawLight(args);
            // drawHalo(args);
            SvgSwitch::draw(args);
        }
        
        Widget::drawLayer(args, layer);
    }    
};
