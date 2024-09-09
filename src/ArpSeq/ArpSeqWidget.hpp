struct NumberChooser : app::SvgKnob
{
    std::vector<std::shared_ptr<Svg>> framesSvg; // Vector to hold Svg pointers
    int *cycle_count_down_ptr = NULL;

    NumberChooser()
    {
        // Reserve space for 16 frames
        framesSvg.reserve(16);

        // Load SVGs
        for (int i = 0; i < 16; ++i)
        {
            // Use string manipulation to create the file path dynamically
            std::string filePath = "res/arpseq/readout/" + std::to_string(i + 1) + ".svg";
            framesSvg.push_back(APP->window->loadSvg(asset::plugin(pluginInstance, filePath)));
        }

        // Set initial SVG
        setSvg(framesSvg[0]);

        shadow->box.pos = Vec(0.0, 1.0);
        this->tw = new TransformWidget();
        this->shadow->opacity = 0.0;
    }

    void setCycleCountDownPtr(int *ptr)
    {
        // Store the pointer to the cycle_count_down value
        cycle_count_down_ptr = ptr;
        
        // Initialize the value of the knob to the value of the cycle_count_down_ptr
        *cycle_count_down_ptr = static_cast<int>(getParamQuantity()->getValue() - 1);
    }

    // Override draw and call the parent draw function
    void draw(const DrawArgs &args) override
    {
        SvgKnob::draw(args);

        if(cycle_count_down_ptr != NULL)
        {
            // Draw a rectangle around this knob with a hight that is proportional to the cycle value
            float cycle_count_down_value = *cycle_count_down_ptr;

            int max_cycle = static_cast<int>(getParamQuantity()->getValue() - 1);
            setSvg(framesSvg[max_cycle]);

            if(cycle_count_down_value > 0)
            {
                // The width and height are 21.0 and 21.0
                float width = 21.0;
                float height = 21.0;

                // Calculate the height of the rectangle
                float rect_height = height * (cycle_count_down_value / 16.0);

                // Calculate the position of the rectangle
                float x = 0.0;
                float y = height - rect_height;

                nvgSave(args.vg);

                // Draw the rectangle
                nvgBeginPath(args.vg);
                nvgRect(args.vg, x, y, width, rect_height);
                nvgFillColor(args.vg, nvgRGBA(255, 215, 20, 50));
                nvgFill(args.vg);           

                nvgRestore(args.vg);
            }
        }
    }

    void onButton(const ButtonEvent &e) override
    {
        ParamWidget::onButton(e);
    }

    void onHover(const HoverEvent &e) override
    {
        GLFWcursor *cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        glfwSetCursor(APP->window->win, cursor);

        ParamWidget::onHover(e);
    }

    void onLeave(const event::Leave& e) override {
        ParamWidget::onLeave(e);

        glfwSetCursor(APP->window->win, NULL);
    }

    void appendContextMenu(Menu *menu) override
    {
        ArpSeq *module = dynamic_cast<ArpSeq *>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Reset Cycles", "", [=]() {
            module->resetPageCycleCounters();
        }));

        menu->addChild(createMenuItem("Smart Randomize", "", [=]() {
            module->smartRandomizePageCycleCounters();
        }));
    }
};

class CustomPanelBorder : public PanelBorder {
public:
    void draw(const DrawArgs &args) override {
        // don't draw anything
    }
};

struct ArpSeqWidget : ModuleWidget
{

    // Adding a Label pointer for shape readout
    // rack::ui::Label *shape_readout_label;

