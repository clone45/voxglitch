//
// mm2px 2.952756

#include <componentlibrary.hpp>
#include "menus/TrackMenu.hpp"
#include "menus/InitializeMenu.hpp"
#include "widgets/RangeGrabbers.hpp"
#include "widgets/GrooveboxBlueLight.hpp"
#include "widgets/SequenceLengthWidget.hpp"
#include "widgets/TrackLabelDisplay.hpp"
#include "widgets/UpdatesWidget.hpp"

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
  {18.8, 332.7667},
  {98, 332.7667},
  {177, 332.7667},
  {256, 332.7667},
  {335, 332.7667},
  {414, 332.7667},
  {493, 332.7667},
  {573, 332.7667},
  {18.8, 360.936},
  {98, 360.936},
  {177, 360.936},
  {256, 360.936},
  {335, 360.936},
  {414, 360.936},
  {493, 360.936},
  {573, 360.936},
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
  GrooveBox *module;
  unsigned int parameter_index = 0;

  TrimpotMedium()
  {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;

    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/TrimpotMedium.svg")));
    bg = new widget::SvgWidget;
    bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/TrimpotMedium_bg.svg")));
    fb->addChildBelow(bg, tw);
  }

  void onDoubleClick(const DoubleClickEvent &e) override
  {
    float value = 0.0;

    switch(module->selected_function)
    {
      case FUNCTION_VOLUME: value = default_volume; break;
      case FUNCTION_PAN: value = default_pan; break;
      case FUNCTION_PITCH: value = default_pitch; break;
      case FUNCTION_RATCHET: value = default_ratchet; break;
      case FUNCTION_SAMPLE_START: value = default_sample_start; break;
      case FUNCTION_SAMPLE_END: value = default_sample_end; break;
      case FUNCTION_PROBABILITY: value = default_probability; break;
      case FUNCTION_REVERSE: value = default_reverse; break;
      case FUNCTION_LOOP: value = default_loop; break;
      case FUNCTION_ATTACK: value = default_attack; break;
      case FUNCTION_RELEASE: value = default_release; break;
      case FUNCTION_DELAY_MIX: value = default_delay_mix; break;
      case FUNCTION_DELAY_LENGTH: value = default_delay_length; break;
      case FUNCTION_DELAY_FEEDBACK: value = default_delay_feedback; break;
    }

    if(module->shift_key)
    {
      // set _all_ knob values to the default
      for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
      {
        module->params[module->STEP_KNOBS + i].setValue(value);
      }
    }
    else
    {
      // set _this_ knob's values to the default
      module->params[parameter_index].setValue(value);
    }
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
    char *path = module->selectFileVCV(dir);
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

    RangeGrabberLeftWidget *range_grabber_left_widget = new RangeGrabberLeftWidget();
    range_grabber_left_widget->setPosition(Vec(button_positions[0][0] - range_grabber_left_widget->radius, button_positions[0][1] - 25 - range_grabber_left_widget->radius));
    range_grabber_left_widget->module = module;
    addChild(range_grabber_left_widget);

    RangeGrabberRightWidget *range_grabber_right_widget = new RangeGrabberRightWidget();
    range_grabber_right_widget->setPosition(Vec(button_positions[0][0] - range_grabber_right_widget->radius, button_positions[0][1] - 25 - range_grabber_right_widget->radius));
    range_grabber_right_widget->module = module;
    addChild(range_grabber_right_widget);

    //
    // Step button related stuff
    //
    for(unsigned int i=0; i<NUMBER_OF_STEPS; i++)
    {
      //
      // Drum pad lights
      //
      VCVLightBezel<GrooveboxBlueLight> *groovebox_blue_light_bezel = createLightParamCentered<VCVLightBezel<GrooveboxBlueLight>>(Vec(button_positions[i][0],button_positions[i][1]), module, GrooveBox::DRUM_PADS + i, GrooveBox::DRUM_PAD_LIGHTS + i);
      GrooveboxBlueLight *groove_box_blue_light = dynamic_cast<GrooveboxBlueLight*>(groovebox_blue_light_bezel->getLight());
      groove_box_blue_light->module = module;
      addParam(groovebox_blue_light_bezel);

      //
      // Step location indicators
      //
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(button_positions[i][0],button_positions[i][1] - 25), module, GrooveBox::STEP_LOCATION_LIGHTS + i));

      //
      // Create attenuator knobs for each step
      //
      TrimpotMedium *knob = createParamCentered<TrimpotMedium>(Vec(button_positions[i][0],button_positions[i][1] + 30), module, GrooveBox::STEP_KNOBS + i);
      knob->module = module;
      knob->parameter_index = GrooveBox::STEP_KNOBS + i;
      addParam(knob);

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

    // Individual track outputs
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

    // Updates widget
    /*
    UpdatesWidget *updates_widget = new UpdatesWidget();
    updates_widget->module = module;
    addChild(updates_widget);
    */
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox*>(this->module);
    assert(module);

    module->shift_key = ((e.mods & RACK_MOD_MASK) && GLFW_MOD_SHIFT);
    module->control_key = ((e.mods & RACK_MOD_MASK) && GLFW_MOD_CONTROL);

    ModuleWidget::onHoverKey(e);
  }

  void appendContextMenu(Menu *menu) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("GrooveBox"));

    // Initialize module
    InitializeMenuItem *initialize_menu_item = createMenuItem<InitializeMenuItem>("Initialize", RIGHT_ARROW);
    initialize_menu_item->module = module;
    menu->addChild(initialize_menu_item);

    // Track actions menu
    TracksMenu *tracks_menu = createMenuItem<TracksMenu>("Tracks", RIGHT_ARROW);
    tracks_menu->module = module;
    menu->addChild(tracks_menu);

    //
    // Start sample selection menu options
    //
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
    menu->addChild(createMenuLabel("Audio Quality"));
    SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
    sample_interpolation_menu_item->module = module;
    menu->addChild(sample_interpolation_menu_item);
  }


};
