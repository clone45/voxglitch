struct FourTrackWidget : VoxglitchSamplerModuleWidget
{
    FourTrackWidget(FourTrack *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/four_track/four_track_panel.svg"),
            asset::plugin(pluginInstance, "res/four_track/four_track_panel-dark.svg"));

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Start, stop, reset inputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("start_input"), module, FourTrack::START_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("stop_input"), module, FourTrack::STOP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, FourTrack::RESET_INPUT));

        // Start, stop, reset buttons
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<GreenLight>>>(panelHelper.findNamed("start_button"), module, FourTrack::START_BUTTON, FourTrack::START_LIGHT));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<RedLight>>>(panelHelper.findNamed("stop_button"), module, FourTrack::STOP_BUTTON, FourTrack::STOP_LIGHT));
        addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(panelHelper.findNamed("reset_button"), module, FourTrack::RESET_BUTTON, FourTrack::RESET_LIGHT));

        // Left and right outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, FourTrack::OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, FourTrack::OUTPUT_RIGHT));

        // Marker outputs
        for (int i = 0; i < 32; i++)
        {
            addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("marker_output_" + std::to_string(i + 1)), module, FourTrack::MARKER_OUTPUTS + i));
        }

        // Marker buttons using small light buttons
        for (int i = 0; i < 32; i++)
        {
            addParam(createLightParamCentered<SmallLightButton<componentlibrary::WhiteLight>>(
                panelHelper.findNamed("marker_selector_" + std::to_string(i + 1)),
                module,
                FourTrack::MARKER_BUTTONS + i,
                FourTrack::MARKER_LIGHTS + i));
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
            waveform_widget->setIndicatorColor(nvgRGBA(255, 0, 0, 255));
            waveform_widget->setContainerBackgroundColor(nvgRGB(0x10, 0x20, 0x20));
            waveform_widget->setDrawContainerBackground(true);
            addChild(waveform_widget);
        }
    }

    void appendContextMenu(Menu *menu) override
    {
        FourTrack *module = dynamic_cast<FourTrack *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Sample"));

        // Add the sample slot to the right-click context menu
        FourTrackLoadSample *menu_item_load_sample = new FourTrackLoadSample();
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
