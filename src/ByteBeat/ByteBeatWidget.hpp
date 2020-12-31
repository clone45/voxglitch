struct ByteBeatWidget : ModuleWidget
{
  ByteBeatWidget(ByteBeat* module)
  {
    float group_y = 30;
    float row_padding = 10.2;

    float triggers_column_x = 9.5;
    float position_inputs_column_x = 10.5 + 10.16;
    float volume_knobs_column_x = 20.5 + 10.16;
    float pan_knobs_column_x = 30.5 + 10.16;

    float output_l_column = 29.08 + 15.24 + 10.16;
    float output_r_column = 39.78 + 15.24 + 10.16;

    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/bytebeat_front_panel.svg")));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 0))), module, ByteBeat::PARAM_INPUT_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 1))), module, ByteBeat::PARAM_INPUT_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 2))), module, ByteBeat::PARAM_INPUT_3));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 0))), module, ByteBeat::AUDIO_OUTPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 0))), module, ByteBeat::CLOCK_DIVISION_KNOB));

    /*
    TextField* textField = createWidget<TextField>(mm2px(Vec(3.39962, 14.8373)));
  	textField->box.size = mm2px(Vec(74.480, 102.753));
  	addChild(textField);
    */
    EquationTextInput *equation_text_input = createWidget<EquationTextInput>(mm2px(Vec(3.39962, 14.8373)));
    equation_text_input->module = module;
    equation_text_input->box.size = mm2px(Vec(74.480, 20.753));
    addChild(equation_text_input);
  }

  struct EquationTextInput : TextField {
  	// EquationDisplay* equationDisplay;
  	Menu* menu;
    ByteBeat *module;

  	void onAction(const event::Action& e) override {
  		TextField::onAction(e);
  		//menu->requestDelete();
      getAncestorOfType<ui::MenuOverlay>()->requestDelete();
  	}
  	void onChange(const event::Change& e) override {
  		TextField::onChange(e);
      module->math_equation = text;
  	}
  };

  /*
  struct TapeNameMenuItem : TextField {
  	TapeNameDisplay* tapeNameDisplay;
  	Menu* menu;

  	void onAction(const event::Action& e) override {
  		TextField::onAction(e);
  		menu->requestDelete();
  	}
  	void onChange(const event::Change& e) override {
  		TextField::onChange(e);
  		tapeNameDisplay->text = text;
  	}
  };
  */
};