    ArpSeqWidget(ArpSeq *module)
    {
        setModule(module);

        // theme.load("ArpSeq");
        // applyTheme();

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/arpseq/arpseq_panel.svg"),
            asset::plugin(pluginInstance, "res/arpseq/arpseq_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        ArpSeq *arp_module = dynamic_cast<ArpSeq *>(module);

        // Create TabsWidget and set its position.  The module->page variable is set
        // to the selected tab index in the TabsWidget::onButton function.  Another way to have done this may have been
        // to pass in a pointer to a function that gets called anytime a tab is selected.
        TabsWidget *tabs_widget = new TabsWidget(&module->page);
        tabs_widget->box.pos = Vec(138.89827, 53.577);
        addChild(tabs_widget); // Add tabs_widget to ArpSeqWidget

        std::string tab_labels[NUMBER_OF_PAGES] = {
            "Gate",
            "Transpose",
            "Mod 1",
            "Mod 2"};

        // beginning of the stuff I want to contain in tabs
        if (module)
        {
            for (unsigned int page_index = 0; page_index < 4; page_index++)
            {
                std::vector<Widget *> widgets_for_tab; // A vector to hold widgets for this tab

                // --- Voltage sequencer display ---------------------

                SequencerDisplayConfig *cfg = new SequencerDisplayConfig();
                cfg->x = 237.10121;
                cfg->y = 80.70864;
                cfg->overlay_width = 363.03758;
                cfg->overlay_height = 211.71036;
                cfg->draw_area_width = 363.03758;
                cfg->draw_area_height = 211.71036;
                cfg->bar_height = 211.71036;
                cfg->bar_horizontal_padding = 0.8;
                cfg->y_axis = (cfg->draw_area_height / 2.0);
                cfg->max_sequencer_steps = 16;
                cfg->visible = false;
                cfg->y_axis = cfg->draw_area_height;

                ArpVoltageSequencerDisplay *arp_sequencer_display = new ArpVoltageSequencerDisplay(&arp_module->pages[page_index].voltage_sequencer, cfg);
                arp_sequencer_display->box.pos = Vec(cfg->x, cfg->y);
                arp_sequencer_display->module = module;
                arp_sequencer_display->tooltipCallback = module->tooltip_callbacks[page_index];
                addChild(arp_sequencer_display);

                widgets_for_tab.push_back(arp_sequencer_display);

                // --- Chance sequencer display ---------------------

                SequencerDisplayConfig *pcent_cfg = new SequencerDisplayConfig();
                pcent_cfg->x = 237.10121;
                pcent_cfg->y = 298.82900;
                pcent_cfg->overlay_width = 363.03758;
                pcent_cfg->overlay_height = 21.171;
                pcent_cfg->draw_area_width = 363.03758;
                pcent_cfg->draw_area_height = 21.171;
                pcent_cfg->bar_height = 21.171;
                pcent_cfg->bar_horizontal_padding = 0.8;
                pcent_cfg->y_axis = pcent_cfg->draw_area_height;
                pcent_cfg->max_sequencer_steps = 16;

                ArpVoltageSequencerDisplay *chance_sequencer_display = new ArpVoltageSequencerDisplay(&arp_module->pages[page_index].chance_sequencer, pcent_cfg);
                chance_sequencer_display->box.pos = Vec(pcent_cfg->x, pcent_cfg->y);
                chance_sequencer_display->module = module;
                chance_sequencer_display->tooltipCallback = std::bind(&ArpSeq::getTooltipTextForPercentage, module, std::placeholders::_1, std::placeholders::_2);
                addChild(chance_sequencer_display);

                widgets_for_tab.push_back(chance_sequencer_display);

                // --- Window (Start, End) display -------------------------------
                if (module)
                {
                    ArpSeqWindow *arp_seq_window = new ArpSeqWindow(&module->pages[page_index].voltage_sequencer, &module->pages[page_index].chance_sequencer);
                    arp_seq_window->box.pos = Vec(237.10121, 61.16473);
                    arp_seq_window->module = module;
                    addChild(arp_seq_window);
                    widgets_for_tab.push_back(arp_seq_window);
                }

                // --- Cycle Knobs ----------------------------------

                float padding = 1.80;     // Calculated padding and spacing between knobs
                float knob_width_px = 21; // Width of a knob
                float knob_height_px = 21;
                float startX = 247.55; // Starting X position

                for (int i = 0; i < 16; i++)
                {
                    int cycle_knob_index = ArpSeq::CYCLE_KNOBS + (page_index * 16) + i;
                    ParamWidget *number_chooser = createParamCentered<NumberChooser>(Vec(startX, 336.7), module, cycle_knob_index);
                    number_chooser->box.size = Vec(knob_width_px, knob_height_px);

                    int *count_down_pointer = module->pages[page_index].cycle_counters->getCountDownPointer(i);

                    if(!count_down_pointer)
                    {
                        DEBUG("count_down_pointer is NULL");
                    }

                    addParam(number_chooser);

                    dynamic_cast<NumberChooser*>(number_chooser)->setCycleCountDownPtr(count_down_pointer);
                    
                    widgets_for_tab.push_back(number_chooser);     // Collect the knob widgets for the current tab
                    startX += (knob_width_px + padding); // Move to the next position
                }

                // --- Top Controls     -----------------------------

                if (page_index == GATE_SEQUENCER)
                {
                    DigitalToggle *digital_toggle = new DigitalToggle(&module->sample_and_hold_mode);
                    DigitalControl *sample_and_hold_control = new DigitalControl(
                        Vec(481.0, 27.5),
                        "Sample & Hold",
                        "Sample & Hold",
                        digital_toggle,
                        module);
                    addChild(sample_and_hold_control);

                    DigitalToggle *step_after_arp_toggle = new DigitalToggle(&module->step_mode);
                    DigitalControl *step_after_arp_control = new DigitalControl(
                        Vec(365.0, 27.5),
                        "Step After Arp",
                        "Step After Arp",
                        step_after_arp_toggle,
                        module);
                    addChild(step_after_arp_control);

                    widgets_for_tab.push_back(sample_and_hold_control);
                    widgets_for_tab.push_back(step_after_arp_control);
                }

                if (page_index == TRANSPOSE_SEQUENCER)
                {
                    /*
                    float offset = 26.5;

                    // 
                    // Add the transpose control
                    //

                    DigitalToggle *digital_toggle = new DigitalToggle(&module->quantize_transpose_cv);

                    DigitalControl *quantize_control = new DigitalControl(
                        Vec(514.0 + offset, 27.5),
                        "Snap",
                        "Snap",
                        digital_toggle,
                        module);

                    addChild(quantize_control);

                    //
                    // Add the selection for scale
                    //
                    DigitalSelect *digital_select = new DigitalSelect(&module->output_quantization_scale_index);

                    DigitalControl *scale_control = new DigitalControl(
                        Vec(330 + offset, 27.5),
                        "Scale:",
                        "Scale:",
                        digital_select,
                        module);

                    addChild(scale_control);

                    widgets_for_tab.push_back(quantize_control);
                    widgets_for_tab.push_back(scale_control);
                    */

                }

                if (page_index == MOD1_SEQUENCER)
                {
                    DigitalSwitch *digital_switch = new DigitalSwitch(&module->mod1_polarity);

                    DigitalControl *polarity_control = new DigitalControl(
                        Vec(519, 27.5),
                        "Uni-Polar",
                        "Bi-Polar",
                        digital_switch,
                        module);

                    addChild(polarity_control);

                    //
                    // Add the range control
                    //

                    DigitalRangeSelector *mod1_range_selector = new DigitalRangeSelector(&module->mod1_polarity, &module->mod1_attenuation_low, &module->mod1_attenuation_high);

                    DigitalControl *mod1_range_control = new DigitalControl(
                        Vec(275, 27.5),
                        "Range: XX.X to XX.XXV",
                        "9.99 to 10.00V",
                        mod1_range_selector,
                        module);
                    addChild(mod1_range_control);

                    // 
                    // Add the slew control using a DigitalHorizontalSlider
                    //

                    SlewSlider *mod1_slew_slider = new SlewSlider(&module->mod1_slew);
                    mod1_slew_slider->tooltipCallback = std::bind(&ArpSeq::getTooltipForMod1Slew, module);
                    DigitalControl *slew_control = new DigitalControl(
                        Vec(145, 27.5),
                        "Slew:",
                        "Slew:",
                        mod1_slew_slider,
                        module);
                    addChild(slew_control);

                    //
                    // Add the digital controls to the widgets_for_tab vector
                    //

                    widgets_for_tab.push_back(polarity_control);
                    widgets_for_tab.push_back(mod1_range_control);
                    widgets_for_tab.push_back(slew_control);
                }

                if (page_index == MOD2_SEQUENCER)
                {
                    //
                    // Add the polarity control
                    //

                    DigitalSwitch *digital_switch = new DigitalSwitch(&module->mod2_polarity);

                    DigitalControl *polarity_control = new DigitalControl(
                        Vec(519, 27.5),
                        "Uni-Polar",
                        "Bi-Polar",
                        digital_switch,
                        module);

                    addChild(polarity_control);

                    //
                    // Add the range control
                    //

                    DigitalRangeSelector *mod2_range_selector = new DigitalRangeSelector(&module->mod2_polarity, &module->mod2_attenuation_low, &module->mod2_attenuation_high);

                    DigitalControl *mod2_range_control = new DigitalControl(
                        Vec(275, 27.5),
                        "Range: XX.X to XX.XXV",
                        "9.99 to 10.00V",
                        mod2_range_selector,
                        module);
                    addChild(mod2_range_control);

                    // 
                    // Add the mod2 slew control using a DigitalHorizontalSlider
                    //

                    SlewSlider *mod2_slew_slider = new SlewSlider(&module->mod2_slew);
                    mod2_slew_slider->tooltipCallback = std::bind(&ArpSeq::getTooltipForMod2Slew, module);
                    DigitalControl *slew_control = new DigitalControl(
                        Vec(145, 27.5),
                        "Slew:",
                        "Slew:",
                        mod2_slew_slider,
                        module);
                    addChild(slew_control);


                    //
                    // Add the digital controls to the widgets_for_tab vector
                    //

                    widgets_for_tab.push_back(polarity_control);
                    widgets_for_tab.push_back(mod2_range_control);
                    widgets_for_tab.push_back(slew_control);
                }

                // --------------------------------------------------

                // Finally, add this tab to TabsWidget
                tabs_widget->addTab(tab_labels[page_index], widgets_for_tab, page_index == 0);

            } // End of stuff that I want to contain in the tabs
        }

        if(!module)
        {
            // Render an svg file that looks like the digital panel for the library
            // This is a "hard coded" version of the module that is rendered when the module is not loaded
            
            panelHelper.loadPanel(
                asset::plugin(pluginInstance, "res/xy/xy_panel.svg"),
                asset::plugin(pluginInstance, "res/xy/xy_panel-dark.svg")
            );


            SvgPanel *panel_render = new SvgPanel();
            panel_render->box.pos = Vec(129.012, 13.735);
            panel_render->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/arpseq/readout/library_render.svg")));
            panel_render->panelBorder->visible = false;
            addChild(panel_render);
        }

        //
        // Set the segment readouts
        //

        SegmentReadoutWidget *rate_readout_widget = new SegmentReadoutWidget("x1");
        rate_readout_widget->box.pos = Vec(15.416, 220.745);
        if (arp_module != NULL)
            rate_readout_widget->display_string_ptr = &arp_module->rate_readout;
        addChild(rate_readout_widget);

        SegmentReadoutWidget *shape_readout_widget = new SegmentReadoutWidget("FWD");
        shape_readout_widget->box.pos = Vec(69.467, 220.745);
        if (arp_module != NULL)
            shape_readout_widget->display_string_ptr = &arp_module->shape_readout;
        addChild(shape_readout_widget);

        //
        // First column components
        //

        addInput(createInputCentered<VoxglitchPolyPort>(panelHelper.findNamed("pitch_input"), module, ArpSeq::POLY_NOTES_INPUT));
        addInput(createInputCentered<VoxglitchPolyPort>(panelHelper.findNamed("gate_input"), module, ArpSeq::POLY_GATE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(38.0, 180.0), module, ArpSeq::CLOCK_INPUT));

        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("on_switch"), module, ArpSeq::ON_SWITCH));
        addParam(createParamCentered<squareToggle>(panelHelper.findNamed("latch_switch"), module, ArpSeq::LATCH_SWITCH));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, ArpSeq::RESET_INPUT));

        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("rate_knob"), module, ArpSeq::RATE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("shape_knob"), module, ArpSeq::SHAPE_KNOB));

        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("rate_attn_knob"), module, ArpSeq::RATE_ATTENUATOR));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("shape_attn_knob"), module, ArpSeq::SHAPE_ATTENUATOR));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("rate_input"), module, ArpSeq::RATE_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("shape_input"), module, ArpSeq::SHAPE_INPUT));

        //
        // Right-most outputs
        //

        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("pitch_output"), module, ArpSeq::PITCH_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output"), module, ArpSeq::GATE_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("prob_output"), module, ArpSeq::PROBABILITY_GATE_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("cycle_output"), module, ArpSeq::CYCLE_GATE_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("mod1_output"), module, ArpSeq::MOD1_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("mod2_output"), module, ArpSeq::MOD2_OUTPUT));
    }


    struct ProbabilityOutputSettingsMenuItem : MenuItem
    {
        ArpSeq *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            menu->addChild(createIndexSubmenuItem("Source",
                {"Attach to Gate Sequencer", "Attach to Transpose Sequencer", "Attach to Mod 1 Sequencer", "Attach to Mod 2 Sequencer"},
                [=]() {
                    return (module->probability_output_sequencer_attachment);
                },
                [=](int index) {
                    module->probability_output_sequencer_attachment = index;
                }
            ));

            menu->addChild(createIndexSubmenuItem("Trigger Length",
                module->getTriggerLengthNames(),
                [=]() {
                    return (module->probability_trigger_length_index);
                },
                [=](int index) {
                    module->probability_trigger_length_index = index;
                }
            ));

            return menu;
        }
    };

    struct CycleOutputSettingsMenuItem : MenuItem
    {
        ArpSeq *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            menu->addChild(createIndexSubmenuItem("Source",
                {"Attach to Gate Sequencer", "Attach to Transpose Sequencer", "Attach to Mod 1 Sequencer", "Attach to Mod 2 Sequencer"},
                [=]() {
                    return (module->cycle_output_sequencer_attachment);
                },
                [=](int index) {
                    module->cycle_output_sequencer_attachment = index;
                }
            ));

            menu->addChild(createIndexSubmenuItem("Trigger Length",
                module->getTriggerLengthNames(),
                [=]() {
                    return (module->cycle_trigger_length_index);
                },
                [=](int index) {
                    module->cycle_trigger_length_index = index;
                }
            ));           

            return menu;
        }
    };

    struct QuantizeOutputSettingsMenuItem : MenuItem
    {
        ArpSeq *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            // Quantization Output Setting
            std::vector<std::string> scale_names = module->output_quantizer.getScaleNames();
            std::vector<std::string> note_names = module->output_quantizer.getNoteNames();

            menu->addChild(createBoolPtrMenuItem("Enable", "", &module->output_quantization));

            menu->addChild(createIndexSubmenuItem("Scale",
                scale_names,
                [=]() {
                    return (module->output_quantization_scale_index);
                },
                [=](int scale_index) {
                    module->setQuantizationScale(scale_index);
                }
            ));

            menu->addChild(createIndexSubmenuItem("Root Note",
                note_names,
                [=]() {
                    return (module->output_quantization_root_note_index);
                },
                [=](int root_note_index) {
                    module->setQuantizationRoot(root_note_index);
                }
            ));

            return menu;
        }
    };

    void appendContextMenu(Menu *menu) override
    {
        ArpSeq *module = dynamic_cast<ArpSeq *>(this->module);
        assert(module);

        // Add horizontal line
        menu->addChild(new MenuEntry);

        menu->addChild(createBoolPtrMenuItem("Legacy Reset Mode", "", &module->legacy_reset_mode));

        QuantizeOutputSettingsMenuItem *quantize_output_settings_menu_item = createMenuItem<QuantizeOutputSettingsMenuItem>("Quantize Output Settings", RIGHT_ARROW);
        quantize_output_settings_menu_item->module = module;
        menu->addChild(quantize_output_settings_menu_item);

        // Probability output sequencer settings
        ProbabilityOutputSettingsMenuItem *probability_output_settings_menu_item = createMenuItem<ProbabilityOutputSettingsMenuItem>("Probability Output Settings", RIGHT_ARROW);
        probability_output_settings_menu_item->module = module;
        menu->addChild(probability_output_settings_menu_item);

        // Cycle output sequencer settings
        CycleOutputSettingsMenuItem *cycle_output_settings_menu_item = createMenuItem<CycleOutputSettingsMenuItem>("Cycle Output Settings", RIGHT_ARROW);
        cycle_output_settings_menu_item->module = module;
        menu->addChild(cycle_output_settings_menu_item);

        menu->addChild(createIndexSubmenuItem("Rate Attenuverter Response",
            {
                "-5V to +5V",
                "-10V to +10V"
            },
            [=]() { 
                if(module->rate_attenuverter_range == 5.0) return(0);
                else return (1);
            },
            [=](int rate_attenuator_range_index) {
                module->setRateAttenuationRange(rate_attenuator_range_index);
            }
        ));

        menu->addChild(createIndexSubmenuItem("Shape Attenuverter Response",
            {
                "-5V to +5V",
                "-10V to +10V"
            },
            [=]() { 
                if(module->shape_attenuverter_range == 5.0) return(0);
                else return (1);
            },
            [=](int shape_attenuator_range_index) {
                module->setShapeAttenuationRange(shape_attenuator_range_index);
            }
        ));

    }
};