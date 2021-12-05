/*
class TrimpotNoRandom : public Trimpot
{
public:
  void randomize() override {} // do nothing. base class would actually randomize
};
*/

/* Abandoning this front-panel control for now
struct FreezeToggle : app::SvgSwitch {
	FreezeToggle() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/freeze-button-off.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/freeze-button-on.svg")));
	};
};
*/

struct DigitalProgrammerWidget : ModuleWidget
{
  DigitalProgrammer* module;

  DigitalProgrammerWidget(DigitalProgrammer* module)
  {
    this->module = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_programmer_front_panel.svg")));

    // Cosmetic rack screws
    // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    // addChild(createWidget<ScrewSilver>(Vec(15, 365)));
    // addChild(createWidget<ScrewSilver>(mm2px(Vec(171.5, 0))));
  }
};
