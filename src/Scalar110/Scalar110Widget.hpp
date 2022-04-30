#include <componentlibrary.hpp>

float button_positions[16][2] = {
  { 29.5, 265.0 },
  { 69, 265.0},
  { 109, 265.0},
  { 149, 265.0},
  { 189, 265.0},
  { 229, 265.0},
  { 269, 265.0},
  { 309, 265.0},
  { 350, 265.0},
  { 390, 265.0 },
  { 430, 265.0 },
  { 470, 265.0 },
  { 510, 265.0 },
  { 550, 265.0 },
  { 590, 265.0 },
  { 631, 265.0 }
};

struct SequenceLengthWidget : TransparentWidget
{
  Scalar110 *module;

  SequenceLengthWidget()
  {
    // box.size = Vec(DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      // Grey background
      nvgBeginPath(vg);
      nvgRoundedRect(vg, 0, 0, button_positions[module->selected_track->length][0] - 10, 12, 5);
      nvgFillColor(vg, nvgRGB(84, 84, 84));
      nvgFill(vg);
    }
    // Paint static content for library display
    else
    {

    }

    nvgRestore(vg);
  }

};

struct TrackLabelDisplay : TransparentWidget
{
  Scalar110 *module;
  unsigned int track_number = 0;

  TrackLabelDisplay(unsigned int track_number)
  {
    this->track_number = track_number;
    box.size = Vec(152, 29);
  }

  void onDoubleClick(const event::DoubleClick &e) override
  {
    std::string root_dir = module->root_directory;
    const std::string dir = root_dir.empty() ? "" : root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("WAV:wav:Wav"));

		if (path)
		{
      root_dir = std::string(path);
			module->sample_players[track_number].loadSample(std::string(path));
      module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
			free(path);
		}
  }

  void onButton(const event::Button &e) override
  {
    TransparentWidget::onButton(e);
    e.consume(this);
  }


  void draw(const DrawArgs& args) override
  {
    const auto vg = args.vg;

    // Debugging code for draw area, which often has to be set experimentally
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(20, 20, 20, 255));
    nvgFill(vg);


    // Save the drawing context to restore later
    nvgSave(vg);

    if(module)
    {
      std::string to_display = module->loaded_filenames[track_number];

      if((to_display != "") && (to_display != "[ empty ]"))
      {
        float text_left_margin = 6;

        nvgFontSize(args.vg, 10);
        nvgTextLetterSpacing(args.vg, 0);
        nvgFillColor(args.vg, nvgRGBA(255, 215, 20, 0xff));
        nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        float wrap_at = 130.0; // Just throw your hands in the air!  And wave them like you just don't

        float bounds[4];
        nvgTextBoxBounds(vg, text_left_margin, 10, wrap_at, to_display.c_str(), NULL, bounds);

        // float textX = bounds[0];
        // float textY = bounds[1];
        // float textWidth = bounds[2];
        float textHeight = bounds[3];

        nvgTextBox(args.vg, text_left_margin, (box.size.y / 2.0f) - (textHeight / 2.0f) + 8, wrap_at, to_display.c_str(), NULL);
      }
    }
    nvgRestore(vg);
  }
};


struct LoadSamplesFromFolderMenuItem : MenuItem
{

	Scalar110 *module;
	unsigned int track_number = 0;


	void onAction(const event::Action &e) override
	{
    std::string root_dir = module->root_directory;;
		const std::string dir = root_dir.empty() ? "" : root_dir;
    char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);

