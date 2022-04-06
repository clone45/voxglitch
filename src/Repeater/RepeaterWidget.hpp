struct RepeaterWidget : ModuleWidget
{
	RepeaterWidget(Repeater* module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/repeater/repeater_front_panel.svg")));

    // Load up the background PNG and add it to the panel

    PNGPanel *png_panel = new PNGPanel("res/repeater/repeater_base_blur.png", 81.28, 128.5);
    // PNGPanel *png_panel = new PNGPanel("res/repeater/repeater_base.png", 81.28, 128.5);
    addChild(png_panel);


    // Add typography layer
    std::shared_ptr<Svg> svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/repeater/repeater_typography.svg"));
    VoxglitchPanel *voxglitch_panel = new VoxglitchPanel;
    voxglitch_panel->setBackground(svg);
    addChild(voxglitch_panel);

    // Medium Knobs
		addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.563736,122.269234), module, Repeater::CLOCK_DIVISION_KNOB));
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.375656,191.931137), module, Repeater::POSITION_KNOB));
		addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.425659,261.852417), module, Repeater::SAMPLE_SELECT_KNOB));
    addParam(createParamCentered<VoxglitchMediumKnob>(Vec(126.402100,332.388794), module, Repeater::PITCH_KNOB));

    // CV Inputs
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,121.665604), module, Repeater::CLOCK_DIVISION_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,191.426514), module, Repeater::POSITION_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,261.237152), module, Repeater::SAMPLE_SELECT_INPUT));
    addInput(createInputCentered<VoxglitchInputPort>(Vec(42.219269,331.698181), module, Repeater::PITCH_INPUT));

    // Attenuators
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,121.868500), module, Repeater::CLOCK_DIVISION_ATTN_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,191.479111), module, Repeater::POSITION_ATTN_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,261.189728), module, Repeater::SAMPLE_SELECT_ATTN_KNOB));
    addParam(createParamCentered<VoxglitchAttenuator>(Vec(83.563423,331.700439), module, Repeater::PITCH_ATTN_KNOB));

    // Clock input
    addInput(createInputCentered<VoxglitchInputPort>(Vec(196.701385,121.515594), module, Repeater::TRIG_INPUT));

    // Smooth switch
    addParam(createParamCentered<squareToggle>(Vec(197.249832,193.950241), module, Repeater::SMOOTH_SWITCH));

    // Outputs
    addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(204.649963,331.449402), module, Repeater::WAV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(204.699982,281.450653), module, Repeater::TRG_OUTPUT));

		Readout *readout = new Readout();
		readout->box.pos = mm2px(Vec(10.0, 19.5)); //22,22
		readout->box.size = Vec(110, 30); // bounding box of the widget
		readout->module = module;
		addChild(readout);
	}

	void appendContextMenu(Menu *menu) override
	{
		Repeater *module = dynamic_cast<Repeater*>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		for(int i=0; i < NUMBER_OF_SAMPLES; i++)
		{
			MenuItemLoadSample *menu_item_load_sample = new MenuItemLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i+1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}

		//
		// Options
		// =====================================================================

		menu->addChild(new MenuEntry);
		menu->addChild(createMenuLabel("Options"));

		// Retrigger option

		struct RetriggerMenuItem : MenuItem {
			Repeater* module;
			void onAction(const event::Action& e) override {
				module->retrigger = !(module->retrigger);
			}
		};

		RetriggerMenuItem* retrigger_menu_item = createMenuItem<RetriggerMenuItem>("Retrigger");
		retrigger_menu_item->rightText = CHECKMARK(module->retrigger == 1);
		retrigger_menu_item->module = module;
		menu->addChild(retrigger_menu_item);
	}

};
