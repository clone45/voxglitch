struct SequencerItem : MenuItem {
  DigitalSequencer *module;
  unsigned int sequencer_number = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    OutputRangeItem *output_range_item = createMenuItem<OutputRangeItem>("Output Range", RIGHT_ARROW);
    output_range_item->sequencer_number = this->sequencer_number;
    output_range_item->module = module;
    menu->addChild(output_range_item);

    InputSnapItem *input_snap_item = createMenuItem<InputSnapItem>("Snap", RIGHT_ARROW);
    input_snap_item->sequencer_number = this->sequencer_number;
    input_snap_item->module = module;
    menu->addChild(input_snap_item);

    SampleAndHoldItem *sample_and_hold_item = createMenuItem<SampleAndHoldItem>("Sample & Hold", CHECKMARK(module->voltage_sequencers[sequencer_number].sample_and_hold));
    sample_and_hold_item->sequencer_number = this->sequencer_number;
    sample_and_hold_item->module = module;
    menu->addChild(sample_and_hold_item);

    return menu;
  }
};
