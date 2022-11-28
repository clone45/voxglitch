//
// mm2px 2.952756

#include <componentlibrary.hpp>
#include "widgets/RangeGrabbers.hpp"
#include "widgets/ParameterKnob.hpp"
#include "widgets/GrooveboxSmallLight.hpp"

#include "widgets/GrooveboxStepButton.hpp"
#include "widgets/GrooveboxSoftButton.hpp"
#include "widgets/GrooveboxParameterButton.hpp"
#include "widgets/GrooveboxMemoryButton.hpp"
#include "widgets/SequenceLengthWidget.hpp"

#include "widgets/LCDDisplay.hpp"
#include "widgets/LCDSampleDisplay.hpp"
#include "widgets/LCDRatchetDisplay.hpp"
#include "widgets/LCDTrackDisplay.hpp"

float memory_slot_button_left_col_X = 126.05; // 125.5;
float memory_slot_button_col_Xstep = 31.25; // 31.4;
float memory_slot_button_top_row_Y = 106.35; // 106.0;
float memory_slot_button_row_Ystep = 31.25; // 31.4;
float memory_slot_button_positions[NUMBER_OF_MEMORY_SLOTS][2] = {
    {memory_slot_button_left_col_X, memory_slot_button_top_row_Y},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep, memory_slot_button_top_row_Y},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*2, memory_slot_button_top_row_Y},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*3, memory_slot_button_top_row_Y},

    {memory_slot_button_left_col_X, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*2, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*3, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep},

    {memory_slot_button_left_col_X, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*2},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*2},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*2, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*2},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*3, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*2},

    {memory_slot_button_left_col_X, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*3},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*3},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*2, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*3},
    {memory_slot_button_left_col_X + memory_slot_button_col_Xstep*3, memory_slot_button_top_row_Y + memory_slot_button_row_Ystep*3}
};

float function_button_left_col_X = 18.2;
float function_button_col_Xstep =  81.26; // 81.16;
float function_button_top_row_Y = 333.5; // 332.6;
float function_button_row_Ystep = 26.15; // 28.0;
float function_button_positions[NUMBER_OF_PARAMETER_LOCKS][2] = {
    {function_button_left_col_X, function_button_top_row_Y}, // FUNCTION_VOLUME
    {function_button_left_col_X + function_button_col_Xstep, function_button_top_row_Y},
    {function_button_left_col_X + function_button_col_Xstep*2, function_button_top_row_Y},
    {function_button_left_col_X + function_button_col_Xstep*3, function_button_top_row_Y},
    {function_button_left_col_X + function_button_col_Xstep*4, function_button_top_row_Y},
    {function_button_left_col_X + function_button_col_Xstep*5, function_button_top_row_Y},
    {function_button_left_col_X + function_button_col_Xstep*6, function_button_top_row_Y},
    {function_button_left_col_X + function_button_col_Xstep*7, function_button_top_row_Y},

    {function_button_left_col_X, function_button_top_row_Y + function_button_row_Ystep},
    {function_button_left_col_X + function_button_col_Xstep, function_button_top_row_Y + function_button_row_Ystep},
    {function_button_left_col_X + function_button_col_Xstep*2, function_button_top_row_Y + function_button_row_Ystep},
    {function_button_left_col_X + function_button_col_Xstep*3, function_button_top_row_Y + function_button_row_Ystep},
    {function_button_left_col_X + function_button_col_Xstep*4, function_button_top_row_Y + function_button_row_Ystep},
    {function_button_left_col_X + function_button_col_Xstep*5, function_button_top_row_Y + function_button_row_Ystep},
    {function_button_left_col_X + function_button_col_Xstep*6, function_button_top_row_Y + function_button_row_Ystep},
    {function_button_left_col_X + function_button_col_Xstep*7, function_button_top_row_Y + function_button_row_Ystep}
};

bool dummy_boolean = false;

