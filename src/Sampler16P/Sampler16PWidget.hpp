struct Sampler16PWidget : VoxglitchSamplerModuleWidget
{
    Sampler16PWidget(Sampler16P *module)
    {
        setModule(module);

        // Load and apply theme
        // theme.load("sampler16p");
        // applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/sampler16p/sampler16p_panel.svg"),
            asset::plugin(pluginInstance, "res/sampler16p/sampler16p_panel-dark.svg")
        ));

        addInput(createInputCentered<VoxglitchInputPort>(Vec(22.2933, 52.0), module, Sampler16P::TRIGGER_INPUTS));

        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(22.3819, 304.1339), module, Sampler16P::AUDIO_MIX_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(22.3819, 340.1575), module, Sampler16P::AUDIO_MIX_OUTPUT_RIGHT));
    }

    void appendContextMenu(Menu *menu) override
    {
        Sampler16P *module = dynamic_cast<Sampler16P *>(this->module);
        assert(module);

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Load individual samples"));

        for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            Sampler16PLoadSample *menu_item_load_sample = new Sampler16PLoadSample();
            menu_item_load_sample->sample_number = i;
            menu_item_load_sample->text = std::to_string(i + 1) + ": " + module->loaded_filenames[i];
            menu_item_load_sample->module = module;
            menu->addChild(menu_item_load_sample);
        }

        menu->addChild(new MenuEntry); // For spacing only
        menu->addChild(createMenuLabel("Or.."));

        Sampler16PLoadFolder *menu_item_load_folder = new Sampler16PLoadFolder();
        menu_item_load_folder->text = "Load first 16 WAV files from a folder";
        menu_item_load_folder->module = module;
        menu->addChild(menu_item_load_folder);

        // Interpolation menu
        menu->addChild(new MenuEntry); // For spacing only
        SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
        sample_interpolation_menu_item->module = module;
        menu->addChild(sample_interpolation_menu_item);
    }
};
