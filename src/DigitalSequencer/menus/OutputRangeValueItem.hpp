struct OutputRangeValueItem : MenuItem {
  DigitalSequencer *module;
  int range_index = 0;
  int sequencer_number = 0;

  void onAction(const event::Action &e) override {
    module->voltage_sequencers[sequencer_number].voltage_range_index = range_index;
  }
};
