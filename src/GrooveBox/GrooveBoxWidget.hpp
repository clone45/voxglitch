//
// mm2px 2.952756

#include <componentlibrary.hpp>
#include "menus/TrackMenu.hpp"
#include "menus/OffsetSnapMenu.hpp"

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
    switch(module->selected_function)
    {
      case FUNCTION_VOLUME: module->params[parameter_index].setValue(default_volume); break;
      case FUNCTION_PAN: module->params[parameter_index].setValue(default_pan); break;
      case FUNCTION_PITCH: module->params[parameter_index].setValue(default_pitch); break;
      case FUNCTION_RATCHET: module->params[parameter_index].setValue(default_ratchet); break;
      case FUNCTION_OFFSET: module->params[parameter_index].setValue(default_offset); break;
      case FUNCTION_PROBABILITY: module->params[parameter_index].setValue(default_probability); break;
      case FUNCTION_REVERSE: module->params[parameter_index].setValue(default_reverse); break;
      case FUNCTION_LOOP: module->params[parameter_index].setValue(default_loop); break;
    }
  }
};

struct GrooveboxBlueLight : BlueLight {
  GrooveBox *module;
  // May expand on this eventually
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
      float length = button_positions[module->selected_track->range_end][0] - button_positions[module->selected_track->range_start][0];
      nvgRoundedRect(vg, button_positions[module->selected_track->range_start][0] - 20, 0, length, 12, 5);
    }
    else {
      // Paint static content for library display
      nvgRoundedRect(vg, 0, 0, mm2px(186.51), 12, 5);
    }

    nvgFillColor(vg, nvgRGB(65, 65, 65));
    nvgFill(vg);

    nvgRestore(vg);
  }

};



//
// SequenceLengthWidget
//
// This is the grey horizontal bar that shows the sequence length of the
// selected track.

struct RangeGrabberRightWidget : TransparentWidget
{
  GrooveBox *module;
  bool is_moused_over = false;
  float diameter = 20.0;
  float radius = diameter / 2.0;
  Vec drag_position;

  RangeGrabberRightWidget()
  {
    box.size = Vec(diameter, diameter);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    /* draw bounding box for testing
    nvgSave(vg);
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGBA(120, 20, 20, 100));
    nvgFill(vg);
    nvgRestore(vg);
    */

    // Draw circle
    nvgSave(vg);
    nvgBeginPath(vg);

    if(module)
    {
      this->box.pos = Vec(button_positions[module->selected_track->range_end][0] - radius, this->box.pos.y);
    }
    else
    {
      this->box.pos = Vec(button_positions[10][0] - radius, this->box.pos.y);
    }


    // box.size.x = button_positions[module->selected_track->length][0];
    nvgCircle(vg, box.size.x - radius, box.size.y - radius, radius);

    if(is_moused_over)
    {
      nvgFillColor(vg, nvgRGB(120,120,120));
    }
    else
    {
      nvgFillColor(vg, nvgRGB(65,65,65));
    }

    nvgFill(vg);

    nvgRestore(vg);

  }


  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      // drag_position = e.pos;
      drag_position = this->box.pos;
    }
  }

  void onEnter(const event::Enter &e) override
  {
		glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));

    this->is_moused_over = true;
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    glfwSetCursor(APP->window->win, NULL);

    this->is_moused_over = false;
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    e.consume(this);
    this->is_moused_over = true;
  }

  void step() override {
    TransparentWidget::step();
  }

  void onDragMove(const event::DragMove &e) override
  {
    // TransparentWidget::onDragMove(e);
    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    int quantized_x = ((drag_position.x - button_positions[0][0]) + diameter) / (button_positions[1][0] - button_positions[0][0]);
    quantized_x = clamp(quantized_x, 0, NUMBER_OF_STEPS - 1);

    if((unsigned int) quantized_x > module->selected_track->range_start) module->selected_track->range_end = quantized_x;
  }
};


struct RangeGrabberLeftWidget : TransparentWidget
{
  GrooveBox *module;
  bool is_moused_over = false;
  float diameter = 20.0;
  float radius = diameter / 2.0;
  Vec drag_position;

