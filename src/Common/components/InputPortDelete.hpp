struct InputPort : app::SvgPort {
	InputPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/input-port-simplified2.svg")));
	}
};
