#include "AllOutputRangesValueItem.hpp"

struct AllOutputRangesItem : MenuItem {
  DigitalSequencerXP *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    for (unsigned int i=0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
    {
      dsxp::AllOutputRangesValueItem *all_output_ranges_value_item = createMenuItem<dsxp::AllOutputRangesValueItem>(module->voltage_range_names[i]);
      all_output_ranges_value_item->module = module;
      all_output_ranges_value_item->range_index = i;
      menu->addChild(all_output_ranges_value_item);
    }

    return menu;
  }
};
