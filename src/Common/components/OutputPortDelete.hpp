struct OutputPort : app::SvgPort {
	OutputPort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/output-port-simplified.svg")));
	}
};
