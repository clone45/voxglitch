struct ResetOnNextOption : MenuItem {
  DigitalSequencerXP *module;

  void onAction(const event::Action &e) override {
    module->legacy_reset = false;
  }
};

struct ResetInstantOption : MenuItem {
  DigitalSequencerXP *module;

  void onAction(const event::Action &e) override {
    module->legacy_reset = true;
  }
};

struct ResetModeItem : MenuItem {
  DigitalSequencerXP *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    dsxp::ResetOnNextOption *reset_on_next = createMenuItem<dsxp::ResetOnNextOption>("Next clock input.", CHECKMARK(! module->legacy_reset));
    reset_on_next->module = module;
    menu->addChild(reset_on_next);

    dsxp::ResetInstantOption *reset_instant = createMenuItem<dsxp::ResetInstantOption>("Instant", CHECKMARK(module->legacy_reset));
    reset_instant->module = module;
    menu->addChild(reset_instant);

    return menu;
  }
};
