//
// mm2px 2.952756

#include <componentlibrary.hpp>

float button_positions_y = mm2px(89.75);

float button_positions[16][2] = {
  { mm2px(9.941), button_positions_y },
  { mm2px(23.52), button_positions_y},
  { mm2px(37.10), button_positions_y},
  { mm2px(50.69), button_positions_y },
  { mm2px(64.27), button_positions_y},
  { mm2px(77.85), button_positions_y},
  { mm2px(91.43), button_positions_y},
  { mm2px(105.02), button_positions_y},
  { mm2px(118.60), button_positions_y},
  { mm2px(132.18), button_positions_y},
  { mm2px(145.76), button_positions_y},
  { mm2px(159.35), button_positions_y},
  { mm2px(172.93), button_positions_y},
  { mm2px(186.51), button_positions_y},
  { mm2px(200.09), button_positions_y},
  { mm2px(213.67), button_positions_y}
};

float memory_slot_button_positions[NUMBER_OF_MEMORY_SLOTS][2] = {
  {125, 93},
  {155, 93},
  {185, 93},
  {215, 93},

  {125, 124.33},
  {155, 124.33},
  {185, 124.33},
  {215, 124.33},

  {125, 155.664},
  {155, 155.664},
  {185, 155.664},
  {215, 155.664},

  {125, 187},
  {155, 187},
  {185, 187},
  {215, 187}
};

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

float track_button_positions[NUMBER_OF_TRACKS][2] = {
  {265, 93},
  {265, 124.33},
  {265, 155.664},
  {265, 187},
  {461.4, 93},
  {461.4, 124.33},
  {461.4, 155.664},
  {461.4, 187}
};

struct ModdedCL1362 : SvgPort {
	ModdedCL1362() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/modded_CL1362.svg")));
	}
};

struct TrimpotMedium : SVGKnob {
  widget::SvgWidget* bg;
  TrimpotMedium()
  {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;

    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/TrimpotMedium.svg")));
    bg = new widget::SvgWidget;
    bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/TrimpotMedium_bg.svg")));
    fb->addChildBelow(bg, tw);
  }
};

//
// SequenceLengthWidget
//
// This is the grey horizontal bar that shows the sequence length of the
// selected track.

struct SequenceLengthWidget : TransparentWidget
{
  GrooveBox *module;

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    nvgSave(vg);
    nvgBeginPath(vg);

    if(module) {
      // Draw horizontal rectangle for track indictor with pretty rounded corners
      nvgRoundedRect(vg, 0, 0, button_positions[module->selected_track->length][0] - 10, 12, 5);
    }
    else {
      // Paint static content for library display
      nvgRoundedRect(vg, 0, 0, mm2px(186.51), 12, 5);
    }

    nvgFillColor(vg, nvgRGB(84, 84, 84));
    nvgFill(vg);

    nvgRestore(vg);
  }

};

//
// TrackLabelDisplay
//
// Bright orange track name displays that are positioned to
// the right of the track selection buttons
//
struct TrackLabelDisplay : TransparentWidget
{
  GrooveBox *module;
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
#ifdef USING_CARDINAL_NOT_RACK
		GrooveBox *module = this->module;
		unsigned int track_number = this->track_number;
		async_dialog_filebrowser(false, dir.c_str(), NULL, [module, track_number](char* path) {
			pathSelected(module, track_number, path);
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("WAV:wav:Wav"));
		pathSelected(module, track_number, path);
#endif
	}

	static void pathSelected(GrooveBox *module, unsigned int track_number, char *path)
	{
		if (path)
		{
			module->sample_players[track_number].loadSample(std::string(path));
			module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
			free(path);
		}
  }

  /*
  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS)
    {
      module->toggleMute(track_number);
    }

    TransparentWidget::onButton(e);
    e.consume(this);
  }
  */

