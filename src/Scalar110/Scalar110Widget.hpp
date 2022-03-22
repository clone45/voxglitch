#include <componentlibrary.hpp>

struct LoadSamplesFromFolderMenuItem : MenuItem
{
	Scalar110 *module;
	unsigned int sample_number = 0;
  std::string root_dir;

	void onAction(const event::Action &e) override
	{
		const std::string dir = root_dir.empty() ? "" : root_dir;
    char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);

		if (path)
		{
      root_dir = std::string(path);

      std::vector<std::string> dirList = system::getEntries(path);

      unsigned int i = 0;

  		for (auto entry : dirList)
  		{
  			if (
          // Something happened in Rack 2 where the extension started to include
          // the ".", so I decided to check for both versions, just in case.
          (rack::string::lowercase(system::getExtension(entry)) == "wav") ||
          (rack::string::lowercase(system::getExtension(entry)) == ".wav")
        )
  			{
          if(i < 6)
          {
            module->tracks[i].sample_player.loadSample(std::string(entry));
            // loaded_filename = module->tracks[i].sampler_player..getFilename();
            i++;
          }
  			}
  		}
		}

    free(path);
	}
};

struct LoadSampleMenuItem : MenuItem
{
	Scalar110 *module;
	unsigned int sample_number = 0;
  std::string root_dir;

	void onAction(const event::Action &e) override
	{
		const std::string dir = root_dir.empty() ? "" : root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("WAV:wav:Wav"));

		if (path)
		{
      root_dir = std::string(path);
			module->tracks[sample_number].sample_player.loadSample(std::string(path));
      // module->loaded_filenames[sample_number] = module->sample_players[sample_number].getFilename();
			free(path);
		}
	}
};

struct Scalar110Widget : ModuleWidget
{
  Scalar110Widget(Scalar110* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/scalar110_front_panel.svg")));

    float button_group_x  = 10.0;
    float button_group_y  = 100.0;
    float button_spacing  = 10.0;

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 40)), module, Scalar110::STEP_INPUT));

    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      addParam(createParamCentered<LEDButton>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y + 10)), module, Scalar110::STEP_SELECT_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y + 10)), module, Scalar110::STEP_SELECT_BUTTON_LIGHTS + i));

      addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y)), module, Scalar110::DRUM_PADS + i, Scalar110::DRUM_PAD_LIGHTS + i));
      addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(button_group_x + (button_spacing * i), button_group_y - 6)), module, Scalar110::STEP_LOCATION_LIGHTS + i));
    }

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(50,50)), module, Scalar110::TRACK_SELECT_KNOB));
    // addParam(createParamCentered<EngineKnob>(mm2px(Vec(50,80)), module, Scalar110::ENGINE_SELECT_KNOB));


    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(120 + (0 * 20),50)), module, Scalar110::SAMPLE_OFFSET_KNOB));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(120 + (1 * 20),50)), module, Scalar110::SAMPLE_VOLUME_KNOB));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(120 + (2 * 20),50)), module, Scalar110::SAMPLE_PITCH_KNOB));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(120 + (3 * 20),50)), module, Scalar110::SAMPLE_PAN_KNOB));

    /*
    for(unsigned int i=0; i<4; i++)
    {
      ParameterKnob* parameter_knob = createParamCentered<ParameterKnob>(mm2px(Vec(offset + (i * 20),50)), module, Scalar110::STEP_PARAMS + i);
  		parameter_knob->parameter_number = i;
  		addParam(parameter_knob);
    }

    for(unsigned int i=4; i<8; i++)
    {
      ParameterKnob* parameter_knob = createParamCentered<ParameterKnob>(mm2px(Vec(offset + ((i-4) * 20),70)), module, Scalar110::STEP_PARAMS + i);
  		parameter_knob->parameter_number = i;
  		addParam(parameter_knob);
    }
    */

    /*
    ParameterKnob* parameter_knob_1 = createParamCentered<ParameterKnob>(mm2px(Vec(offset + 20,50)), module, Scalar110::ENGINE_PARAMS + 1);
		parameter_knob_1->parameter_number = 1;
		addParam(parameter_knob_1);

    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset,50)), module, Scalar110::ENGINE_PARAMS));
    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset + 20,50)), module, Scalar110::ENGINE_PARAMS + 1));
    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset + 40,50)), module, Scalar110::ENGINE_PARAMS + 2));
    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset + 60,50)), module, Scalar110::ENGINE_PARAMS + 3));
    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset,70)), module, Scalar110::ENGINE_PARAMS + 4));
    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset + 20,70)), module, Scalar110::ENGINE_PARAMS + 5));
    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset + 40,70)), module, Scalar110::ENGINE_PARAMS + 6));
    addParam(createParamCentered<ParameterKnob>(mm2px(Vec(offset + 60,70)), module, Scalar110::ENGINE_PARAMS + 7));
    */
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(210, 114.702)), module, Scalar110::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(220, 114.702)), module, Scalar110::AUDIO_OUTPUT_RIGHT));


    /*
      LCDWidget *lcd_widget = new LCDWidget(module);
      // lcd_widget->module = module;
      lcd_widget->box.pos = mm2px(Vec(LCD_DISPLAY_X, LCD_DISPLAY_Y));
      addChild(lcd_widget);
      */

      /*
    FileSelectWidget *file_select_widget = new FileSelectWidget();
    file_select_widget->module = module;
    file_select_widget->box.pos = mm2px(Vec(LCD_DISPLAY_X + 2, LCD_DISPLAY_Y + 5));
    addChild(file_select_widget);
    */

  }

  void appendContextMenu(Menu *menu) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Load individual samples"));


    //
    // Add the sample slots to the right-click context menu
    //

    for(int i=0; i < NUMBER_OF_TRACKS; i++)
    {
      LoadSampleMenuItem *menu_item_load_sample = new LoadSampleMenuItem();
      menu_item_load_sample->sample_number = i;
      menu_item_load_sample->text = std::to_string(i+1) + ": " + module->tracks[i].sample_player.getFilename();
      menu_item_load_sample->module = module;
      menu->addChild(menu_item_load_sample);
    }

    // Add spacer
    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Or.."));

    // Add menu item for loading samples from a folder
    LoadSamplesFromFolderMenuItem *menu_item_load_folder = new LoadSamplesFromFolderMenuItem();
    menu_item_load_folder->text = "Load first 8 WAV files from a folder";
    menu_item_load_folder->module = module;
    menu->addChild(menu_item_load_folder);
  }

};
