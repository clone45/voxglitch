#include "OutputRangeValueItem.hpp"

struct OutputRangeItem : MenuItem {
  DigitalSequencer *module;
  int sequencer_number = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    for (unsigned int i=0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
    {
      OutputRangeValueItem *output_range_value_menu_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->voltage_sequencers[sequencer_number].voltage_range_index == i));
      output_range_value_menu_item->module = module;
      output_range_value_menu_item->range_index = i;
      output_range_value_menu_item->sequencer_number = this->sequencer_number;
      menu->addChild(output_range_value_menu_item);
    }

    return menu;
  }
};