float track_button_positions[NUMBER_OF_TRACKS][2] = {
    {257, 102 - 1.6},
    {257, 133.33 - 1.6},
    {257, 164.664 - 1.6},
    {257, 196 - 1.6},
    {462.4 - 14, 102 - 1.6},
    {462.4 - 14, 133.33 - 1.6},
    {462.4 - 14, 164.664 - 1.6},
    {462.4 - 14, 196 - 1.6}};

struct ModdedCL1362 : SvgPort
{
  ModdedCL1362()
  {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/modded_CL1362.svg")));
  }
};


struct LoadSamplesFromFolderMenuItem : MenuItem
{
  GrooveBox *module;
  unsigned int track_number = 0;

  void onAction(const event::Action &e) override
  {
#ifdef USING_CARDINAL_NOT_RACK
    GrooveBox *module = this->module;
    async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load folder", [module](char *path)
                             {
			if(path){
				pathSelected(module, std::string(path));
				free(path);
			} });
#else
    pathSelected(module, module->selectPathVCV());
#endif
  }

  static void pathSelected(GrooveBox *module, std::string path)
  {
    if (path != "")
    {
      std::vector<std::string> dirList = system::getEntries(path);

      unsigned int i = 0;

      for (auto entry : dirList)
      {
        if (
            // Something happened in Rack 2 where the extension started to include
            // the ".", so I decided to check for both versions, just in case.
            (rack::string::lowercase(system::getExtension(entry)) == "wav") ||
            (rack::string::lowercase(system::getExtension(entry)) == ".wav"))
        {
          if (i < NUMBER_OF_TRACKS)
          {
            module->sample_players[i].loadSample(std::string(entry));
            module->loaded_filenames[i] = module->sample_players[i].getFilename();
            i++;
          }
        }
      }

      module->setRoot(path);
    }
  }
};

struct LoadSampleMenuItem : MenuItem
{
  GrooveBox *module;
  unsigned int track_number = 0;

  void onAction(const event::Action &e) override
  {
#ifdef USING_CARDINAL_NOT_RACK
    GrooveBox *module = this->module;
    unsigned int track_number = this->track_number;
    async_dialog_filebrowser(false, NULL, module->samples_root_dir.c_str(), "Load sample", [module, track_number](char *filename)
                             {
      if(filename)
      {
        fileSelected(module, track_number, std::string(filename));
        free(filename);
      } });
#else
    fileSelected(module, this->track_number, module->selectFileVCV());
#endif
  }

  static void fileSelected(GrooveBox *module, unsigned int track_number, std::string filename)
  {
    if (filename != "")
    {
      module->sample_players[track_number].loadSample(filename);
      module->loaded_filenames[track_number] = module->sample_players[track_number].getFilename();
      module->setRoot(filename);
    }
  }
};