  RangeGrabberLeftWidget()
  {
    box.size = Vec(diameter, diameter);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;

    // Draw circle
    nvgSave(vg);
    nvgBeginPath(vg);

    if(module)
    {
      this->box.pos = Vec(button_positions[module->selected_track->range_start][0] - radius, this->box.pos.y);
    }
    else
    {
      this->box.pos = Vec(button_positions[10][0] - radius, this->box.pos.y);
    }

    nvgCircle(vg, box.size.x - radius, box.size.y - radius, radius);

    if(is_moused_over) {
      nvgFillColor(vg, nvgRGB(120,120,120));
    }
    else {
      nvgFillColor(vg, nvgRGB(65,65,65));
    }

    nvgFill(vg);
    nvgRestore(vg);

  }


  void onButton(const event::Button &e) override
  {
    if(e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
    {
      e.consume(this);
      drag_position = this->box.pos;
    }
  }

  void onEnter(const event::Enter &e) override
  {
		glfwSetCursor(APP->window->win, glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));

    this->is_moused_over = true;
    TransparentWidget::onEnter(e);
  }

  void onLeave(const event::Leave &e) override
  {
    glfwSetCursor(APP->window->win, NULL);

    this->is_moused_over = false;
    TransparentWidget::onLeave(e);
  }

  void onHover(const event::Hover& e) override {
    e.consume(this);
    this->is_moused_over = true;
  }

  void step() override {
    TransparentWidget::step();
  }

  void onDragMove(const event::DragMove &e) override
  {
    // TransparentWidget::onDragMove(e);
    float zoom = getAbsoluteZoom();
    drag_position = drag_position.plus(e.mouseDelta.div(zoom));

    int quantized_x = ((drag_position.x - button_positions[0][0]) + diameter) / (button_positions[1][0] - button_positions[0][0]);
    quantized_x = clamp(quantized_x, 0, NUMBER_OF_STEPS - 1);

    if((unsigned int) quantized_x < module->selected_track->range_end) module->selected_track->range_start = quantized_x;
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
    char *path = module->selectFileVCV(dir);
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


  void onButton(const event::Button &e) override
  {
    TransparentWidget::onButton(e);
    e.consume(this);
  }

  void draw_track_label(std::string label, NVGcontext *vg)
  {
    float text_left_margin = 6;

    nvgFontSize(vg, 10);
    nvgTextLetterSpacing(vg, 0);
    nvgFillColor(vg, nvgRGBA(255, 215, 20, 0xff));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    float wrap_at = 130.0; // Just throw your hands in the air!  And wave them like you just don't 130.0

    const char *end = NULL;
    NVGtextRow rows[3];
    unsigned int max_rows = 3;
    unsigned int number_of_lines = nvgTextBreakLines(vg, label.c_str(), NULL, wrap_at, rows, max_rows);

    if(number_of_lines > 1) end = rows[1].end;

    float bounds[4];
    nvgTextBoxBounds(vg, text_left_margin, 10, wrap_at, label.c_str(), end, bounds);

    float textHeight = bounds[3];
    nvgTextBox(vg, text_left_margin, (box.size.y / 2.0f) - (textHeight / 2.0f) + 8, wrap_at, label.c_str(), end);
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
      VCVLightBezel<GrooveboxBlueLight> *groovebox_light = createLightParamCentered<VCVLightBezel<GrooveboxBlueLight>>(Vec(button_positions[i][0],button_positions[i][1]), module, GrooveBox::DRUM_PADS + i, GrooveBox::DRUM_PAD_LIGHTS + i);
      groovebox_light->module = module;
      addParam(groovebox_light);

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
    menu->addChild(createMenuLabel("GrooveBox"));

    // Track actions menu
    TracksMenu *tracks_menu = createMenuItem<TracksMenu>("Tracks", RIGHT_ARROW);
    tracks_menu->module = module;
    menu->addChild(tracks_menu);

    // Offset Snap settings menu
    /*
    OffsetSnapMenuItem *offset_snap_menu_item = createMenuItem<OffsetSnapMenuItem>("Offset Snap", RIGHT_ARROW);
    offset_snap_menu_item->module = module;
    menu->addChild(offset_snap_menu_item);
    */

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
