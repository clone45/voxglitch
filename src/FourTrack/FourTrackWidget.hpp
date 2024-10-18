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

        // Add the track widgets
        TrackWidget *track_widget_1 = new TrackWidget(140.0, 25.0, 468.0, 100.0, &module->track1);

        addChild(track_widget_1);
    }

    void appendContextMenu(Menu *menu) override
    {
        FourTrack *module = dynamic_cast<FourTrack *>(this->module);
        assert(module);
    }
};