struct GrooveBoxWidget : VoxglitchSamplerModuleWidget
{
  GrooveBoxWidget(GrooveBox *module)
  {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/groovebox/groove_box_front_panel.svg")));

    addInput(createInputCentered<PJ301MPort>(Vec(39.2, 102), module, GrooveBox::STEP_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(39.2, 154), module, GrooveBox::RESET_INPUT));

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
    for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
    {
      //
      // Drum step buttons
      //
      GrooveboxStepButton *step_button = createParamCentered<GrooveboxStepButton>(Vec(button_positions[i][0], button_positions[i][1]), module, GrooveBox::DRUM_PADS + i);
      step_button->module = module;
      step_button->index = i;
      GrooveboxSmallLight *inner_button = new GrooveboxSmallLight((module) ? &module->inner_light_booleans[i] : &dummy_boolean);
      inner_button->box.pos = Vec(10.358, 6.710);
      step_button->addChild(inner_button);
      addParam(step_button);

      //
      // Step location indicators
      //
      GrooveboxSmallLight *step_light_widget = new GrooveboxSmallLight((module) ? &module->light_booleans[i] : &dummy_boolean);
      step_light_widget->box.pos = Vec(button_positions[i][0] - (step_light_widget->box.size.x / 2.0), button_positions[i][1] - 25 - (step_light_widget->box.size.y / 2.0));
      addChild(step_light_widget);
      
      //
      // Create parameter knobs for each step
      //
      ParameterKnob *knob = createParamCentered<ParameterKnob>(Vec(button_positions[i][0], button_positions[i][1] + 35.65), module, GrooveBox::STEP_KNOBS + i);
      knob->module = module;
      knob->parameter_index = GrooveBox::STEP_KNOBS + i;
      knob->step = i;
      addParam(knob);
    }

    for(unsigned int parameter_slot = 0; parameter_slot<NUMBER_OF_PARAMETER_LOCKS; parameter_slot++)
    {
      unsigned int parameter_index = parameter_slots[parameter_slot];

      float x = function_button_positions[parameter_slot][0];
      float y = function_button_positions[parameter_slot][1];

      GrooveboxParameterButton *groovebox_parameter_button = createParamCentered<GrooveboxParameterButton>(Vec(x, y), module, GrooveBox::PARAMETER_LOCK_BUTTONS + parameter_index);
      groovebox_parameter_button->parameter_index = parameter_index;
      groovebox_parameter_button->parameter_slot = parameter_slot;
      groovebox_parameter_button->module = module;
      addParam(groovebox_parameter_button);
    }


    // Individual track outputs
    for (unsigned int i = 0; i < (NUMBER_OF_TRACKS * 2); i++)
    {
      addOutput(createOutputCentered<ModdedCL1362>(mm2px(Vec(10 + (i * 11), 6)), module, GrooveBox::TRACK_OUTPUTS + i));
    }

    // Mix output L/R
    addOutput(createOutputCentered<ModdedCL1362>(mm2px(Vec(202.25, 6)), module, GrooveBox::AUDIO_OUTPUT_LEFT));
    addOutput(createOutputCentered<ModdedCL1362>(mm2px(Vec(213.25, 6)), module, GrooveBox::AUDIO_OUTPUT_RIGHT));

    // Master volume
    addParam(createParamCentered<Trimpot>(mm2px(Vec(188.8465, 6)), module, GrooveBox::MASTER_VOLUME));

    //
    // Memory buttons
    //
    for (unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      float x = memory_slot_button_positions[i][0];
      float y = memory_slot_button_positions[i][1];

      GrooveboxMemoryButton *groovebox_memory_button = createParamCentered<GrooveboxMemoryButton>(Vec(x, y), module, GrooveBox::MEMORY_SLOT_BUTTONS + i);
      groovebox_memory_button->module = module;
      groovebox_memory_button->memory_slot = i;
      addParam(groovebox_memory_button);
    }

    // Memory CV input
    addInput(createInputCentered<PJ301MPort>(Vec(87.622, 102), module, GrooveBox::MEM_INPUT));

    GrooveboxSoftButton *copy_button = createParamCentered<GrooveboxSoftButton>(Vec(86.56, 152.50), module, GrooveBox::COPY_BUTTON);
    copy_button->momentary = true;
    addParam(copy_button);

    GrooveboxSoftButton *paste_button = createParamCentered<GrooveboxSoftButton>(Vec(86.56, 193), module, GrooveBox::PASTE_BUTTON);
    paste_button->momentary = true;
    addParam(paste_button);

    //
    // LCD displays
    //

    LCDTrackDisplay *lcd_track_display = new LCDTrackDisplay(module);
    addChild(lcd_track_display);

    if(module) // skip these when viewinng the module in the library
    {
      LCDSampleDisplay *lcd_sample_display = new LCDSampleDisplay(module);
      addChild(lcd_sample_display);

      LCDRatchetDisplay *lcd_ratchet_display = new LCDRatchetDisplay(module);
      addChild(lcd_ratchet_display);
    }
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
    assert(module);

    module->shift_key = ((e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT);

    ModuleWidget::onHoverKey(e);
  }

  void appendContextMenu(Menu *menu) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
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

    // All Track actions menu
    AllTracksMenu *all_tracks_menu = createMenuItem<AllTracksMenu>("All Tracks", RIGHT_ARROW);
    all_tracks_menu->module = module;
    menu->addChild(all_tracks_menu);

    // LCD color theme
    LCDColorThemeMenu *lcd_color_theme_menu = createMenuItem<LCDColorThemeMenu>("LCD Color Theme", RIGHT_ARROW);
    lcd_color_theme_menu->module = module;
    menu->addChild(lcd_color_theme_menu);

    //
    // Start sample selection menu options
    //
    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Load individual samples"));

    //
    // Add the sample slots to the right-click context menu
    //
    for (int i = 0; i < NUMBER_OF_TRACKS; i++)
    {
      LoadSampleMenuItem *menu_item_load_sample = new LoadSampleMenuItem();
      menu_item_load_sample->track_number = i;
      menu_item_load_sample->text = std::to_string(i + 1) + ": " + module->sample_players[i].getFilename();
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

  // =================================================================
  // MENUS
  // =================================================================

  struct InitializeConfirmMenuItem : MenuItem
  {
    GrooveBox *module;

    void onAction(const event::Action &e) override
    {
      module->initialize();
    }
  };

  struct InitializeMenuItem : MenuItem
  {
    GrooveBox *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      menu->addChild(createMenuLabel("This will clear ALL of the module's data.  Are you sure??"));

      InitializeConfirmMenuItem *initialize_confirm_menu_item = createMenuItem<InitializeConfirmMenuItem>("YES");
      initialize_confirm_menu_item->module = module;
      menu->addChild(initialize_confirm_menu_item);
      return menu;
    }
  };

  struct SamplePositionSnapValueItem : MenuItem
  {
    GrooveBox *module;
    unsigned int sample_position_snap_index = 0;
    unsigned int track_index = 0;

    void onAction(const event::Action &e) override
    {
      module->sample_position_snap_indexes[track_index] = sample_position_snap_index;
      module->setSamplePositionSnapIndex(sample_position_snap_index, track_index);
    }
  };

  struct SamplePositionSnapMenuItem : MenuItem
  {
    GrooveBox *module;
    unsigned int track_index = 0;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      for (unsigned int i = 0; i < NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS; i++)
      {
        SamplePositionSnapValueItem *sample_position_snap_value_item = createMenuItem<SamplePositionSnapValueItem>(sample_position_snap_names[i], CHECKMARK(module->sample_position_snap_indexes[track_index] == i));
        sample_position_snap_value_item->module = module;
        sample_position_snap_value_item->sample_position_snap_index = i;
        sample_position_snap_value_item->track_index = this->track_index;
        menu->addChild(sample_position_snap_value_item);
      }

      return menu;
    }
  };