  void draw_track_label(std::string label, NVGcontext *vg)
  {
    float text_left_margin = 6;

    //
    // Display track label
    //
    nvgFontSize(vg, 10);
    nvgTextLetterSpacing(vg, 0);
    nvgFillColor(vg, nvgRGBA(255, 215, 20, 0xff));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    float wrap_at = 130.0; // Just throw your hands in the air!  And wave them like you just don't

    float bounds[4];
    nvgTextBoxBounds(vg, text_left_margin, 10, wrap_at, label.c_str(), NULL, bounds);

    float textHeight = bounds[3];
    nvgTextBox(vg, text_left_margin, (box.size.y / 2.0f) - (textHeight / 2.0f) + 8, wrap_at, label.c_str(), NULL);
  }

  void draw_track_mute_overlay(NVGcontext *vg)
  {
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 162));
    nvgFill(vg);
  }

  void draw(const DrawArgs& args) override
  {
    const auto vg = args.vg;

    // Save the drawing context to restore later
    nvgSave(vg);

    // Draw dark background
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(20, 20, 20, 255));
    nvgFill(vg);

    //
    // Draw track names
    //
    if(module)
    {
      std::string to_display = module->loaded_filenames[track_number];

      // If the track name is not empty, then display it
      if((to_display != "") && (to_display != "[ empty ]"))
      {
        draw_track_label(to_display, vg);
      }

      // If the track is muted, then display an overlay
<<<<<<< HEAD
      if(module->track_mutes[track_number])
=======
      if(module->mutes[track_number])
>>>>>>> 9cea2850aa58840745ab8853ce449064f935365b
      {
        draw_track_mute_overlay(vg);
      }
    }
    //
    // Draw placeholder track names for library view
    //
    else
    {
      draw_track_label(PLACEHOLDER_TRACK_NAMES[track_number], vg);
    }
    nvgRestore(vg);
  }
};


struct LoadSamplesFromFolderMenuItem : MenuItem
{
	GrooveBox *module;
	unsigned int track_number = 0;

	void onAction(const event::Action &e) override
	{
		std::string root_dir = module->root_directory;;
		const std::string dir = root_dir.empty() ? "" : root_dir;
#ifdef USING_CARDINAL_NOT_RACK
		GrooveBox *module = this->module;
		async_dialog_filebrowser(false, dir.c_str(), NULL, [module](char* path) {
			pathSelected(module, path);
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN_DIR, dir.c_str(), NULL, NULL);
		pathSelected(module, path);
#endif
	}

	static void pathSelected(GrooveBox *module, char *path)
	{
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
	GrooveBox *module;
	unsigned int track_number = 0;

	void onAction(const event::Action &e) override
	{
		std::string root_dir = module->root_directory;
		const std::string dir = root_dir.empty() ? "" : root_dir;

#ifdef USING_CARDINAL_NOT_RACK
		GrooveBox *module = this->module;
		unsigned int track_number = this->track_number;
		async_dialog_filebrowser(false, dir.c_str(), NULL, [module, track_number](char* path) {
			pathSelected(module, track_number, path);
		});
#else
		char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, osdialog_filters_parse("WAV:wav:Wav"));
		pathSelected(module, track_number, path);
#endif
	}

	static void pathSelected(GrooveBox *module, unsigned int track_number, char *path)
	{
		if (path)
		{
      module->root_directory = std::string(path);
			module->sample_players[track_number].loadSample(std::string(path));
      module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
			free(path);
		}
	}
};

struct GrooveBoxWidget : VoxglitchSamplerModuleWidget
{
  GrooveBoxWidget(GrooveBox* module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groove_box_front_panel.svg")));

    addInput(createInputCentered<PJ301MPort>(Vec(39.4, 98), module, GrooveBox::STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(39.4, 159), module, GrooveBox::RESET_INPUT));

    // sequence length indicator
    SequenceLengthWidget *sequence_length_widget = new SequenceLengthWidget();
    sequence_length_widget->setPosition(Vec(button_positions[0][0] - 10, button_positions[0][1] - 31));
    sequence_length_widget->module = module;
    addChild(sequence_length_widget);

    //
    // Step button related stuff
    //
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      //
      // Drum pad lights
      //
      addParam(createLightParamCentered<VCVLightBezel<BlueLight>>(Vec(button_positions[i][0],button_positions[i][1]), module, GrooveBox::DRUM_PADS + i, GrooveBox::DRUM_PAD_LIGHTS + i));

      //
      // Step location indicators
      //
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(button_positions[i][0],button_positions[i][1] - 25), module, GrooveBox::STEP_LOCATION_LIGHTS + i));

