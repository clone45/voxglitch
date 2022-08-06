struct QuickKeyMenu : MenuItem {
  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    menu->addChild(createMenuLabel("      f : Toggle Freeze Mode (for easy editing)"));
    menu->addChild(createMenuLabel("      g : When frozen, press 'g' to send gate out"));
    menu->addChild(createMenuLabel(""));
    menu->addChild(createMenuLabel("      r : Randomize gate or voltage sequence"));
    menu->addChild(createMenuLabel("      ↑ : Nudge up voltage for hovered step"));
    menu->addChild(createMenuLabel("      ↓ : Nudge down voltage for hovered step"));
    menu->addChild(createMenuLabel("      → : Shift hovered sequence to the right"));
    menu->addChild(createMenuLabel("      ← : Shift hovered sequence to the left"));
    menu->addChild(createMenuLabel("    1-6 : Quickly select active sequencer"));
    menu->addChild(createMenuLabel("ctrl-c  : copy selected sequence"));
    menu->addChild(createMenuLabel("ctrl-v  : paste selected sequence"));

    return menu;
  }
};
