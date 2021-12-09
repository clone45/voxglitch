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

    /*
    DPSliderDisplay *dp_slider_display = new DPSliderDisplay(0.0, 0.0, &module->sliders[0]);
    dp_slider_display->setPosition(mm2px(Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y)));
    dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
    dp_slider_display->module = module;
    addChild(dp_slider_display);
    */
    DPSliderDisplay *dp_slider_display2 = new DPSliderDisplay(0.0,0.0, &module->sliders[1 + 0]);
    dp_slider_display2->setPosition(mm2px(Vec(DRAW_AREA_POSITION_X + SLIDER_HORIZONTAL_PADDING, DRAW_AREA_POSITION_Y)));
    dp_slider_display2->setSize(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
    dp_slider_display2->module = module;
    addChild(dp_slider_display2);

    // Cosmetic rack screws
    // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    // addChild(createWidget<ScrewSilver>(Vec(15, 365)));
    // addChild(createWidget<ScrewSilver>(mm2px(Vec(171.5, 0))));
  }
};