  struct ClearMenuItem : MenuItem
  {
    GrooveBox *module;
    int track_index = 0;

    void onAction(const event::Action &e) override
    {
      module->selected_memory_slot->tracks[track_index].clear();
      module->updatePanelControls();
    }
  };

  struct TrackMenuItem : MenuItem
  {
    GrooveBox *module;
    unsigned int track_index = 0;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      SamplePositionSnapMenuItem *sample_position_snap_menu_item = createMenuItem<SamplePositionSnapMenuItem>("Sample Position Snap Division", RIGHT_ARROW);
      sample_position_snap_menu_item->track_index = this->track_index;
      sample_position_snap_menu_item->module = module;
      menu->addChild(sample_position_snap_menu_item);

      ClearMenuItem *clear_menu_item = createMenuItem<ClearMenuItem>("Clear Track Data");
      clear_menu_item->track_index = this->track_index;
      clear_menu_item->module = module;
      menu->addChild(clear_menu_item);

      return menu;
    }
  };

  struct TracksMenu : MenuItem
  {
    GrooveBox *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;
      TrackMenuItem *track_menu_items[NUMBER_OF_TRACKS];

      for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
      {
        track_menu_items[i] = createMenuItem<TrackMenuItem>("Track #" + std::to_string(i + 1), RIGHT_ARROW);
        track_menu_items[i]->module = module;
        track_menu_items[i]->track_index = i;
        menu->addChild(track_menu_items[i]);
      }

      return menu;
    }
  };

  struct ShiftActionValueItem : MenuItem
  {
    GrooveBox *module;
    int direction = 0; // 1 == left, -1 == right

    void onAction(const event::Action &e) override
    {
      module->shiftAllTracks(direction);
    }
  };

  struct ShiftActionMenuItem : MenuItem
  {
    GrooveBox *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      ShiftActionValueItem *shift_action_left = createMenuItem<ShiftActionValueItem>("Shift Left");
      shift_action_left->module = module;
      shift_action_left->direction = 1;
      menu->addChild(shift_action_left);

      ShiftActionValueItem *shift_action_right = createMenuItem<ShiftActionValueItem>("Shift Right");
      shift_action_right->module = module;
      shift_action_right->direction = -1;
      menu->addChild(shift_action_right);

      return menu;
    }
  };

  struct AllTracksMenu : MenuItem
  {
    GrooveBox *module;

    Menu *createChildMenu() override
    {
      Menu *menu = new Menu;

      ShiftActionMenuItem *shift_action_menu_item = createMenuItem<ShiftActionMenuItem>("Shift All Sequences", RIGHT_ARROW);
      shift_action_menu_item->module = module;
      menu->addChild(shift_action_menu_item);

      return menu;
    }
  };

  struct LCDColorMenuItem : MenuItem {
    GrooveBox *module;
    unsigned int theme_id = 0;

    void onAction(const event::Action &e) override {
      LCDColorScheme::selected_color_scheme = theme_id;
    }
  };

  struct LCDColorThemeMenu : MenuItem
  {
    GrooveBox *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      LCDColorMenuItem *lcd_color_menu_item_0 = createMenuItem<LCDColorMenuItem>("Legacy Red", CHECKMARK(LCDColorScheme::selected_color_scheme == 0));
      lcd_color_menu_item_0->module = module;
      lcd_color_menu_item_0->theme_id = 0;
      menu->addChild(lcd_color_menu_item_0);

      LCDColorMenuItem *lcd_color_menu_item_1 = createMenuItem<LCDColorMenuItem>("Soft Yellow", CHECKMARK(LCDColorScheme::selected_color_scheme == 1));
      lcd_color_menu_item_1->module = module;
      lcd_color_menu_item_1->theme_id = 1;
      menu->addChild(lcd_color_menu_item_1);

      LCDColorMenuItem *lcd_color_menu_item_2 = createMenuItem<LCDColorMenuItem>("Cool White", CHECKMARK(LCDColorScheme::selected_color_scheme == 2));
      lcd_color_menu_item_2->module = module;
      lcd_color_menu_item_2->theme_id = 2;
      menu->addChild(lcd_color_menu_item_2);

      LCDColorMenuItem *lcd_color_menu_item_3 = createMenuItem<LCDColorMenuItem>("Ice Blue", CHECKMARK(LCDColorScheme::selected_color_scheme == 3));
      lcd_color_menu_item_3->module = module;
      lcd_color_menu_item_3->theme_id = 3;
      menu->addChild(lcd_color_menu_item_3);

      LCDColorMenuItem *lcd_color_menu_item_4 = createMenuItem<LCDColorMenuItem>("Data Green", CHECKMARK(LCDColorScheme::selected_color_scheme == 4));
      lcd_color_menu_item_4->module = module;
      lcd_color_menu_item_4->theme_id = 4;
      menu->addChild(lcd_color_menu_item_4);

      return menu;
    }
  };  
};
