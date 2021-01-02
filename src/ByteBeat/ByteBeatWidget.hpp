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

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 3))), module, ByteBeat::EXPRESSION_INPUT_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 4))), module, ByteBeat::EXPRESSION_INPUT_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 5))), module, ByteBeat::EXPRESSION_INPUT_3));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 0))), module, ByteBeat::AUDIO_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 1))), module, ByteBeat::DEBUG_OUTPUT));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 0))), module, ByteBeat::CLOCK_DIVISION_KNOB));

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