		if (path)
		{
      module->root_directory = std::string(path);

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
          if(i < NUMBER_OF_TRACKS)
          {
            module->sample_players[i].loadSample(std::string(entry));
            module->loaded_filenames[i] = module->sample_players[i].getFilename();
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
	unsigned int track_number = 0;

	void onAction(const event::Action &e) override
	{
    std::string root_dir = module->root_directory;

		const std::string dir = root_dir.empty() ? "" : root_dir;
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("WAV:wav:Wav"));

		if (path)
		{
      module->root_directory = std::string(path);
			module->sample_players[track_number].loadSample(std::string(path));
      module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
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

    addInput(createInputCentered<PJ301MPort>(Vec(39.4, 98), module, Scalar110::STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(39.4, 146), module, Scalar110::RESET_INPUT));

    float function_button_positions[NUMBER_OF_FUNCTIONS][2] = {
      {18.8, 349},
      {98, 349},
      {177, 349},
      {256, 349},
      {335, 349},
      {414, 349},
      {493, 349},
      {573, 349}
    };

    float col_1 = 265;
    float col_2 = 461.4;

    float row_1 = 90.5;
    float row_2 = 121.750000;
    float row_3 = 152.683594;
    float row_4 = 184;

    float track_button_positions[NUMBER_OF_TRACKS][2] = {
      {col_1, row_1},
      {col_1, row_2},
      {col_1, row_3},
      {col_1, row_4},
      {col_2, row_1},
      {col_2, row_2},
      {col_2, row_3},
      {col_2, row_4}
    };

    //
    // sequence length indicator
    //
    SequenceLengthWidget *sequence_length_widget = new SequenceLengthWidget();
    sequence_length_widget->setPosition(Vec(button_positions[0][0] - 10, button_positions[0][1] - 31));
    sequence_length_widget->module = module;
    addChild(sequence_length_widget);

    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      //
      // Drum pad lights
      //
      addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(Vec(button_positions[i][0],button_positions[i][1]), module, Scalar110::DRUM_PADS + i, Scalar110::DRUM_PAD_LIGHTS + i));

      //
      // Step location indicators
      //
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(button_positions[i][0],button_positions[i][1] - 25), module, Scalar110::STEP_LOCATION_LIGHTS + i));

      //
      // Create attenuator knobs for each step
      //
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

    //
    // Track buttons
    //
    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      float x = track_button_positions[i][0];
      float y = track_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x,y), module, Scalar110::TRACK_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x,y), module, Scalar110::TRACK_BUTTON_LIGHTS + i));

      TrackLabelDisplay *track_label_display = new TrackLabelDisplay(i);
      track_label_display->setPosition(Vec(x + 16, y - 14));
      track_label_display->module = module;
      addChild(track_label_display);
    }

    // Track outputs
    for(unsigned int i=0; i<(NUMBER_OF_TRACKS * 2); i++)
    {
      addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10 + (i * 11), 6)), module, Scalar110::TRACK_OUTPUTS + i));
    }

    // Mix output
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(203, 6)), module, Scalar110::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(213, 6)), module, Scalar110::AUDIO_OUTPUT_RIGHT));

    //
    // Memory buttons
    //

    addInput(createInputCentered<PJ301MPort>(Vec(85.255859,124.761719), module, Scalar110::MEM_INPUT));

    float memory_slot_button_positions[NUMBER_OF_MEMORY_SLOTS][2] = {
      {125, 93},
      {155, 93},
      {185, 93},
      {215, 93},

      {125, 124.5},
      {155, 124.5},
      {185, 124.5},
      {215, 124.5},

      {125, 155},
      {155, 155},
      {185, 155},
      {215, 155},

      {125, 187},
      {155, 187},
      {185, 187},
      {215, 187}
    };

    for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++)
    {
      float x = memory_slot_button_positions[i][0];
      float y = memory_slot_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x,y), module, Scalar110::MEMORY_SLOT_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x,y), module, Scalar110::MEMORY_SLOT_BUTTON_LIGHTS + i));
    }
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    Scalar110 *module = dynamic_cast<Scalar110*>(this->module);
    assert(module);

    if(e.action == GLFW_PRESS && e.key == GLFW_KEY_P)
    {
      std::string debug_string = "mouse at: " + std::to_string(e.pos.x) + "," + std::to_string(e.pos.y);
      DEBUG(debug_string.c_str());
    }

    if((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
    {
      module->shift_key = true;
    }
    else
    {
      module->shift_key = false;
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
      menu_item_load_sample->track_number = i;
      menu_item_load_sample->text = std::to_string(i+1) + ": " + module->sample_players[i].getFilename();
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
