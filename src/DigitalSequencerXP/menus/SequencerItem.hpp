struct SequencerItem : MenuItem {
  DigitalSequencerXP *module;
  unsigned int sequencer_number = 0;

  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    dsxp::OutputRangeItem *output_range_item = createMenuItem<dsxp::OutputRangeItem>("Output Range", RIGHT_ARROW);
    output_range_item->sequencer_number = this->sequencer_number;
    output_range_item->module = module;
    menu->addChild(output_range_item);

    dsxp::InputSnapItem *input_snap_item = createMenuItem<dsxp::InputSnapItem>("Snap", RIGHT_ARROW);
    input_snap_item->sequencer_number = this->sequencer_number;
    input_snap_item->module = module;
    menu->addChild(input_snap_item);

    dsxp::SampleAndHoldItem *sample_and_hold_item = createMenuItem<dsxp::SampleAndHoldItem>("Sample & Hold", CHECKMARK(module->voltage_sequencers[sequencer_number].sample_and_hold));
    sample_and_hold_item->sequencer_number = this->sequencer_number;
    sample_and_hold_item->module = module;
    menu->addChild(sample_and_hold_item);

    // Add label input
    auto holder = new rack::Widget;
    holder->box.size.x = 220; // This value will determine the width of the menu
    holder->box.size.y = 20;

    auto lab = new rack::Label;
    lab->text = "Label: ";
    lab->box.size = 50; // label box size determins the bounding box around #1, #2, #3 etc.
    holder->addChild(lab);

    auto textfield = new dsxp::LabelTextField(sequencer_number);
    textfield->module = module;
    textfield->text = module->labels[sequencer_number];
    holder->addChild(textfield);

    menu->addChild(holder);

    return menu;
  }
};
