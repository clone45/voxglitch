#include "AllOutputRangesItem.hpp"
#include "AllInputSnapsItem.hpp"
#include "AllSampleAndHoldsItem.hpp"

struct AllSequencersItem : MenuItem {
  DigitalSequencer *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    AllOutputRangesItem *all_output_ranges_item = createMenuItem<AllOutputRangesItem>("Output Range", RIGHT_ARROW);
    all_output_ranges_item->module = module;
    menu->addChild(all_output_ranges_item);

    AllInputSnapsItem *all_input_snaps_item = createMenuItem<AllInputSnapsItem>("Snap", RIGHT_ARROW);
    all_input_snaps_item->module = module;
    menu->addChild(all_input_snaps_item);

    AllSampleAndHoldsItem *all_sample_and_holds_items = createMenuItem<AllSampleAndHoldsItem>("Sample & Hold", RIGHT_ARROW);
    all_sample_and_holds_items->module = module;
    menu->addChild(all_sample_and_holds_items);

    return menu;
  }
};
