#ifdef _TIME_DRAWING
static DrawTimer drawTimer("DigitalProgrammer");
#endif

struct DigitalProgrammerWidget : ModuleWidget
{
    DigitalProgrammer *module = NULL;

    DigitalProgrammerWidget(DigitalProgrammer *module)
    {
        this->module = module;
        setModule(module);

        // Load and apply theme
        // theme.load("digital_programmer");
        // applyTheme();

//        setPanel(createPanel(
//            asset::plugin(pluginInstance, "res/modules/digital_programmer/digital_programmer_panel.svg"),
//            asset::plugin(pluginInstance, "res/modules/digital_programmer/digital_programmer_panel-dark.svg")));

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/digital_programmer/digital_programmer_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/digital_programmer/digital_programmer_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Add sliders
        addSlider(0, Vec(27.000000, 25.250000));
        addSlider(1, Vec(58.750000, 25.250000));
        addSlider(2, Vec(90.500000, 25.250000));
        addSlider(3, Vec(122.000000, 25.250000));
        addSlider(4, Vec(154.250000, 25.250000));
        addSlider(5, Vec(185.750000, 25.250000));
        addSlider(6, Vec(217.250000, 25.250000));
        addSlider(7, Vec(249.000000, 25.250000));
        addSlider(8, Vec(280.750000, 25.250000));
        addSlider(9, Vec(312.500000, 25.250000));
        addSlider(10, Vec(344.500000, 25.250000));
        addSlider(11, Vec(376.250000, 25.250000));
        addSlider(12, Vec(408.000000, 25.250000));
        addSlider(13, Vec(439.750000, 25.250000));
        addSlider(14, Vec(471.500000, 25.250000));
        addSlider(15, Vec(503.000000, 25.250000));

        // Add outputs
        addOutputEx(0, panelHelper.findNamed("output_0"));
        addOutputEx(1, panelHelper.findNamed("output_1"));
        addOutputEx(2, panelHelper.findNamed("output_2"));
        addOutputEx(3, panelHelper.findNamed("output_3"));
        addOutputEx(4, panelHelper.findNamed("output_4"));
        addOutputEx(5, panelHelper.findNamed("output_5"));
        addOutputEx(6, panelHelper.findNamed("output_6"));
        addOutputEx(7, panelHelper.findNamed("output_7"));
        addOutputEx(8, panelHelper.findNamed("output_8"));
        addOutputEx(9, panelHelper.findNamed("output_9"));
        addOutputEx(10, panelHelper.findNamed("output_10"));
        addOutputEx(11, panelHelper.findNamed("output_11"));
        addOutputEx(12, panelHelper.findNamed("output_12"));
        addOutputEx(13, panelHelper.findNamed("output_13"));
        addOutputEx(14, panelHelper.findNamed("output_14"));
        addOutputEx(15, panelHelper.findNamed("output_15"));

        // Add bank buttons
        addBankButton(0, Vec(561.8, 126.8));
        addBankButton(1, Vec(592.3, 126.8));
        addBankButton(2, Vec(622.8, 126.8));
        addBankButton(3, Vec(653.3, 126.8));
        addBankButton(4, Vec(683.8, 126.8));
        addBankButton(5, Vec(714.3, 126.8));
        addBankButton(6, Vec(561.8, 157.1));
        addBankButton(7, Vec(592.3, 157.1));
        addBankButton(8, Vec(622.8, 157.1));
        addBankButton(9, Vec(653.3, 157.1));
        addBankButton(10, Vec(683.8, 157.1));
        addBankButton(11, Vec(714.3, 157.1));
        addBankButton(12, Vec(561.8, 187.4));
        addBankButton(13, Vec(592.3, 187.4));
        addBankButton(14, Vec(622.8, 187.4));
        addBankButton(15, Vec(653.3, 187.4));
        addBankButton(16, Vec(683.8, 187.4));
        addBankButton(17, Vec(714.3, 187.4));
        addBankButton(18, Vec(561.8, 217.7));
        addBankButton(19, Vec(592.3, 217.7));
        addBankButton(20, Vec(622.8, 217.7));
        addBankButton(21, Vec(653.3, 217.7));
        addBankButton(22, Vec(683.8, 217.7));
        addBankButton(23, Vec(714.3, 217.7));

        // Poly add input
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("poly_add_input"), module, DigitalProgrammer::POLY_ADD_INPUT));

        // Poly output
        addOutput(createOutputCentered<VoxglitchPolyPort>(panelHelper.findNamed("poly_output"), module, DigitalProgrammer::POLY_OUTPUT));

        // Bank controls
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("bank_cv_input"), module, DigitalProgrammer::BANK_CV_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("bank_reset_input"), module, DigitalProgrammer::BANK_RESET_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("bank_prev_input"), module, DigitalProgrammer::BANK_PREV_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("bank_next_input"), module, DigitalProgrammer::BANK_NEXT_INPUT));

        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("bank_prev_button"), module, DigitalProgrammer::BANK_PREV_PARAM, DigitalProgrammer::BANK_PREV_LIGHT));
        addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(panelHelper.findNamed("bank_next_button"), module, DigitalProgrammer::BANK_NEXT_PARAM, DigitalProgrammer::BANK_NEXT_LIGHT));

        // Copy/paste mode toggle
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("copy_button"), module, DigitalProgrammer::COPY_MODE_PARAM));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("clear_button"), module, DigitalProgrammer::CLEAR_MODE_PARAM));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("randomize_button"), module, DigitalProgrammer::RANDOMIZE_MODE_PARAM));
    }

    struct InputSnapValueItem : MenuItem
    {
        DigitalProgrammer *module;
        int snap_division_index = 0;
        int slider_number = 0;

        void onAction(const event::Action &e) override
        {
            module->snap_settings[slider_number] = snap_division_index;
        }
    };

    struct InputSnapItem : MenuItem
    {
        DigitalProgrammer *module;
        int slider_number = 0;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            for (unsigned int i = 0; i < NUMBER_OF_SNAP_DIVISIONS; i++)
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

    struct SliderItem : MenuItem
    {
        DigitalProgrammer *module;
        unsigned int slider_number = 0;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            InputSnapItem *input_snap_item = createMenuItem<InputSnapItem>("Snap", RIGHT_ARROW);
            input_snap_item->slider_number = this->slider_number;
            input_snap_item->module = module;
            menu->addChild(input_snap_item);

            return menu;
        }
    };

    struct ColorfulSlidersMenuItem : MenuItem
    {
        DigitalProgrammer *module;
        void onAction(const event::Action &e) override
        {
            module->colorful_sliders = !(module->colorful_sliders);
        }
    };

    struct VisualizeSumsMenuItem : MenuItem
    {
        DigitalProgrammer *module;
        void onAction(const event::Action &e) override
        {
            module->visualize_sums = !(module->visualize_sums);
        }
    };

    struct labelTextField : TextField
    {

        DigitalProgrammer *module;
        unsigned int column = 0;

        labelTextField(unsigned int column)
        {
            this->column = column;
            this->box.pos.x = 30;
            this->box.size.x = 160;
            this->multiline = false;
        }

        void onChange(const event::Change &e) override
        {
            module->labels[column] = text;
        };
    };

    struct LabelsMenu : MenuItem
    {
        DigitalProgrammer *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            menu->addChild(new MenuEntry);

            for (unsigned int i = 0; i < NUMBER_OF_SLIDERS; i++)
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

    struct OutputRangeValueItem : MenuItem
    {
        DigitalProgrammer *module;
        int range_index = 0;
        int slider_number = 0;

        void onAction(const event::Action &e) override
        {
            module->range_settings[slider_number] = range_index;
        }
    };

    struct OutputRangeItem : MenuItem
    {
        DigitalProgrammer *module;
        int slider_number = 0;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            for (unsigned int i = 0; i < NUMBER_OF_VOLTAGE_RANGES; i++)
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

    struct SliderMenuItem : MenuItem
    {
        DigitalProgrammer *module;
        unsigned int slider_number = 0;

        Menu *createChildMenu() override
        {
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
        if (module)
            module->output_ports[column] = output_port;
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
        ColorfulSlidersMenuItem *colorful_sliders_menu_item = createMenuItem<ColorfulSlidersMenuItem>("Match Cable Colors");
        colorful_sliders_menu_item->rightText = CHECKMARK(module->colorful_sliders == 1);
        colorful_sliders_menu_item->module = module;
        menu->addChild(colorful_sliders_menu_item);

        // Add visualize sums toggle
        VisualizeSumsMenuItem *visualize_sums_menu_item = createMenuItem<VisualizeSumsMenuItem>("Visualize Summed Voltages");
        visualize_sums_menu_item->rightText = CHECKMARK(module->visualize_sums == 1);
        visualize_sums_menu_item->module = module;
        menu->addChild(visualize_sums_menu_item);

        // Add user-supplied labels
        LabelsMenu *labels_menu = createMenuItem<LabelsMenu>("Labels", RIGHT_ARROW);
        labels_menu->module = module;
        menu->addChild(labels_menu);

        // Add individual slider settings
        for (unsigned int i = 0; i < NUMBER_OF_SLIDERS; i++)
        {
            SliderMenuItem *slider_menu_item = createMenuItem<SliderMenuItem>("Slider #" + std::to_string(i + 1), RIGHT_ARROW);
            slider_menu_item->module = module;
            slider_menu_item->slider_number = i;
            menu->addChild(slider_menu_item);
        }
    }
};
