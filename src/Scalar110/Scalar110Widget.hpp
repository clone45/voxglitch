#include <componentlibrary.hpp>

struct TrackLabelDisplay : TransparentWidget
{
  Scalar110 *module;
  unsigned int track_number = 0;

  TrackLabelDisplay(unsigned int track_number)
  {
    this->track_number = track_number;
    box.size = Vec(5, 5);
  }

  void draw(const DrawArgs& args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      std::string to_display = module->loaded_filenames[track_number];

      if(to_display != "")
      {
        nvgFontSize(args.vg, 10);
        nvgTextLetterSpacing(args.vg, 0);
        nvgFillColor(args.vg, nvgRGBA(0, 0, 0, 0xff));
        nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
        // float x_position = LABEL_POSITIONS[track_number][0];
        // float y_position = LABEL_POSITIONS[track_number][1];
        float wrap_at = 80.0; // Just throw your hands in the air!  And wave them like you just don't
        nvgTextBox(args.vg, 0, 0, wrap_at, to_display.c_str(), NULL);
      }
    }
    nvgRestore(vg);
  }
};


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
            module->loaded_filenames[i] = module->tracks[i].sample_player.getFilename();
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
      module->loaded_filenames[sample_number] = module->tracks[sample_number].sample_player.getFilename();
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

    addInput(createInputCentered<PJ301MPort>(Vec(39.007812,83), module, Scalar110::STEP_INPUT));

    float button_positions[16][2] = {
      { 38.007812,265.011719 },
      { 74.007812,265.011719},
      { 115.007812,265.011719},
      { 153.007812,265.011719},
      { 191.007812, 265.011719},
      { 230.007812,265.011719},
      { 268.007812, 265.011719},
      { 305.007812, 265.011719},
      { 345.007812, 265.011719},
      { 383.007812,265.011719 },
      { 421.007812,265.011719 },
      { 459.007812,265.011719 },
      { 498.007812,265.011719 },
      { 537.007812,265.011719 },
      { 575.007812,264.011719 },
      { 612.007812,265.011719 }
    };

    float function_button_positions[NUMBER_OF_FUNCTIONS][2] = {
      {22.007812,348.011719},
      {99.007812,348.011719},
      {177.007812,350.011719},
      {253.007812,348.011719},
      {331.007812,349.011719},
      {409.007812,349.011719},
      {485.007812,347.011719},
      {562.007812,347.011719}
    };

    float track_button_positions[NUMBER_OF_TRACKS][2] = {
      {380.007812,83},
      {380.007812,114.5},
      {379.007812,145.011719},
      {380.007812,177.011719},
      {507.007812,83},
      {507.007812,114.5},
      {507.007812,145.011719},
      {507.007812,177.011719}
    };

    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      addParam(createLightParamCentered<VCVLightBezel<>>(Vec(button_positions[i][0],button_positions[i][1]), module, Scalar110::DRUM_PADS + i, Scalar110::DRUM_PAD_LIGHTS + i));
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(button_positions[i][0],button_positions[i][1] - 26), module, Scalar110::STEP_LOCATION_LIGHTS + i));

      // Create attenuator knobs for each step
      addParam(createParamCentered<Trimpot>(Vec(button_positions[i][0],button_positions[i][1] + 30), module, Scalar110::STEP_KNOBS + i));
    }

    // Function Buttons
    for(unsigned int i=0; i<NUMBER_OF_FUNCTIONS; i++)
    {
      float x = function_button_positions[i][0];
      float y = function_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x, y), module, Scalar110::FUNCTION_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x, y), module, Scalar110::FUNCTION_BUTTON_LIGHTS + i));
    }

    // addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(50,50)), module, Scalar110::TRACK_SELECT_KNOB));
    // addParam(createParamCentered<EngineKnob>(mm2px(Vec(50,80)), module, Scalar110::ENGINE_SELECT_KNOB));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(200, 10)), module, Scalar110::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(210, 10)), module, Scalar110::AUDIO_OUTPUT_RIGHT));

    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      float x = track_button_positions[i][0];
      float y = track_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x,y), module, Scalar110::TRACK_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x,y), module, Scalar110::TRACK_BUTTON_LIGHTS + i));

      TrackLabelDisplay *track_label_display = new TrackLabelDisplay(i);
      track_label_display->setPosition(Vec(LABEL_POSITIONS[i][0], LABEL_POSITIONS[i][1]));
      track_label_display->setSize(Vec(200, 50)); // Need to adjust this
      track_label_display->module = module;
      addChild(track_label_display);
    }
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    if(e.action == GLFW_PRESS && e.key == GLFW_KEY_P)
    {
      std::string debug_string = "mouse at: " + std::to_string(e.pos.x) + "," + std::to_string(e.pos.y);
      DEBUG(debug_string.c_str());
    }
    ModuleWidget::onHoverKey(e);
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
