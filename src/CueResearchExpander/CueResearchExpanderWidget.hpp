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

        // Add all inputs and selectors using named positions from SVG
        for (int i = 0; i < 32; i++) {
            // Add input jack
            addInput(createInputCentered<VoxglitchInputPort>(
                panelHelper.findNamed("marker_input_" + std::to_string(i + 1)),
                module,
                CueResearchExpander::MARKER_INPUTS + i
            ));

            // Add selector button with light
            addParam(createLightParamCentered<SmallLightButton<componentlibrary::WhiteLight>>(
                panelHelper.findNamed("marker_selector_" + std::to_string(i + 1)),
                module,
                CueResearchExpander::MARKER_SELECTORS + i,
                CueResearchExpander::MARKER_SELECTOR_LIGHTS + i
            ));
        }
    }

    void appendContextMenu(Menu *menu) override
    {
        CueResearchExpander *module = dynamic_cast<CueResearchExpander *>(this->module);
        assert(module);

        // menu->addChild(new MenuEntry); // For spacing only
        // menu->addChild(createMenuLabel("Menu Title Goes Here"));
    }
};
