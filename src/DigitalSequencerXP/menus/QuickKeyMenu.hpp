struct QuickKeyMenu : MenuItem {
  Menu *createChildMenu() override {
    Menu *menu = new Menu;
    menu->addChild(createMenuLabel("f: Toggle Freeze Mode (for easy editing)"));
    menu->addChild(createMenuLabel("g: When frozen, press 'g' to send gate out"));
    menu->addChild(createMenuLabel("r: Randomize current howevered sequencer"));
    menu->addChild(createMenuLabel("shift-r: Randomize both active sequencers (CV/Gate)"));
    menu->addChild(createMenuLabel("shift+drag : Shift hovered sequence left or right"));
    return menu;
  }
};
