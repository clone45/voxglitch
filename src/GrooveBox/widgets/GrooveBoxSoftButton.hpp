struct GrooveboxSoftButton : SvgSwitch
{
    GrooveBox *module;
    bool is_moused_over = false;

    Vec drag_position;
  
    GrooveboxSoftButton()
    {
        momentary = false;
        shadow->opacity = 0;
                                                                     
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_soft_button_lit.svg")));

        box.size = Vec(16.240, 16.240); // was 15.5   (19.28)
    }

    void draw(const DrawArgs &args) override
    {
        SvgSwitch::draw(args);
    }  
};
