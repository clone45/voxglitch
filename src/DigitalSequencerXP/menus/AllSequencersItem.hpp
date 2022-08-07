#include "AllOutputRangesItem.hpp"
#include "AllInputSnapsItem.hpp"
#include "AllSampleAndHoldsItem.hpp"

struct AllSequencersItem : MenuItem {
  DigitalSequencerXP *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    dsxp::AllOutputRangesItem *all_output_ranges_item = createMenuItem<dsxp::AllOutputRangesItem>("Output Range", RIGHT_ARROW);
    all_output_ranges_item->module = module;
    menu->addChild(all_output_ranges_item);

    dsxp::AllInputSnapsItem *all_input_snaps_item = createMenuItem<dsxp::AllInputSnapsItem>("Snap", RIGHT_ARROW);
    all_input_snaps_item->module = module;
    menu->addChild(all_input_snaps_item);

    dsxp::AllSampleAndHoldsItem *all_sample_and_holds_items = createMenuItem<dsxp::AllSampleAndHoldsItem>("Sample & Hold", RIGHT_ARROW);
    all_sample_and_holds_items->module = module;
    menu->addChild(all_sample_and_holds_items);

    return menu;
  }
};
