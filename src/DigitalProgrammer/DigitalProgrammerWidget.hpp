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

    // panel_x_position and panel_y_position specify where a slider should
    // be displayed on the front panel relative to 0,0.
    float panel_x_position = 0;
    float panel_y_position = 0;
    float margin_left = 7;  // offset from the far-lefthand side of the panel
    float horizontal_padding = 11.0; // Offset from the top of the panel

    // Location of the inputs relative to the panel
    float inputs_margin_left = 11.0;
    float inputs_vertical_position = 120.0;

    //
    // Loop through each column and draw the slider and outputs
    //
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      // Calculate where to put the slider
      panel_x_position = (column * horizontal_padding) + margin_left;
      panel_y_position = 7.0;

      // Add slider widget
      DPSliderDisplay *dp_slider_display = new DPSliderDisplay(column);
      dp_slider_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
      dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, SLIDER_HEIGHT));
      dp_slider_display->module = module;
      addChild(dp_slider_display);

      // Add corresponding output
      addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec((column * horizontal_padding) + inputs_margin_left, inputs_vertical_position)), module, column));

    }

    // Add bank lights

    // 1 through 4
    float bank_buttons_x[4] = {150.0, 160.0, 170.0, 180.0};
    float bank_buttons_y[4] = {10.0, 20.0, 30.0, 40.0};
    float x = 0;
    float y = 0;

    for(unsigned int bank_index = 0; bank_index < NUMBER_OF_BANKS; bank_index++)
    {
      x = bank_buttons_x[bank_index % 4];
      y = bank_buttons_y[bank_index / 4];
      addParam(createParamCentered<LEDButton>(mm2px(Vec(x,y)), module, bank_index));
      addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(x, y)), module, bank_index));

      // addParam(createParamCentered<MomentarySwitch<>>(mm2px(Vec(x, y)), module, DigitalProgrammer::BANK_BUTTONS + bank_index));
    }

    /*
    addParam(createParamCentered<LEDButton>(mm2px(Vec(150, 10)), module, DigitalProgrammer::BANK_1_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(150, 10)), module, DigitalProgrammer::BANK_1_LIGHT));

    addParam(createParamCentered<LEDButton>(mm2px(Vec(160, 10)), module, DigitalProgrammer::BANK_2_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(160, 10)), module, DigitalProgrammer::BANK_2_LIGHT));

    addParam(createParamCentered<LEDButton>(mm2px(Vec(170, 10)), module, DigitalProgrammer::BANK_3_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(170, 10)), module, DigitalProgrammer::BANK_3_LIGHT));

    addParam(createParamCentered<LEDButton>(mm2px(Vec(180, 10)), module, DigitalProgrammer::BANK_4_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(180, 10)), module, DigitalProgrammer::BANK_4_LIGHT));

    // 5 through 7

    addParam(createParamCentered<LEDButton>(mm2px(Vec(150, 20)), module, DigitalProgrammer::BANK_5_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(150, 20)), module, DigitalProgrammer::BANK_5_LIGHT));

    addParam(createParamCentered<LEDButton>(mm2px(Vec(160, 20)), module, DigitalProgrammer::BANK_6_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(160, 20)), module, DigitalProgrammer::BANK_6_LIGHT));

    addParam(createParamCentered<LEDButton>(mm2px(Vec(170, 20)), module, DigitalProgrammer::BANK_7_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(170, 20)), module, DigitalProgrammer::BANK_7_LIGHT));

    addParam(createParamCentered<LEDButton>(mm2px(Vec(180, 20)), module, DigitalProgrammer::BANK_8_BUTTON));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(180, 20)), module, DigitalProgrammer::BANK_8_LIGHT));
    */
  }
};
