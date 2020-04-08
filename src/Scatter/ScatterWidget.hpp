struct ScatterWidget : ModuleWidget
{
	ScatterWidget(Scatter* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scatter_front_panel.svg")));

	}

};
