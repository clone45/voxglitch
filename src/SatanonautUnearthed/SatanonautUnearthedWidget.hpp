struct SatanonautUnearthedWidget : ModuleWidget
{
    SatanonautUnearthedWidget(SatanonautUnearthed *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/satanonaut_unearthed/satanonaut_unearthed_panel.svg"),
            asset::plugin(pluginInstance, "res/satanonaut_unearthed/satanonaut_unearthed_panel-dark.svg")
        );
    }
};