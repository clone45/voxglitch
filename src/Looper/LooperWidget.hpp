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

        // Load and apply theme
        theme.load("looper");
        applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/looper/looper_panel.svg"),
            asset::plugin(pluginInstance, "res/looper/looper_panel-dark.svg")
        ));

        // Add output jacks
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(22.3819, 303.249878), module, Looper::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(22.3819, 354.949951), module, Looper::AUDIO_OUTPUT_RIGHT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(22.2933, 52.0), module, Looper::RESET_INPUT));

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
    }
};
