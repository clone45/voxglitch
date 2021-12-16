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

    float panel_x_position = 0;
    float panel_y_position = 0;

    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      panel_x_position = (column * 11.0);
      panel_y_position = 10.0;

      DPSliderDisplay *dp_slider_display = new DPSliderDisplay(&module->sliders[column]);
      dp_slider_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
      dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
      dp_slider_display->module = module;
      addChild(dp_slider_display);
    }
/*
    float panel_x_position = 16.0;
    float panel_y_position = 10.0;

    DPSliderDisplay *dp_slider_display = new DPSliderDisplay(&module->sliders[0]);
    dp_slider_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
    dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
    dp_slider_display->module = module;
    addChild(dp_slider_display);

    panel_x_position = 32.0;
    panel_y_position = 10.0;

    DPSliderDisplay *dp_slider_display_2 = new DPSliderDisplay(&module->sliders[1]);
    dp_slider_display_2->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
    dp_slider_display_2->setSize(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
    dp_slider_display_2->module = module;
    addChild(dp_slider_display_2);
*/
  }
};
