struct AllInputSnapsValueItem : MenuItem {
  DigitalSequencer *module;
  int snap_division_index = 0;

  void onAction(const event::Action &e) override {
    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
    {
      module->voltage_sequencers[i].snap_division_index = snap_division_index;
    }
  }
};
