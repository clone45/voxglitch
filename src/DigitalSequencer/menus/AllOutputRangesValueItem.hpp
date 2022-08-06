struct AllOutputRangesValueItem : MenuItem {
  DigitalSequencer *module;
  int range_index = 0;

  void onAction(const event::Action &e) override {
    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
    {
      module->voltage_sequencers[i].voltage_range_index = range_index;
    }
  }
};
