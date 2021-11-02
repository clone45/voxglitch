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
    float row_padding = 10.2;

    float triggers_column_x = 9.5;
    float position_inputs_column_x = 10.5 + 10.16;
    float volume_knobs_column_x = 20.5 + 10.16;
    float pan_knobs_column_x = 30.5 + 10.16;
    float mute_buttons_column_x = 40.5 + 10.16;

    float output_l_column = 29.08 + 15.24 + 10.16 + 10.0;
    float output_r_column = 39.78 + 15.24 + 10.16 + 10.0;

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 0))), module, SamplerX8::TRIGGER_INPUT_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 1))), module, SamplerX8::TRIGGER_INPUT_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 2))), module, SamplerX8::TRIGGER_INPUT_3));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 3))), module, SamplerX8::TRIGGER_INPUT_4));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 4))), module, SamplerX8::TRIGGER_INPUT_5));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 5))), module, SamplerX8::TRIGGER_INPUT_6));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 6))), module, SamplerX8::TRIGGER_INPUT_7));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * 7))), module, SamplerX8::TRIGGER_INPUT_8));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 0))), module, SamplerX8::POSITION_INPUT_1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 1))), module, SamplerX8::POSITION_INPUT_2));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 2))), module, SamplerX8::POSITION_INPUT_3));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 3))), module, SamplerX8::POSITION_INPUT_4));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 4))), module, SamplerX8::POSITION_INPUT_5));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 5))), module, SamplerX8::POSITION_INPUT_6));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 6))), module, SamplerX8::POSITION_INPUT_7));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * 7))), module, SamplerX8::POSITION_INPUT_8));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 0))), module, SamplerX8::VOLUME_KNOB_1));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 1))), module, SamplerX8::VOLUME_KNOB_2));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 2))), module, SamplerX8::VOLUME_KNOB_3));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 3))), module, SamplerX8::VOLUME_KNOB_4));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 4))), module, SamplerX8::VOLUME_KNOB_5));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 5))), module, SamplerX8::VOLUME_KNOB_6));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 6))), module, SamplerX8::VOLUME_KNOB_7));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * 7))), module, SamplerX8::VOLUME_KNOB_8));

    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 0))), module, SamplerX8::PAN_KNOB_1));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 1))), module, SamplerX8::PAN_KNOB_2));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 2))), module, SamplerX8::PAN_KNOB_3));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 3))), module, SamplerX8::PAN_KNOB_4));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 4))), module, SamplerX8::PAN_KNOB_5));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 5))), module, SamplerX8::PAN_KNOB_6));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 6))), module, SamplerX8::PAN_KNOB_7));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * 7))), module, SamplerX8::PAN_KNOB_8));

    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 0))), module, SamplerX8::MUTE_BUTTON_1));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 0))), module, SamplerX8::MUTE_BUTTON_1_LIGHT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 1))), module, SamplerX8::MUTE_BUTTON_2));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 1))), module, SamplerX8::MUTE_BUTTON_2_LIGHT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 2))), module, SamplerX8::MUTE_BUTTON_3));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 2))), module, SamplerX8::MUTE_BUTTON_3_LIGHT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 3))), module, SamplerX8::MUTE_BUTTON_4));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 3))), module, SamplerX8::MUTE_BUTTON_4_LIGHT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 4))), module, SamplerX8::MUTE_BUTTON_5));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 4))), module, SamplerX8::MUTE_BUTTON_5_LIGHT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 5))), module, SamplerX8::MUTE_BUTTON_6));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 5))), module, SamplerX8::MUTE_BUTTON_6_LIGHT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 6))), module, SamplerX8::MUTE_BUTTON_7));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 6))), module, SamplerX8::MUTE_BUTTON_7_LIGHT));
    addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 7))), module, SamplerX8::MUTE_BUTTON_8));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * 7))), module, SamplerX8::MUTE_BUTTON_8_LIGHT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 0))), module, SamplerX8::AUDIO_OUTPUT_1_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 0))), module, SamplerX8::AUDIO_OUTPUT_1_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 1))), module, SamplerX8::AUDIO_OUTPUT_2_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 1))), module, SamplerX8::AUDIO_OUTPUT_2_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 2))), module, SamplerX8::AUDIO_OUTPUT_3_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 2))), module, SamplerX8::AUDIO_OUTPUT_3_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 3))), module, SamplerX8::AUDIO_OUTPUT_4_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 3))), module, SamplerX8::AUDIO_OUTPUT_4_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 4))), module, SamplerX8::AUDIO_OUTPUT_5_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 4))), module, SamplerX8::AUDIO_OUTPUT_5_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 5))), module, SamplerX8::AUDIO_OUTPUT_6_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 5))), module, SamplerX8::AUDIO_OUTPUT_6_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 6))), module, SamplerX8::AUDIO_OUTPUT_7_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 6))), module, SamplerX8::AUDIO_OUTPUT_7_RIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, group_y + (row_padding * 7))), module, SamplerX8::AUDIO_OUTPUT_8_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, group_y + (row_padding * 7))), module, SamplerX8::AUDIO_OUTPUT_8_RIGHT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, 114.702)), module, SamplerX8::AUDIO_MIX_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, 114.702)), module, SamplerX8::AUDIO_MIX_OUTPUT_RIGHT));
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
