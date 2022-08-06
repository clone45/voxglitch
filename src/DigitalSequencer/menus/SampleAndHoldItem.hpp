struct SampleAndHoldItem : MenuItem {
  DigitalSequencer *module;
  int sequencer_number = 0;

  void onAction(const event::Action &e) override {
    module->voltage_sequencers[sequencer_number].sample_and_hold ^= true; // flip the value
  }
};
