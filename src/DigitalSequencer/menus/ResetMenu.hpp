struct ResetOnNextOption : MenuItem {
  DigitalSequencer *module;

  void onAction(const event::Action &e) override {
    module->legacy_reset = false;
  }
};

struct ResetInstantOption : MenuItem {
  DigitalSequencer *module;

  void onAction(const event::Action &e) override {
    module->legacy_reset = true;
  }
};

struct ResetModeItem : MenuItem {
  DigitalSequencer *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    dseq::ResetOnNextOption *reset_on_next = createMenuItem<dseq::ResetOnNextOption>("Next clock input.", CHECKMARK(! module->legacy_reset));
    reset_on_next->module = module;
    menu->addChild(reset_on_next);

    dseq::ResetInstantOption *reset_instant = createMenuItem<dseq::ResetInstantOption>("Instant", CHECKMARK(module->legacy_reset));
    reset_instant->module = module;
    menu->addChild(reset_instant);

    return menu;
  }
};