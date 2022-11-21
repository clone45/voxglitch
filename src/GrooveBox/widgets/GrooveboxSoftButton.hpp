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
   
    void drawLayer(const DrawArgs& args, int layer) override {

        ParamQuantity *param_quantity = this->getParamQuantity();

        if (layer == 1 && param_quantity && param_quantity->getValue()) {
            nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
            SvgSwitch::draw(args);
        }
        
        Widget::drawLayer(args, layer);
    }    
};