      //
      // Create attenuator knobs for each step
      //
      addParam(createParamCentered<TrimpotMedium>(Vec(button_positions[i][0],button_positions[i][1] + 30), module, GrooveBox::STEP_KNOBS + i));
    }

    // Function Buttons
    for(unsigned int i=0; i<NUMBER_OF_FUNCTIONS; i++)
    {
      float x = function_button_positions[i][0];
      float y = function_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x, y), module, GrooveBox::FUNCTION_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x, y), module, GrooveBox::FUNCTION_BUTTON_LIGHTS + i));
    }

    // Track buttons and labels
    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      float x = track_button_positions[i][0];
      float y = track_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x,y), module, GrooveBox::TRACK_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x,y), module, GrooveBox::TRACK_BUTTON_LIGHTS + i));

      TrackLabelDisplay *track_label_display = new TrackLabelDisplay(i);
      track_label_display->setPosition(Vec(x + 16, y - 14));
      track_label_display->module = module;
      addChild(track_label_display);
    }

    // Indivisual track outputs
    for(unsigned int i=0; i<(NUMBER_OF_TRACKS * 2); i++)
    {
      addOutput(createOutputCentered<ModdedCL1362>(mm2px(Vec(10 + (i * 11), 6)), module, GrooveBox::TRACK_OUTPUTS + i));
    }

    // Mix output L/R
    addOutput(createOutputCentered<ModdedCL1362>(mm2px(Vec(203, 6)), module, GrooveBox::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<ModdedCL1362>(mm2px(Vec(213, 6)), module, GrooveBox::AUDIO_OUTPUT_RIGHT));

    // Master volume
    addParam(createParamCentered<Trimpot>(mm2px(Vec(188.8465, 6)), module, GrooveBox::MASTER_VOLUME));

    //
    // Memory buttons
    //
    for(unsigned int i=0; i<NUMBER_OF_MEMORY_SLOTS; i++)
    {
      float x = memory_slot_button_positions[i][0];
      float y = memory_slot_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x,y), module, GrooveBox::MEMORY_SLOT_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x,y), module, GrooveBox::MEMORY_SLOT_BUTTON_LIGHTS + i));
    }

    // Memory CV input
    addInput(createInputCentered<PJ301MPort>(Vec(87.622, 98), module, GrooveBox::MEM_INPUT));

    // Copy/Paste Memory buttons
    addParam(createParamCentered<VCVButton>(Vec(87.622, 144.00), module, GrooveBox::COPY_BUTTON));
    addParam(createParamCentered<VCVButton>(Vec(87.622, 187), module, GrooveBox::PASTE_BUTTON));
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox*>(this->module);
    assert(module);

    // Read and store shift key status
    module->shift_key = ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT);

    ModuleWidget::onHoverKey(e);
  }

  void appendContextMenu(Menu *menu) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox*>(this->module);
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

    menu->addChild(createMenuLabel("Or.. Double click on a track window"));
    menu->addChild(createMenuLabel("to select a sample for that track."));

    menu->addChild(new MenuEntry); // For spacing only

    // Add interpolation menu from /Common/VoxglitchSamplerModuleWidget.hpp
    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
  }


};
