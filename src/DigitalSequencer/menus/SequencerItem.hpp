struct SequencerItem : MenuItem {
  DigitalSequencer *module;
  unsigned int sequencer_number = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    dseq::OutputRangeItem *output_range_item = createMenuItem<dseq::OutputRangeItem>("Output Range", RIGHT_ARROW);
    output_range_item->sequencer_number = this->sequencer_number;
    output_range_item->module = module;
    menu->addChild(output_range_item);

    dseq::InputSnapItem *input_snap_item = createMenuItem<dseq::InputSnapItem>("Snap", RIGHT_ARROW);
    input_snap_item->sequencer_number = this->sequencer_number;
    input_snap_item->module = module;
    menu->addChild(input_snap_item);

    dseq::SampleAndHoldItem *sample_and_hold_item = createMenuItem<dseq::SampleAndHoldItem>("Sample & Hold", CHECKMARK(module->voltage_sequencers[sequencer_number].sample_and_hold));
    sample_and_hold_item->sequencer_number = this->sequencer_number;
    sample_and_hold_item->module = module;
    menu->addChild(sample_and_hold_item);

    return menu;
  }
};
