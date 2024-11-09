struct CueResearchExpanderWidget : ModuleWidget
{
    CueResearchExpanderWidget(CueResearchExpander *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/cue_research_expander/cue_research_expander_panel.svg"),
            asset::plugin(pluginInstance, "res/cue_research_expander/cue_research_expander_panel-dark.svg"));

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // TODO: Add additional widgets here
    }

    void appendContextMenu(Menu *menu) override
    {
        CueResearchExpander *module = dynamic_cast<CueResearchExpander *>(this->module);
        assert(module);

        // menu->addChild(new MenuEntry); // For spacing only
        // menu->addChild(createMenuLabel("Menu Title Goes Here"));
    }
};
