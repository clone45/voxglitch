struct OffsetSnapValueItem : MenuItem {
  GrooveBox *module;
  unsigned int offset_snap_index = 0;
  unsigned int track_index = 0;

  void onAction(const event::Action &e) override {
    module->offset_snap_indexes[track_index] = offset_snap_index;
    module->setOffsetSnapIndex(offset_snap_index, track_index);

    // TODO: Set the offset_snap_index in the track, not the module
  }
};

struct OffsetSnapMenuItem : MenuItem {
  GrooveBox *module;
  unsigned int track_index = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    for (unsigned int i=0; i < NUMBER_OF_OFFSET_SNAP_OPTIONS; i++)
    {
      OffsetSnapValueItem *offset_snap_value_item = createMenuItem<OffsetSnapValueItem>(offset_snap_names[i], CHECKMARK(module->offset_snap_indexes[track_index] == i));
      offset_snap_value_item->module = module;
      offset_snap_value_item->offset_snap_index = i;
      offset_snap_value_item->track_index = this->track_index;
      menu->addChild(offset_snap_value_item);
    }

    return menu;
  }
};

struct ClearMenuItem : MenuItem {
  GrooveBox *module;
  int track_index = 0;

  void onAction(const event::Action &e) override {
    module->selected_memory_slot->tracks[track_index].clear();
    module->updateKnobPositions();
  }
};


struct TrackMenuItem : MenuItem {
  GrooveBox *module;
  unsigned int track_index = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    OffsetSnapMenuItem *offset_snap_menu_item = createMenuItem<OffsetSnapMenuItem>("Offset Snap Division", RIGHT_ARROW);
    offset_snap_menu_item->track_index = this->track_index;
    offset_snap_menu_item->module = module;
    menu->addChild(offset_snap_menu_item);

    ClearMenuItem *clear_menu_item = createMenuItem<ClearMenuItem>("Clear Track Data");
    clear_menu_item->track_index = this->track_index;
    clear_menu_item->module = module;
    menu->addChild(clear_menu_item);

    // Randomize??

    /*
    InputSnapItem *input_snap_item = createMenuItem<InputSnapItem>("Snap", RIGHT_ARROW);
    input_snap_item->sequencer_number = this->sequencer_number;
    input_snap_item->module = module;
    menu->addChild(input_snap_item);

    SampleAndHoldItem *sample_and_hold_item = createMenuItem<SampleAndHoldItem>("Sample & Hold", CHECKMARK(module->voltage_sequencers[sequencer_number].sample_and_hold));
    sample_and_hold_item->sequencer_number = this->sequencer_number;
    sample_and_hold_item->module = module;
    menu->addChild(sample_and_hold_item);
    */

    return menu;
  }
};

struct TracksMenu : MenuItem {
  GrooveBox *module;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;
    TrackMenuItem *track_menu_items[NUMBER_OF_TRACKS];

    for(unsigned int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      track_menu_items[i] = createMenuItem<TrackMenuItem>("Track #" + std::to_string(i + 1), RIGHT_ARROW);
      track_menu_items[i]->module = module;
      track_menu_items[i]->track_index = i;
      menu->addChild(track_menu_items[i]);
    }

    return menu;
  }
};
