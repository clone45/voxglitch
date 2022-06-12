struct InitializeConfirmMenuItem : MenuItem {
  GrooveBox *module;

  void onAction(const event::Action &e) override {
    module->initialize();
  }
};

struct InitializeMenuItem : MenuItem {
  GrooveBox *module;


  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    menu->addChild(createMenuLabel("This will clear ALL of the module's data.  Are you sure??"));

    InitializeConfirmMenuItem *initialize_confirm_menu_item = createMenuItem<InitializeConfirmMenuItem>("YES");
    initialize_confirm_menu_item->module = module;
    menu->addChild(initialize_confirm_menu_item);
    return menu;
  }
};
