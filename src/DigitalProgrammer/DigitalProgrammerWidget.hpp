#ifdef _TIME_DRAWING
static DrawTimer drawTimer("DigitalProgrammer");
#endif

struct DigitalProgrammerWidget : VoxglitchModuleWidget
{
  DigitalProgrammer* module = NULL;

  DigitalProgrammerWidget(DigitalProgrammer* module)
  {
    this->module = module;
    setModule(module);

    // Load and apply theme
    theme.load("digital_programmer");
    applyTheme();


    //
    // Loop through each column and draw the sliders
    //
    for(unsigned int column = 0; column < NUMBER_OF_SLIDERS; column ++)
    {
      std::string slider_name("SLIDER_" + std::to_string(column + 1));
      std::string output_name("OUTPUT_" + std::to_string(column + 1));
      addSlider(column, themePos(slider_name));
      addOutputEx(column, themePos(output_name));
    }

    // bank buttons
    for(unsigned int i = 0; i < NUMBER_OF_BANKS; i ++)
    {
      std::string bank_name("BANK_" + std::to_string(i + 1));
      addBankButton(i, themePos(bank_name));
    }

    // Poly add input
    addInput(createInputCentered<VoxglitchInputPort>(themePos("POLY_ADD_INPUT"), module, DigitalProgrammer::POLY_ADD_INPUT));

    // Poly output
    addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("POLY_OUTPUT"), module, DigitalProgrammer::POLY_OUTPUT));

    // bank controls
    addInput(createInputCentered<VoxglitchInputPort>(themePos("BANK_CV_INPUT"), module, DigitalProgrammer::BANK_CV_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("BANK_RESET_INPUT"), module, DigitalProgrammer::BANK_RESET_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("BANK_PREV_INPUT"), module, DigitalProgrammer::BANK_PREV_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(themePos("BANK_NEXT_INPUT"), module, DigitalProgrammer::BANK_NEXT_INPUT));

    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("BANK_PREV_PARAM"), module, DigitalProgrammer::BANK_PREV_PARAM));
    addParam(createParamCentered<VoxglitchRoundMomentaryLampSwitch>(themePos("BANK_NEXT_PARAM"), module, DigitalProgrammer::BANK_NEXT_PARAM));

    // copy/paste mode toggle
    addParam(createParamCentered<squareToggle>(themePos("COPY_MODE_PARAM"), module, DigitalProgrammer::COPY_MODE_PARAM));
    addParam(createParamCentered<squareToggle>(themePos("CLEAR_MODE_PARAM"), module, DigitalProgrammer::CLEAR_MODE_PARAM));
    addParam(createParamCentered<squareToggle>(themePos("RANDOMIZE_MODE_PARAM"), module, DigitalProgrammer::RANDOMIZE_MODE_PARAM));

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

  void addSlider(unsigned int column, Vec pos)
  {
    DPSliderDisplay *dp_slider_display = new DPSliderDisplay(column);
    dp_slider_display->setPosition(pos);
    dp_slider_display->setSize(Vec(DRAW_AREA_WIDTH, SLIDER_HEIGHT));
    dp_slider_display->module = module;
    addChild(dp_slider_display);
  }

  void addOutputEx(unsigned int column, Vec pos)
  {
    // Add outputs  39.333263,349.639343
    PortWidget *output_port = createOutputCentered<VoxglitchOutputPort>(pos, module, column);
    addOutput(output_port);

    // Store pointer to outputs so that I can later fetch the cable informatiion
    // (especially color) later.
    if(module) module->output_ports[column] = output_port;
  }

  void addBankButton(unsigned int index, Vec pos)
  {
    DPBankButtonDisplay *dp_bank_button_display = new DPBankButtonDisplay(index);
    dp_bank_button_display->setPosition(pos);
    dp_bank_button_display->setSize(Vec(BANK_BUTTON_WIDTH, BANK_BUTTON_HEIGHT));
    dp_bank_button_display->module = module;
    addChild(dp_bank_button_display);
  }

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
