//
// mm2px 2.952756

#include <componentlibrary.hpp>
#include "widgets/RangeGrabbers.hpp"
#include "widgets/GrooveboxBlueLight.hpp"
#include "widgets/SequenceLengthWidget.hpp"
#include "widgets/SampleVisualizer.hpp"
#include "widgets/RatchetVisualizer.hpp"
#include "widgets/TrackLabelDisplay.hpp"
#include "widgets/TrackSampleNudge.hpp"

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
    {18.8, 332.7667}, // FUNCTION_VOLUME
    {98, 332.7667},   // FUNCTION_PAN
    {177, 332.7667},  // FUNCTION_PITCH
    {256, 332.7667},  // FUNCTION_RATCHET
    {335, 360.936},   // FUNCTION_SAMPLE_START
    {335, 332.7667},  // FUNCTION_PROBABILITY
    {177, 360.936},   // FUNCTION_LOOP
    {256, 360.936},   // FUNCTION_REVERSE

    {18.8, 360.936}, // FUNCTION_ATTACK
    {98, 360.936},   // FUNCTION_RELEASE
    {414, 332.7667}, // FUNCTION_DELAY_MIX
    {493, 332.7667}, // FUNCTION_DELAY_LENGTH
    {573, 332.7667}, // FUNCTION_DELAY_FEEDBACK
    {414, 360.936},  // FUNCTION_SAMPLE_END
    {493, 360.936},  // Position #15
    {573, 360.936},  // Position #16
};

float track_button_positions[NUMBER_OF_TRACKS][2] = {
    {256, 93 - 1.6},
    {256, 124.33 - 1.6},
    {256, 155.664 - 1.6},
    {256, 187 - 1.6},
    {461.4 - 14, 93 - 1.6},
    {461.4 - 14, 124.33 - 1.6},
    {461.4 - 14, 155.664 - 1.6},
    {461.4 - 14, 187 - 1.6}};

struct ModdedCL1362 : SvgPort
{
  ModdedCL1362()
  {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/modded_CL1362.svg")));
  }
};

struct TrimpotMedium : SvgKnob
{
  widget::SvgWidget *bg;
  GrooveBox *module;
  unsigned int parameter_index = 0;
  unsigned int step = 0;

  TrimpotMedium()
  {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;

    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/TrimpotMedium.svg")));
    bg = new widget::SvgWidget;
    bg->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/TrimpotMedium_bg.svg")));
    fb->addChildBelow(bg, tw);
  }

  void onDoubleClick(const DoubleClickEvent &e) override
  {
    float value = 0.0;

    switch (module->selected_function)
    {
    case FUNCTION_VOLUME:
      value = default_volume;
      break;
    case FUNCTION_PAN:
      value = default_pan;
      break;
    case FUNCTION_PITCH:
      value = default_pitch;
      break;
    case FUNCTION_RATCHET:
      value = default_ratchet;
      break;
    case FUNCTION_SAMPLE_START:
      value = default_sample_start;
      break;
    case FUNCTION_SAMPLE_END:
      value = default_sample_end;
      break;
    case FUNCTION_PROBABILITY:
      value = default_probability;
      break;
    case FUNCTION_REVERSE:
      value = default_reverse;
      break;
    case FUNCTION_LOOP:
      value = default_loop;
      break;
    case FUNCTION_ATTACK:
      value = default_attack;
      break;
    case FUNCTION_RELEASE:
      value = default_release;
      break;
    case FUNCTION_DELAY_MIX:
      value = default_delay_mix;
      break;
    case FUNCTION_DELAY_LENGTH:
      value = default_delay_length;
      break;
    case FUNCTION_DELAY_FEEDBACK:
      value = default_delay_feedback;
      break;
    }

    if (module->shift_key)
    {
      // set _all_ knob values to the default
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
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

  void onButton(const event::Button &e) override
  {
    if (module->selected_function == FUNCTION_SAMPLE_START || module->selected_function == FUNCTION_SAMPLE_END)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        if (module->lcd_screen_mode != module->SAMPLE)
        {
          module->lcd_screen_mode = module->SAMPLE;
          module->visualizer_step = step;
        }
      }
    }

    if (module->selected_function == FUNCTION_RATCHET)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
      {
        if (module->lcd_screen_mode != module->RATCHET)
        {
          module->lcd_screen_mode = module->RATCHET;
          module->visualizer_step = step;
        }
      }
    }

