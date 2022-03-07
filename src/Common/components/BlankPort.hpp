struct BlankPort : app::SvgPort {
	BlankPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/blank-port.svg")));
	}
};
