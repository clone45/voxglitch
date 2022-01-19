#ifdef _TIME_DRAWING
static DrawTimer drawTimer("DigitalProgrammer");
#endif

struct DigitalProgrammerWidget : ModuleWidget
{
  DigitalProgrammer* module = NULL;

  float bank_button_positions[24][2] = {
    {189.56, 47.975}, {200.827, 47.975}, {212.093, 47.975}, {223.360, 47.975},
    {189.56, 59.242}, {200.827, 59.242}, {212.093, 59.242}, {223.360, 59.242},
    {189.56, 70.509}, {200.827, 70.509}, {212.093, 70.509}, {223.360, 70.509},
    {189.56, 81.776}, {200.827, 81.776}, {212.093, 81.776}, {223.360, 81.776},
    {189.56, 93.043}, {200.827, 93.043}, {212.093, 93.043}, {223.360, 93.043},
    {189.56, 104.317}, {200.827, 104.317}, {212.093, 104.317}, {223.360, 104.317}
  };

  DigitalProgrammerWidget(DigitalProgrammer* module)
  {
    this->module = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_programmer_front_panel.svg")));

    // panel_x_position and panel_y_position specify where a slider should
    // be displayed on the front panel relative to 0,0.
    float panel_x_position = 0;
    float panel_y_position = 0;

    float outputs_vertical_position = 118.0;
    float slider_column_x[NUMBER_OF_SLIDERS] = { 7, 18, 29, 40, 51, 62, 73, 84, 95, 106, 117, 128, 139, 150, 161, 172 };

    //
    // Loop through each column and draw the slider and outputs
    //
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      // Calculate where to put the slider
      // panel_x_position = (column * horizontal_padding) + margin_left;
      panel_x_position = slider_column_x[column];
      panel_y_position = 7.0;

      // Add slider widget
      DPSliderDisplay *dp_slider_display = new DPSliderDisplay(column);
      dp_slider_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
      dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, SLIDER_HEIGHT));
      dp_slider_display->module = module;
      addChild(dp_slider_display);
    }

    //
    // bank buttons
    //
    for(unsigned int i = 0; i < NUMBER_OF_BANKS; i ++)
    {
      // calculation panel_x_position, panel_y_position
      panel_x_position = bank_button_positions[i][0];
      panel_y_position = bank_button_positions[i][1];

      // Add button widget
      DPBankButtonDisplay *dp_bank_button_display = new DPBankButtonDisplay(i);
      dp_bank_button_display->setPosition(mm2px(Vec(panel_x_position, panel_y_position)));
      dp_bank_button_display->setSize(Vec(BANK_BUTTON_WIDTH, BANK_BUTTON_HEIGHT));
      dp_bank_button_display->module = module;
      addChild(dp_bank_button_display);
    }

    // add copy/paste label
    CopyPasteLabel *copy_paste_label = new CopyPasteLabel();
    copy_paste_label->setPosition(mm2px(Vec(198.5, 123.4)));
    copy_paste_label->module = module;
    addChild(copy_paste_label);

    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      // calculation panel_x_position, panel_y_position
      panel_x_position = slider_column_x[column];
      panel_y_position = 7.0;

      // Add outputs
      PortWidget *output_port = createOutput<PJ301MPort>(mm2px(Vec(panel_x_position, outputs_vertical_position)), module, column);
      addOutput(output_port);

      // Store pointer to outputs so that I can later fetch the cable informatiion
      // (especially color) later.
      if(module) module->output_ports[column] = output_port;
    }

    // Poly add input
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(202.404, 11.366)), module, DigitalProgrammer::POLY_ADD_INPUT));

    // Poly output
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(225.683, 11.366)), module, DigitalProgrammer::POLY_OUTPUT));

    // bank controls
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(193.162, 36.593 + 2.642)), module, DigitalProgrammer::BANK_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(205.383, 36.593 + 2.642)), module, DigitalProgrammer::BANK_NEXT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(217.612, 36.593 + 2.642)), module, DigitalProgrammer::BANK_PREV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(229.824, 36.593 + 2.642)), module, DigitalProgrammer::BANK_RESET_INPUT));

    // copy/paste mode toggle
    addParam(createParamCentered<LEDButton>(mm2px(Vec(193.162, 122)), module, DigitalProgrammer::COPY_MODE_PARAM));
    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(193.162, 122)), module, DigitalProgrammer::COPY_MODE_LIGHT));

  }

  struct InputSnapValueItem : MenuItem {
    DigitalProgrammer *module;
    int snap_division_index = 0;
    int slider_number = 0;

    void onAction(const event::Action &e) override {
      module->snap_settings[slider_number] = snap_division_index;
    }
  };

  struct InputSnapItem : MenuItem {
    DigitalProgrammer *module;
    int slider_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      for (unsigned int i=0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
      {
        InputSnapValueItem *input_snap_value_item = createMenuItem<InputSnapValueItem>(module->snap_division_names[i], CHECKMARK(module->snap_settings[slider_number] == i));
        input_snap_value_item->module = module;
        input_snap_value_item->snap_division_index = i;
        input_snap_value_item->slider_number = this->slider_number;
        menu->addChild(input_snap_value_item);
      }

      return menu;
    }
  };

  struct SliderItem : MenuItem {
    DigitalProgrammer *module;
    unsigned int slider_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      InputSnapItem *input_snap_item = createMenuItem<InputSnapItem>("Snap", RIGHT_ARROW);
      input_snap_item->slider_number = this->slider_number;
      input_snap_item->module = module;
      menu->addChild(input_snap_item);

      return menu;
    }
  };

  struct ColorfulSlidersMenuItem : MenuItem {
    DigitalProgrammer* module;
    void onAction(const event::Action& e) override {
      module->colorful_sliders = !(module->colorful_sliders);
    }
  };

  struct VisualizeSumsMenuItem : MenuItem {
    DigitalProgrammer* module;
    void onAction(const event::Action& e) override {
      module->visualize_sums = !(module->visualize_sums);
    }
  };

  struct labelTextField : TextField {

    DigitalProgrammer *module;
    unsigned int column = 0;

    labelTextField(unsigned int column)
    {
      this->column = column;
      this->box.pos.x = 30;
      this->box.size.x = 160;
      this->multiline = false;
    }

    void onChange(const event::Change& e) override {
      module->labels[column] = text;
    };

  };

  struct LabelsMenu : MenuItem {
    DigitalProgrammer *module;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      menu->addChild(new MenuEntry);

      for(unsigned int i=0; i < NUMBER_OF_SLIDERS; i++)
      {
        auto holder = new rack::Widget;
        holder->box.size.x = 200; // This value will determine the width of the menu
        holder->box.size.y = 20;

        auto lab = new rack::Label;
        lab->text = std::to_string(i + 1) + ":";
        lab->box.size = 40; // label box size determins the bounding box around #1, #2, #3 etc.
        holder->addChild(lab);

        auto textfield = new labelTextField(i);
        textfield->module = module;
        textfield->text = module->labels[i];
        holder->addChild(textfield);

        menu->addChild(holder);
      }

      menu->addChild(new MenuEntry);

      return menu;
    }
  };



  void appendContextMenu(Menu *menu) override
  {
    // Add a space
		menu->addChild(new MenuEntry);

    // Add colorful slider toggle
    ColorfulSlidersMenuItem* colorful_sliders_menu_item = createMenuItem<ColorfulSlidersMenuItem>("Match Cable Colors");
    colorful_sliders_menu_item->rightText = CHECKMARK(module->colorful_sliders == 1);
    colorful_sliders_menu_item->module = module;
    menu->addChild(colorful_sliders_menu_item);

    // Add colorful slider toggle
    VisualizeSumsMenuItem* visualize_sums_menu_item = createMenuItem<VisualizeSumsMenuItem>("Visualize Summed Voltages");
    visualize_sums_menu_item->rightText = CHECKMARK(module->visualize_sums == 1);
    visualize_sums_menu_item->module = module;
    menu->addChild(visualize_sums_menu_item);

    /*
    LabelMenuItem * label_menu_item = createMenuItem<LabelMenuItem>("sample input");
    label_menu_item->module = module;
    menu->addChild(label_menu_item);
    */
    // menu->addChild(new LabelTextField());


    /*
    DigitalProgrammer *module = dynamic_cast<DigitalProgrammer*>(this->module);
    assert(module);

    menu->addChild(new MenuEntry); // For spacing only
    menu->addChild(createMenuLabel("Slider Settings"));

    // Add individual sequencer settings
    SliderItem *slider_items[16];

    for(unsigned int i=0; i < NUMBER_OF_SLIDERS; i++)
    {
      slider_items[i] = createMenuItem<SliderItem>("Slider #" + std::to_string(i + 1), RIGHT_ARROW);
      slider_items[i]->module = module;
      slider_items[i]->slider_number = i;
      menu->addChild(slider_items[i]);
    }
    */


    LabelsMenu *labels_menu = createMenuItem<LabelsMenu>("Labels", RIGHT_ARROW);
    labels_menu->module = module;
    menu->addChild(labels_menu);
  }

};