    SvgKnob::onButton(e);
  }

  void onDragEnd(const DragEndEvent &e) override
  {

    if (module->selected_function == FUNCTION_SAMPLE_START || module->selected_function == FUNCTION_SAMPLE_END)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT)
        module->lcd_screen_mode = module->TRACK;
    }

    if (module->selected_function == FUNCTION_RATCHET)
    {
      if (e.button == GLFW_MOUSE_BUTTON_LEFT)
        module->lcd_screen_mode = module->TRACK;
    }

    SvgKnob::onDragEnd(e);
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
    for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++)
    {
      //
      // Drum pad lights
      //
      VCVLightBezel<GrooveboxBlueLight> *groovebox_blue_light_bezel = createLightParamCentered<VCVLightBezel<GrooveboxBlueLight>>(Vec(button_positions[i][0], button_positions[i][1]), module, GrooveBox::DRUM_PADS + i, GrooveBox::DRUM_PAD_LIGHTS + i);
      GrooveboxBlueLight *groove_box_blue_light = dynamic_cast<GrooveboxBlueLight *>(groovebox_blue_light_bezel->getLight());
      groove_box_blue_light->module = module;
      groove_box_blue_light->index = i;
      addParam(groovebox_blue_light_bezel);

      //
      // Step location indicators
      //
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(button_positions[i][0], button_positions[i][1] - 25), module, GrooveBox::STEP_LOCATION_LIGHTS + i));

      //
      // Create attenuator knobs for each step
      //
      TrimpotMedium *knob = createParamCentered<TrimpotMedium>(Vec(button_positions[i][0], button_positions[i][1] + 30), module, GrooveBox::STEP_KNOBS + i);
      knob->module = module;
      knob->parameter_index = GrooveBox::STEP_KNOBS + i;
      knob->step = i;
      addParam(knob);
    }

    // Function Buttons
    for (unsigned int i = 0; i < NUMBER_OF_FUNCTIONS; i++)
    {

      // The function buttons got shifted around at some point during the evolution
      // of the module.  The "ordering_of_functions" array maps the function index
      // to the correct location on the front panel.
      float x = function_button_positions[i][0];
      float y = function_button_positions[i][1];

      addParam(createParamCentered<LEDButton>(Vec(x, y), module, GrooveBox::FUNCTION_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x, y), module, GrooveBox::FUNCTION_BUTTON_LIGHTS + i));
    }

    // Track buttons and labels

    for (unsigned int i = 0; i < NUMBER_OF_TRACKS; i++)
    {

      float x = track_button_positions[i][0];
      float y = track_button_positions[i][1];

      TrackLabelDisplay *track_label_display = new TrackLabelDisplay(i);
      track_label_display->setPosition(Vec(x, y - 14));
      track_label_display->module = module;
      addChild(track_label_display);

      TrackSampleNudge *track_sample_nudge_up = new TrackSampleNudge(i);
      track_sample_nudge_up->setPosition(Vec(x + 163, y - 14)); // 162
      track_sample_nudge_up->module = module;
      track_sample_nudge_up->direction = -1;
      addChild(track_sample_nudge_up);

      TrackSampleNudge *track_sample_nudge_down = new TrackSampleNudge(i);
      track_sample_nudge_down->setPosition(Vec(x + 163, y + 1));
      track_sample_nudge_down->module = module;
      track_sample_nudge_down->direction = 1;
      addChild(track_sample_nudge_down);
    }

    // Individual track outputs
    for (unsigned int i = 0; i < (NUMBER_OF_TRACKS * 2); i++)
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
    for (unsigned int i = 0; i < NUMBER_OF_MEMORY_SLOTS; i++)
    {
      float x = memory_slot_button_positions[i][0];
      float y = memory_slot_button_positions[i][1];
      addParam(createParamCentered<LEDButton>(Vec(x, y), module, GrooveBox::MEMORY_SLOT_BUTTONS + i));
      addChild(createLightCentered<MediumLight<GreenLight>>(Vec(x, y), module, GrooveBox::MEMORY_SLOT_BUTTON_LIGHTS + i));
    }

    // Memory CV input
    addInput(createInputCentered<PJ301MPort>(Vec(87.622, 98), module, GrooveBox::MEM_INPUT));

    // Copy/Paste Memory buttons
    addParam(createParamCentered<VCVButton>(Vec(87.622, 144.00), module, GrooveBox::COPY_BUTTON));
    addParam(createParamCentered<VCVButton>(Vec(87.622, 187), module, GrooveBox::PASTE_BUTTON));

    // Sample Visualizer Widget

    SampleVisualizerWidget *sampler_visualizer_widget = new SampleVisualizerWidget();
    sampler_visualizer_widget->module = module;
    sampler_visualizer_widget->box.pos.x = 83.348 * 2.952756;
    sampler_visualizer_widget->box.pos.y = 21.796 * 2.952756;
    addChild(sampler_visualizer_widget);

    RatchetVisualizerWidget *ratchet_visualizer_widget = new RatchetVisualizerWidget();
    ratchet_visualizer_widget->module = module;
    ratchet_visualizer_widget->box.pos.x = 83.348 * 2.952756;
    ratchet_visualizer_widget->box.pos.y = 21.796 * 2.952756;
    addChild(ratchet_visualizer_widget);
  }

  void onHoverKey(const event::HoverKey &e) override
  {
    GrooveBox *module = dynamic_cast<GrooveBox *>(this->module);
    assert(module);

    module->shift_key = ((e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT);
    module->control_key = ((e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL);

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
      module->updateKnobPositions();
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
};
