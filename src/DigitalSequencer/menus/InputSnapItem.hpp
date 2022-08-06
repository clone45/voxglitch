#include "InputSnapValueItem.hpp"

struct InputSnapItem : MenuItem {
  DigitalSequencer *module;
  int sequencer_number = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    for (unsigned int i=0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
    {
      InputSnapValueItem *input_snap_value_item = createMenuItem<InputSnapValueItem>(module->snap_division_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].snap_division_index == i));
      input_snap_value_item->module = module;
      input_snap_value_item->snap_division_index = i;
      input_snap_value_item->sequencer_number = this->sequencer_number;
      menu->addChild(input_snap_value_item);
    }

    return menu;
  }
};
