struct AutobreakStudioWidget : VoxglitchSamplerModuleWidget
{
	AutobreakStudio *module;

	AutobreakStudioWidget(AutobreakStudio *module)
	{
		this->module = module;
		setModule(module);

		// Load and apply theme
		theme.load("autobreak_studio");
		applyTheme();

		// =================== PLACE COMPONENTS ====================================


		VoltageSequencerDisplayABS *position_sequencer_display = new VoltageSequencerDisplayABS(& this->module->position_sequencer);
		position_sequencer_display->box.pos = themePos("SEQUENCER_DISPLAY");
		position_sequencer_display->module = module;
		addChild(position_sequencer_display);

		if(module)
		{
			VoltageSequencerDisplayABS *sample_sequencer_display = new VoltageSequencerDisplayABS(& this->module->sample_sequencer);
			sample_sequencer_display->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			sample_sequencer_display->module = module;
			sample_sequencer_display->hide();
			addChild(sample_sequencer_display);	

			VoltageSequencerDisplayABS *volume_sequencer_display = new VoltageSequencerDisplayABS(& this->module->volume_sequencer);
			volume_sequencer_display->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			volume_sequencer_display->module = module;
			volume_sequencer_display->hide();
			addChild(volume_sequencer_display);

			VoltageSequencerDisplayABS *pan_sequencer_display = new VoltageSequencerDisplayABS(& this->module->pan_sequencer);
			pan_sequencer_display->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			pan_sequencer_display->module = module;
			pan_sequencer_display->hide();
			pan_sequencer_display->draw_horizontal_guide = true;
			addChild(pan_sequencer_display);	

			VoltageToggleSequencerDisplay *reverse_sequencer_display = new VoltageToggleSequencerDisplay(& this->module->reverse_sequencer);
			reverse_sequencer_display->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			reverse_sequencer_display->module = module;
			reverse_sequencer_display->hide();
			addChild(reverse_sequencer_display);

			VoltageSequencerDisplayABS *ratchet_sequencer_display = new VoltageSequencerDisplayABS(& this->module->ratchet_sequencer);
			ratchet_sequencer_display->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			ratchet_sequencer_display->module = module;
			ratchet_sequencer_display->hide();
			addChild(ratchet_sequencer_display);

			// Here, how about creating an array of pointers to the sequencer displays, and pass that array into LcdTabsWidget, which 
			// will give it the ability to call the sequencer displays' show and hide methods?

			SequencerDisplayABS *sequencer_displays[NUMBER_OF_SEQUENCERS] = {
				position_sequencer_display,
				sample_sequencer_display,
				volume_sequencer_display,
				pan_sequencer_display,
				reverse_sequencer_display,
				ratchet_sequencer_display
			};


			LcdTabsWidget *lcd_tabs_widget = new LcdTabsWidget(sequencer_displays);
			lcd_tabs_widget->box.pos = themePos("TABS_DISPLAY");
			lcd_tabs_widget->module = module;
			addChild(lcd_tabs_widget);

			for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
			{
				WaveformWidget *waveform_widget = new WaveformWidget(this->module, i);
				waveform_widget->box.pos = themePos("WAVEFORM_DISPLAY");
				waveform_widget->hide();
				addChild(waveform_widget);
			}
		}
		else
		{
			std::string placeholder_waveform_file_path = asset::plugin(pluginInstance, "res/autobreak_studio/themes/default/waveform-placeholder.jpg");

			ImageWidget *image_widget = new ImageWidget(placeholder_waveform_file_path, 900.0, 109.0, 1.0, 0.15);
			image_widget->box.pos = themePos("WAVEFORM_DISPLAY");
			addChild(image_widget);

			LcdTabsWidget *lcd_tabs_widget = new LcdTabsWidget(true);
			lcd_tabs_widget->box.pos = themePos("TABS_DISPLAY");
			lcd_tabs_widget->box.size = Vec(DRAW_AREA_WIDTH, LCD_TABS_HEIGHT);
			lcd_tabs_widget->module = module;
			addChild(lcd_tabs_widget);			
		}

		addInput(createInputCentered<VoxglitchInputPort>(themePos("CLOCK_INPUT"), module, AutobreakStudio::CLOCK_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, AutobreakStudio::RESET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("RATCHET_INPUT"), module, AutobreakStudio::RATCHET_INPUT));

		addInput(createInputCentered<VoxglitchInputPort>(themePos("MEMORY_SELECT_INPUT"), module, AutobreakStudio::MEMORY_SELECT_INPUT));

		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(themePos("COPY_BUTTON"), module, AutobreakStudio::COPY_BUTTON, AutobreakStudio::COPY_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(themePos("CLEAR_BUTTON"), module, AutobreakStudio::CLEAR_BUTTON, AutobreakStudio::CLEAR_LIGHT));

		// Add inputs for CV sequencer override
		addInput(createInputCentered<VoxglitchInputPort>(themePos("POSITION_CV_INPUT"), module, AutobreakStudio::POSITION_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("SAMPLE_CV_INPUT"), module, AutobreakStudio::SAMPLE_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("VOLUME_CV_INPUT"), module, AutobreakStudio::VOLUME_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("PAN_CV_INPUT"), module, AutobreakStudio::PAN_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("REVERSE_CV_INPUT"), module, AutobreakStudio::REVERSE_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(themePos("RATCHET_CV_INPUT"), module, AutobreakStudio::RATCHET_CV_INPUT));

		// Add outputs for CV sequencers
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("POSITION_CV_OUTPUT"), module, AutobreakStudio::POSITION_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("SAMPLE_CV_OUTPUT"), module, AutobreakStudio::SAMPLE_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("VOLUME_CV_OUTPUT"), module, AutobreakStudio::VOLUME_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("PAN_CV_OUTPUT"), module, AutobreakStudio::PAN_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("REVERSE_CV_OUTPUT"), module, AutobreakStudio::REVERSE_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RATCHET_CV_OUTPUT"), module, AutobreakStudio::RATCHET_CV_OUTPUT));

		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_1"), module, AutobreakStudio::MEMORY_BUTTONS + 0));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_2"), module, AutobreakStudio::MEMORY_BUTTONS + 1));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_3"), module, AutobreakStudio::MEMORY_BUTTONS + 2));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_4"), module, AutobreakStudio::MEMORY_BUTTONS + 3));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_5"), module, AutobreakStudio::MEMORY_BUTTONS + 4));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_6"), module, AutobreakStudio::MEMORY_BUTTONS + 5));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_7"), module, AutobreakStudio::MEMORY_BUTTONS + 6));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_8"), module, AutobreakStudio::MEMORY_BUTTONS + 7));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_9"), module, AutobreakStudio::MEMORY_BUTTONS + 8));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_10"), module, AutobreakStudio::MEMORY_BUTTONS + 9));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_11"), module, AutobreakStudio::MEMORY_BUTTONS + 10));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_12"), module, AutobreakStudio::MEMORY_BUTTONS + 11));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_13"), module, AutobreakStudio::MEMORY_BUTTONS + 12));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_14"), module, AutobreakStudio::MEMORY_BUTTONS + 13));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_15"), module, AutobreakStudio::MEMORY_BUTTONS + 14));
		addParam(createParamCentered<squareToggle>(themePos("MEMORY_BUTTON_16"), module, AutobreakStudio::MEMORY_BUTTONS + 15));

		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_1"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 0));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_2"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 1));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_3"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 2));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_4"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 3));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_5"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 4));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_6"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 5));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_7"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 6));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("LEFT_OUTPUT_8"), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 7));

		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_1"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 0));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_2"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 1));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_3"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 2));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_4"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 3));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_5"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 4));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_6"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 5));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_7"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 6));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("RIGHT_OUTPUT_8"), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 7));

		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_LEFT"), module, AutobreakStudio::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("AUDIO_OUTPUT_RIGHT"), module, AutobreakStudio::AUDIO_OUTPUT_RIGHT));
	}

	void appendContextMenu(Menu *menu) override
	{
		AutobreakStudio *module = dynamic_cast<AutobreakStudio *>(this->module);
		assert(module);

		menu->addChild(new MenuEntry); // For spacing only
		menu->addChild(createMenuLabel("Samples"));

		//
		// Add the five "Load Sample.." menu options to the right-click context menu
		//

		std::string menu_text = "#";

		for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
		{
			AutobreakStudioLoadSample *menu_item_load_sample = new AutobreakStudioLoadSample;
			menu_item_load_sample->sample_number = i;
			menu_item_load_sample->text = std::to_string(i + 1) + ": " + module->loaded_filenames[i];
			menu_item_load_sample->module = module;
			menu->addChild(menu_item_load_sample);
		}

		menu->addChild(new MenuEntry); // For spacing only

		// Add folder loading menu
		AutobreakStudioLoadFolder *menu_item_load_folder = new AutobreakStudioLoadFolder();
		menu_item_load_folder->text = "Load first 8 WAV files from a folder";
		menu_item_load_folder->module = module;
		menu->addChild(menu_item_load_folder);

		//
		// Add interpolation menu
		//
		menu->addChild(new MenuEntry); // For spacing only
		SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
		sample_interpolation_menu_item->module = module;
		menu->addChild(sample_interpolation_menu_item);
	}
};
