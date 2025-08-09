struct VoxglitchSliderLong : app::SvgSlider
{
    VoxglitchSliderLong()
    {
        maxHandlePos = mm2px(Vec(-.10, -3.0));
        minHandlePos = mm2px(Vec(-.10, 36.0));
        setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/VoxglitchSliderLong.svg")));
        setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/VoxglitchSliderHandle.svg")));
    }
};

struct LooperWidget : VoxglitchSamplerModuleWidget
{
    LooperWidget(Looper *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/looper/looper_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/looper/looper_panel-dark.svg")
        );

        // Add output jacks
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"), module, Looper::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, Looper::AUDIO_OUTPUT_RIGHT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, Looper::RESET_INPUT));

        // Add volume slider
        addParam(createParamCentered<VoxglitchSliderLong>(Vec(22.3228, 177.1654), module, Looper::VOLUME_SLIDER));
    }

    void appendContextMenu(Menu *menu) override
    {
        Looper *module = dynamic_cast<Looper *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Sample"));

        // Add the sample slot to the right-click context menu
        LooperLoadSample *menu_item_load_sample = new LooperLoadSample();
        menu_item_load_sample->text = module->loaded_filename;
        menu_item_load_sample->module = module;
        menu->addChild(menu_item_load_sample);

        SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
        sample_interpolation_menu_item->module = module;
        menu->addChild(sample_interpolation_menu_item);

        menu->addChild(new MenuEntry); // For spacing
        menu->addChild(createMenuLabel("Output"));

        // Add output range selector
        OutputRangeItem *output_range_item = createMenuItem<OutputRangeItem>("Output Range", RIGHT_ARROW);
        output_range_item->module = module;
        menu->addChild(output_range_item);
    }

    struct OutputRangeValueItem : MenuItem
    {
        Looper *module;
        int range_index = 0;

        void onAction(const event::Action &e) override
        {
            module->voltage_range_index = range_index;
        }
    };

    struct OutputRangeItem : MenuItem
    {
        Looper *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            for (unsigned int i = 0; i < 8; i++)
            {
                OutputRangeValueItem *output_range_value_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->voltage_range_index == i));
                output_range_value_item->module = module;
                output_range_value_item->range_index = i;
                menu->addChild(output_range_value_item);
            }

            return menu;
        }
    };
};
