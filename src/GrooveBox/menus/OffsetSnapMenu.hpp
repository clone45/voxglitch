/*
struct OffsetSnapValueItem : MenuItem {
  GrooveBox *module;
  unsigned int offset_snap_index = 0;

  void onAction(const event::Action &e) override {
    // module->offset_snap_index = offset_snap_index;
    module->setOffsetSnapIndex(offset_snap_index);
  }
};

struct OffsetSnapMenu : MenuItem {
  GrooveBox *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    for (unsigned int i=0; i < NUMBER_OF_OFFSET_SNAP_OPTIONS; i++)
    {
      OffsetSnapValueItem *offset_snap_value_item = createMenuItem<OffsetSnapValueItem>(offset_snap_names[i], CHECKMARK(module->offset_snap_index == i));
      offset_snap_value_item->module = module;
      offset_snap_value_item->offset_snap_index = i;
      menu->addChild(offset_snap_value_item);
    }

    return menu;
  }
};
*/
