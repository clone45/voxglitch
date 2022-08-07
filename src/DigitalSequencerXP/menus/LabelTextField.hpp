struct LabelTextField : TextField {

  DigitalSequencerXP *module;
  unsigned int index = 0;

  LabelTextField(unsigned int index)
  {
    this->index = index;
    this->box.pos.x = 50;
    this->box.size.x = 160;
    this->multiline = false;
  }

  void onChange(const event::Change& e) override {
    module->labels[index] = text;
  }

};
