#include "AllSampleAndHoldsValueItem.hpp"

struct AllSampleAndHoldsItem : MenuItem {
  DigitalSequencer *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    AllSampleAndHoldsValueItem *sample_and_hold_off = createMenuItem<AllSampleAndHoldsValueItem>("Off");
    sample_and_hold_off->module = module;
    sample_and_hold_off->value = false;
    menu->addChild(sample_and_hold_off);

    AllSampleAndHoldsValueItem *sample_and_hold_on = createMenuItem<AllSampleAndHoldsValueItem>("On");
    sample_and_hold_on->module = module;
    sample_and_hold_on->value = true;
    menu->addChild(sample_and_hold_on);

    return menu;
  }

};
