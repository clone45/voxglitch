#ifdef _TIME_DRAWING
static DrawTimer drawTimer("DigitalProgrammer");
#endif

struct DigitalProgrammerWidget : VoxglitchModuleWidget
{
  DigitalProgrammer* module = NULL;

  float bank_button_positions[24][2] = {
    {561.8, 126.8}, {592.3, 126.8}, {622.8, 126.8}, {653.3, 126.8}, {683.8,126.8}, {714.3,126.8},
    {561.8, 157.1}, {592.3, 157.1}, {622.8, 157.1}, {653.3, 157.1}, {683.8,157.1}, {714.3,157.1},
    {561.8, 187.4}, {592.3, 187.4}, {622.8, 187.4}, {653.3, 187.4}, {683.8,187.4}, {714.3,187.4},
    {561.8, 217.7}, {592.3, 217.7}, {622.8,217.7}, {653.3,217.7}, {683.8,217.7}, {714.3,217.7}
  };

  float output_positions[16][2] = {
    {189.56, 47.975}, {200.827, 47.975}, {212.093, 47.975}, {223.360, 47.975},
    {189.56, 59.242}, {200.827, 59.242}, {212.093, 59.242}, {223.360, 59.242},
    {189.56, 70.509}, {200.827, 70.509}, {212.093, 70.509}, {223.360, 70.509},
    {189.56, 81.776}, {200.827, 81.776}, {212.093, 81.776}, {223.360, 81.776},
  };

  DigitalProgrammerWidget(DigitalProgrammer* module)
  {
    this->module = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_programmer/digital_programmer_front_panel.svg")));

    PNGPanel *png_panel = new PNGPanel("res/digital_programmer/digital_programmer_full.png", 259.08, 128.5);
    addChild(png_panel);

    // Add typography layer
    /*
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/digital_sequencer/digital_sequencer_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);
    */

    // panel_x_position and panel_y_position specify where a slider should
    // be displayed on the front panel relative to 0,0.
    float panel_x_position = 0;
    float panel_y_position = 0;

    float slider_column_x[NUMBER_OF_SLIDERS] = {
      27.000000, 58.750000, 90.500000, 122.000000,
      154.250000, 185.750000, 217.250000, 249.000000,
      280.750000, 312.500000, 344.500000, 376.250000,
      408.000000, 439.750000, 471.500000, 503.000000,
    };

    float output_positions_x[NUMBER_OF_SLIDERS] = {
      39.233265, 71.086334, 102.949997, 134.663681,
      166.403931, 198.153900, 229.803955, 261.553925,
      293.399902, 325.000000, 356.599976, 388.400024,
      420.149902, 451.849915, 483.699951, 515.500000
    };


    //
    // Loop through each column and draw the slider and outputs
    //

    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      // Calculate where to put the slider
      // panel_x_position = (column * horizontal_padding) + margin_left;
      panel_x_position = slider_column_x[column];
      panel_y_position = 25.250000;

      // Add slider widget
      DPSliderDisplay *dp_slider_display = new DPSliderDisplay(column);
      dp_slider_display->setPosition(Vec(panel_x_position, panel_y_position));
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
      dp_bank_button_display->setPosition(Vec(panel_x_position, panel_y_position));
      dp_bank_button_display->setSize(Vec(BANK_BUTTON_WIDTH, BANK_BUTTON_HEIGHT));
      dp_bank_button_display->module = module;
      addChild(dp_bank_button_display);
    }

    // Add outputs
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      // calculation panel_x_position, panel_y_position
      panel_x_position = output_positions_x[column];

      // Add outputs  39.333263,349.639343
      PortWidget *output_port = createOutputCentered<VoxglitchOutputPort>(Vec(panel_x_position, 349.703735), module, column);
      addOutput(output_port);

      // Store pointer to outputs so that I can later fetch the cable informatiion
      // (especially color) later.
      if(module) module->output_ports[column] = output_port;
    }


    // Poly add input
    addInput(createInputCentered<VoxglitchInputPort>(Vec(722.000000,292.000000), module, DigitalProgrammer::POLY_ADD_INPUT));

    // Poly output
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(722.250000,349.500000), module, DigitalProgrammer::POLY_OUTPUT));

    // bank controls
    addInput(createInputCentered<VoxglitchInputPort>(Vec(574.209290,50.708843), module, DigitalProgrammer::BANK_CV_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(623.195435,51.008839), module, DigitalProgrammer::BANK_RESET_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(673.005005,50.808842), module, DigitalProgrammer::BANK_PREV_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(722.345642,50.858841), module, DigitalProgrammer::BANK_NEXT_INPUT));

    // copy/paste mode toggle
    addParam(createParamCentered<squareToggle>(Vec(574.108398,291.704529), module, DigitalProgrammer::COPY_MODE_PARAM));
    addParam(createParamCentered<squareToggle>(Vec(623.299927,291.799927), module, DigitalProgrammer::CLEAR_MODE_PARAM));
    addParam(createParamCentered<squareToggle>(Vec(672.949829,291.699951), module, DigitalProgrammer::RANDOMIZE_MODE_PARAM));
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

  struct OutputRangeValueItem : MenuItem {
    DigitalProgrammer *module;
    int range_index = 0;
    int slider_number = 0;

    void onAction(const event::Action &e) override {
      module->range_settings[slider_number] = range_index;
    }
  };

  struct OutputRangeItem : MenuItem {
    DigitalProgrammer *module;
    int slider_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      for (unsigned int i=0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
      {
        OutputRangeValueItem *output_range_value_menu_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->range_settings[i] == i));
        output_range_value_menu_item->module = module;
        output_range_value_menu_item->range_index = i;
        output_range_value_menu_item->slider_number = this->slider_number;
        menu->addChild(output_range_value_menu_item);
      }

      return menu;
    }
  };

  struct SliderMenuItem : MenuItem {
    DigitalProgrammer *module;
    unsigned int slider_number = 0;

    Menu *createChildMenu() override {
      Menu *menu = new Menu;

      OutputRangeItem *output_range_item = createMenuItem<OutputRangeItem>("Output Range", RIGHT_ARROW);
      output_range_item->slider_number = this->slider_number;
      output_range_item->module = module;
      menu->addChild(output_range_item);

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

    // Add visualize sums toggle
    VisualizeSumsMenuItem* visualize_sums_menu_item = createMenuItem<VisualizeSumsMenuItem>("Visualize Summed Voltages");
    visualize_sums_menu_item->rightText = CHECKMARK(module->visualize_sums == 1);
    visualize_sums_menu_item->module = module;
    menu->addChild(visualize_sums_menu_item);

    // Add user-supplied labels
    LabelsMenu *labels_menu = createMenuItem<LabelsMenu>("Labels", RIGHT_ARROW);
    labels_menu->module = module;
    menu->addChild(labels_menu);

    // Add individual slider settings
    for(unsigned int i=0; i < NUMBER_OF_SLIDERS; i++)
    {
      SliderMenuItem *slider_menu_item = createMenuItem<SliderMenuItem>("Slider #" + std::to_string(i + 1), RIGHT_ARROW);
      slider_menu_item->module = module;
      slider_menu_item->slider_number = i;
      menu->addChild(slider_menu_item);
    }
  }

};
