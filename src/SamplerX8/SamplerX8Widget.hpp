struct SamplerX8Widget : ModuleWidget
{
  SamplerX8Widget(SamplerX8* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/samplerx8_front_panel.svg")));

    // Cosmetic rack screws
		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));

    float group_y = 30;
    float row_padding = 11.5;

    float triggers_column_x = 9 + .5;
    float column_1_x = 28.58 + .5;
    float column_2_x = 39.28 + .5;

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 0))), module, SamplerX8::TRIGGER_INPUT_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 1))), module, SamplerX8::TRIGGER_INPUT_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 2))), module, SamplerX8::TRIGGER_INPUT_3));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 3))), module, SamplerX8::TRIGGER_INPUT_4));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 4))), module, SamplerX8::TRIGGER_INPUT_5));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 5))), module, SamplerX8::TRIGGER_INPUT_6));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 6))), module, SamplerX8::TRIGGER_INPUT_7));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 7))), module, SamplerX8::TRIGGER_INPUT_8));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 0))), module, SamplerX8::AUDIO_OUTPUT_1_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 0))), module, SamplerX8::AUDIO_OUTPUT_1_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 1))), module, SamplerX8::AUDIO_OUTPUT_2_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 1))), module, SamplerX8::AUDIO_OUTPUT_2_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 2))), module, SamplerX8::AUDIO_OUTPUT_3_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 2))), module, SamplerX8::AUDIO_OUTPUT_3_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 3))), module, SamplerX8::AUDIO_OUTPUT_4_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 3))), module, SamplerX8::AUDIO_OUTPUT_4_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 4))), module, SamplerX8::AUDIO_OUTPUT_5_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 4))), module, SamplerX8::AUDIO_OUTPUT_5_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 5))), module, SamplerX8::AUDIO_OUTPUT_6_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 5))), module, SamplerX8::AUDIO_OUTPUT_6_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 6))), module, SamplerX8::AUDIO_OUTPUT_7_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 6))), module, SamplerX8::AUDIO_OUTPUT_7_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_1_x, group_y + (row_padding * 7))), module, SamplerX8::AUDIO_OUTPUT_8_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(column_2_x, group_y + (row_padding * 7))), module, SamplerX8::AUDIO_OUTPUT_8_RIGHT));
  }


  void appendContextMenu(Menu *menu) override
  {
    SamplerX8 *module = dynamic_cast<SamplerX8*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Samples"));

    //
    // Add the sample slots to the right-click context menu
    //

    for(int i=0; i < NUMBER_OF_SAMPLES; i++)
    {
      SamplerX8LoadSample *menu_item_load_sample = new SamplerX8LoadSample();
      menu_item_load_sample->sample_number = i;
      menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
      menu_item_load_sample->module = module;
      menu->addChild(menu_item_load_sample);
    }
  }
};
