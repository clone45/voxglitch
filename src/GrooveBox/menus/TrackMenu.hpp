struct SamplePositionSnapValueItem : MenuItem {
  GrooveBox *module;
  unsigned int sample_position_snap_index = 0;
  unsigned int track_index = 0;

  void onAction(const event::Action &e) override {
    module->sample_position_snap_indexes[track_index] = sample_position_snap_index;
    module->setSamplePositionSnapIndex(sample_position_snap_index, track_index);
  }
};

struct SamplePositionSnapMenuItem : MenuItem {
  GrooveBox *module;
  unsigned int track_index = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    for (unsigned int i=0; i < NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS; i++)
    {
      SamplePositionSnapValueItem *sample_position_snap_value_item = createMenuItem<SamplePositionSnapValueItem>(sample_position_snap_names[i], CHECKMARK(module->sample_position_snap_indexes[track_index] == i));
      sample_position_snap_value_item->module = module;
      sample_position_snap_value_item->sample_position_snap_index = i;
      sample_position_snap_value_item->track_index = this->track_index;
      menu->addChild(sample_position_snap_value_item);
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

    SamplePositionSnapMenuItem *sample_position_snap_menu_item = createMenuItem<SamplePositionSnapMenuItem>("Sample Position Snap Division", RIGHT_ARROW);
    sample_position_snap_menu_item->track_index = this->track_index;
    sample_position_snap_menu_item->module = module;
    menu->addChild(sample_position_snap_menu_item);

    ClearMenuItem *clear_menu_item = createMenuItem<ClearMenuItem>("Clear Track Data");
    clear_menu_item->track_index = this->track_index;
    clear_menu_item->module = module;
    menu->addChild(clear_menu_item);

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
