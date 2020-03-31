struct XYWidget : ModuleWidget
{
  XYWidget(XY* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/xy_front_panel.svg")));

    // Cosmetic rack screws
    // addChild(createWidget<ScrewSilver>(Vec(15, 0)));
    addChild(createWidget<ScrewSilver>(Vec(15, 365)));

    // X,Y outputs
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(75.508, MODULE_HEIGHT_MM - 13.891)), module, XY::X_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(92.294, MODULE_HEIGHT_MM - 13.891)), module, XY::Y_OUTPUT));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.477, MODULE_HEIGHT_MM - 13.891)), module, XY::CLK_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25, MODULE_HEIGHT_MM - 13.891)), module, XY::RESET_INPUT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(39.5, MODULE_HEIGHT_MM - 13.891)), module, XY::RETRIGGER_SWITCH));
    addParam(createParamCentered<CKSS>(mm2px(Vec(54, MODULE_HEIGHT_MM - 13.891)), module, XY::PUNCH_SWITCH));

    // xy mouse entry box
    XYDisplay *xy_display;
    xy_display = new XYDisplay(module);
    xy_display->box.pos = mm2px(Vec(3.4, MODULE_HEIGHT_MM - 30 - DRAW_AREA_HEIGHT_MM + .4));
    addChild(xy_display);
  }

  struct ClicklessOption : MenuItem {
    XY *module;

    void onAction(const event::Action &e) override {
      module->tablet_mode ^= true; // flip the value
    }
  };

  void appendContextMenu(Menu *menu) override
  {
    XY *module = dynamic_cast<XY*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Options"));

    ClicklessOption *clickless_option = createMenuItem<ClicklessOption>("Tablet Mode", CHECKMARK(module->tablet_mode));
    clickless_option->module = module;
    menu->addChild(clickless_option);
  }

};
