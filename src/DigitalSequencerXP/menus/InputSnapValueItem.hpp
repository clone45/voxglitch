struct InputSnapValueItem : MenuItem {
  DigitalSequencerXP *module;
  int snap_division_index = 0;
  int sequencer_number = 0;

  void onAction(const event::Action &e) override {
    module->voltage_sequencers[sequencer_number].snap_division_index = snap_division_index;
  }
};
