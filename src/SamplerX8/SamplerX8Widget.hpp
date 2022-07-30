struct SamplerX8Widget : VoxglitchSamplerModuleWidget
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
    float position_inputs_column_x = 20.66;
    float volume_knobs_column_x = 30.66;
    float pan_knobs_column_x = 40.66;
    float mute_buttons_column_x = 50.66;

    float output_l_column = 64.48;
    float output_r_column = 75.18;

    Vec audio_left_output_positions[NUMBER_OF_SAMPLES] = {
      Vec(output_l_column, group_y + (row_padding * 0)),
      Vec(output_l_column, group_y + (row_padding * 1)),
      Vec(output_l_column, group_y + (row_padding * 2)),
      Vec(output_l_column, group_y + (row_padding * 3)),
      Vec(output_l_column, group_y + (row_padding * 4)),
      Vec(output_l_column, group_y + (row_padding * 5)),
      Vec(output_l_column, group_y + (row_padding * 6)),
      Vec(output_l_column, group_y + (row_padding * 7)),
    };

    Vec audio_right_output_positions[NUMBER_OF_SAMPLES] = {
      Vec(output_r_column, group_y + (row_padding * 0)),
      Vec(output_r_column, group_y + (row_padding * 1)),
      Vec(output_r_column, group_y + (row_padding * 2)),
      Vec(output_r_column, group_y + (row_padding * 3)),
      Vec(output_r_column, group_y + (row_padding * 4)),
      Vec(output_r_column, group_y + (row_padding * 5)),
      Vec(output_r_column, group_y + (row_padding * 6)),
      Vec(output_r_column, group_y + (row_padding * 7))
    };

    for(unsigned int i=0; i < NUMBER_OF_SAMPLES; i++)
    {
      // Add trigger inputs
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(triggers_column_x, group_y + (row_padding * i))), module, SamplerX8::TRIGGER_INPUTS + i));

      // Add position inputs
      addInput(createInputCentered<PJ301MPort>(mm2px(Vec(position_inputs_column_x, group_y + (row_padding * i))), module, SamplerX8::POSITION_INPUTS + i));

      // Add volume knobs
      addParam(createParamCentered<Trimpot>(mm2px(Vec(volume_knobs_column_x, group_y + (row_padding * i))), module, SamplerX8::VOLUME_KNOBS + i));

      // Add pan knobs
      addParam(createParamCentered<Trimpot>(mm2px(Vec(pan_knobs_column_x, group_y + (row_padding * i))), module, SamplerX8::PAN_KNOBS + i));

      // Add Mute buttons and lights
      addParam(createParamCentered<LEDButton>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * i))), module, SamplerX8::MUTE_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(mute_buttons_column_x, group_y + (row_padding * i))), module, SamplerX8::MUTE_BUTTON_LIGHTS + i));

      // Add audio left outputs // Vec(output_l_column, group_y + (row_padding * 0)
      addOutput(createOutputCentered<PJ301MPort>(mm2px(audio_left_output_positions[i]), module, SamplerX8::AUDIO_LEFT_OUTPUTS + i));
      addOutput(createOutputCentered<PJ301MPort>(mm2px(audio_right_output_positions[i]), module, SamplerX8::AUDIO_RIGHT_OUTPUTS + i));
    }

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_l_column, 114.702)), module, SamplerX8::AUDIO_MIX_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(output_r_column, 114.702)), module, SamplerX8::AUDIO_MIX_OUTPUT_RIGHT));
  }

  void appendContextMenu(Menu *menu) override
  {
    SamplerX8 *module = dynamic_cast<SamplerX8*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Load individual samples"));

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

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Or.."));

    SamplerX8LoadFolder *menu_item_load_folder = new SamplerX8LoadFolder();
    menu_item_load_folder->text = "Load first 8 WAV files from a folder";
    menu_item_load_folder->module = module;
    menu->addChild(menu_item_load_folder);

    // Interpolation menu
    menu->addChild(new MenuEntry); // For spacing only
    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
  }
};
