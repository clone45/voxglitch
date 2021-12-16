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
    float margin_left = 7;
    float horizontal_padding = 11.0;

    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      panel_x_position = (column * horizontal_padding) + margin_left;
      panel_y_position = 20.0;

      DPSliderDisplay *dp_slider_display = new DPSliderDisplay(&module->sliders[column]);
      dp_slider_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
      dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT));
      dp_slider_display->module = module;
      addChild(dp_slider_display);

    }

    float inputs_margin_left = 11.0;
    float inputs_vertical_position = 10.0;

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((0 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_0));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((1 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_1));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((2 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_2));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((3 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_3));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((4 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_4));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((5 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_5));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((6 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_6));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((7 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_7));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((8 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_8));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((9 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_9));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((10 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_10));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((11 * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, DigitalProgrammer::CV_OUTPUT_11));


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
