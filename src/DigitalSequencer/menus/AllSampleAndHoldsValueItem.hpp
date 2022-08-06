struct AllSampleAndHoldsValueItem : MenuItem {
  DigitalSequencer *module;
  bool value = false;

  void onAction(const event::Action &e) override {
    for(unsigned int i=0; i<NUMBER_OF_SEQUENCERS; i++)
    {
      module->voltage_sequencers[i].sample_and_hold = value;
    }
  }
};
