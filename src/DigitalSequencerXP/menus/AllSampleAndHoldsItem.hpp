#include "AllSampleAndHoldsValueItem.hpp"

struct AllSampleAndHoldsItem : MenuItem {
  DigitalSequencerXP *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    dsxp::AllSampleAndHoldsValueItem *sample_and_hold_off = createMenuItem<dsxp::AllSampleAndHoldsValueItem>("Off");
    sample_and_hold_off->module = module;
    sample_and_hold_off->value = false;
    menu->addChild(sample_and_hold_off);

    dsxp::AllSampleAndHoldsValueItem *sample_and_hold_on = createMenuItem<dsxp::AllSampleAndHoldsValueItem>("On");
    sample_and_hold_on->module = module;
    sample_and_hold_on->value = true;
    menu->addChild(sample_and_hold_on);

    return menu;
  }

};
