struct FourTrackWidget : VoxglitchSamplerModuleWidget
{
    FourTrackWidget(FourTrack *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/four_track/four_track_panel.svg"),
            asset::plugin(pluginInstance, "res/four_track/four_track_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Start, stop, reset inputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("start_input"), module, FourTrack::START_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("stop_input"), module, FourTrack::STOP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, FourTrack::RESET_INPUT));

        // Left and right outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, FourTrack::OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, FourTrack::OUTPUT_RIGHT));

        // Marker outputs
        for (int i = 0; i < 32; i++)
        {
            addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("marker_output_" + std::to_string(i + 1)), module, FourTrack::MARKER_OUTPUTS + i));
        }

        // Marker buttons using small light buttons
        for (int i = 0; i < 32; i++) {
            addParam(createLightParamCentered<SmallLightButton<componentlibrary::WhiteLight>>(
                panelHelper.findNamed("marker_selector_" + std::to_string(i + 1)),
                module,
                FourTrack::MARKER_BUTTONS + i,
                FourTrack::MARKER_LIGHTS + i
            ));
        }

        // Add the track widgets
        if(module)
        {
            TrackWidget *track_widget = new TrackWidget(46.0, 15.0, 468.0, 250.0, &module->track);
            addChild(track_widget);

            // Add the lower waveform widget
            WaveformWidget *waveform_widget = new WaveformWidget(46.0, 305.0, 468.0, 35.0, &module->waveform_model);
            waveform_widget->visible = true;
            waveform_widget->setIndicatorWidth(1.0);
            waveform_widget->setIndicatorColor(nvgRGBA(255, 0, 0, 255));
            addChild(waveform_widget);
        }

    }

    void appendContextMenu(Menu *menu) override
    {
        FourTrack *module = dynamic_cast<FourTrack *>(this->module);
        assert(module);
    }
};
