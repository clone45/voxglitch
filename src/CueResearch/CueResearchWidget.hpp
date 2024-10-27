struct CueResearchWidget : VoxglitchSamplerModuleWidget
{
    CueResearchWidget(CueResearch *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/cue_research/cue_research_panel.svg"),
            asset::plugin(pluginInstance, "res/cue_research/cue_research_panel-dark.svg"));

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Start, stop, reset inputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("start_input"), module, CueResearch::START_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("stop_input"), module, CueResearch::STOP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, CueResearch::RESET_INPUT));

        // Start, stop, reset buttons
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(panelHelper.findNamed("start_button"), module, CueResearch::START_BUTTON, CueResearch::START_LIGHT));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(panelHelper.findNamed("stop_button"), module, CueResearch::STOP_BUTTON, CueResearch::STOP_LIGHT));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(panelHelper.findNamed("reset_button"), module, CueResearch::RESET_BUTTON, CueResearch::RESET_LIGHT));

        // Left and right outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, CueResearch::OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, CueResearch::OUTPUT_RIGHT));

        // Marker outputs
        for (int i = 0; i < 32; i++)
        {
            addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("marker_output_" + std::to_string(i + 1)), module, CueResearch::MARKER_OUTPUTS + i));
        }

        // Marker buttons using small light buttons
        for (int i = 0; i < 32; i++)
        {
            addParam(createLightParamCentered<SmallLightButton<componentlibrary::WhiteLight>>(
                panelHelper.findNamed("marker_selector_" + std::to_string(i + 1)),
                module,
                CueResearch::MARKER_BUTTONS + i,
                CueResearch::MARKER_LIGHTS + i));
        }

        // Add the track widgets
        if (module)
        {
            TrackWidget *track_widget = new TrackWidget(92.0, 25.0, 468.0, 280.0, &module->track);
            addChild(track_widget);

            // Add the lower waveform widget
            WaveformWidget *waveform_widget = new WaveformWidget(92.0, 312.5, 468.0, 40.0, &module->waveform_model);
            waveform_widget->visible = true;
            waveform_widget->setIndicatorWidth(1.0);
            waveform_widget->setIndicatorColor(nvgRGBA(255, 215, 20, 200));
            waveform_widget->setContainerBackgroundColor(nvgRGB(0x10, 0x20, 0x20));
            waveform_widget->setDrawContainerBackground(true);
            addChild(waveform_widget);
        }
    }

    void appendContextMenu(Menu *menu) override
    {
        CueResearch *module = dynamic_cast<CueResearch *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Sample"));

        // Add the sample slot to the right-click context menu
        CueResearchLoadSample *menu_item_load_sample = new CueResearchLoadSample();
        menu_item_load_sample->text = module->loaded_filename;
        menu_item_load_sample->module = module;
        menu->addChild(menu_item_load_sample);

        SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
        sample_interpolation_menu_item->module = module;
        menu->addChild(sample_interpolation_menu_item);

        // Add separator and our new options
        menu->addChild(new MenuEntry);  // Another separator
        menu->addChild(createMenuLabel("Options"));
        menu->addChild(createBoolPtrMenuItem("Enable Vertical-Drag Zoom", "", &module->enable_vertical_drag_zoom));
        menu->addChild(createBoolPtrMenuItem("Lock Markers", "", &module->lock_markers));

        menu->addChild(createIndexSubmenuItem("Trigger Length",
            module->getTriggerLengthNames(),
            [=]() {
                return (module->trigger_length_index);
            },
            [=](int trigger_length_index) {
                module->setTriggerLengthIndex(trigger_length_index);
            }
        ));

    }
};
