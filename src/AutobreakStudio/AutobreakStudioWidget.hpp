struct AutobreakStudioWidget : VoxglitchSamplerModuleWidget
{
	AutobreakStudio *module;

	AutobreakStudioWidget(AutobreakStudio *module)
	{
		this->module = module;
		setModule(module);

		// Load and apply theme
		// theme.load("autobreak_studio");
		// applyTheme();

		setPanel(createPanel(
			asset::plugin(pluginInstance, "res/autobreak_studio/autobreak_studio_panel.svg"),
			asset::plugin(pluginInstance, "res/autobreak_studio/autobreak_studio_panel-dark.svg")
		));

		// =================== PLACE COMPONENTS ====================================

		// Screws
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		VoltageSequencerDisplayABS *position_sequencer_display = new VoltageSequencerDisplayABS(& this->module->position_sequencer, 0);
		position_sequencer_display->box.pos = Vec(157.778, 75.59056); // {"x": 157.778, "y": 75.59056 }
		position_sequencer_display->module = module;
		addChild(position_sequencer_display);

		if(module)
		{
			VoltageSequencerDisplayABS *sample_sequencer_display = new VoltageSequencerDisplayABS(& this->module->sample_sequencer, 1);
			sample_sequencer_display->box.pos = Vec(157.778, 75.59056);
			sample_sequencer_display->module = module;
			sample_sequencer_display->hide();
			addChild(sample_sequencer_display);	

			VoltageSequencerDisplayABS *volume_sequencer_display = new VoltageSequencerDisplayABS(& this->module->volume_sequencer, 2);
			volume_sequencer_display->box.pos = Vec(157.778, 75.59056);
			volume_sequencer_display->module = module;
			volume_sequencer_display->hide();
			addChild(volume_sequencer_display);

			VoltageSequencerDisplayABS *pan_sequencer_display = new VoltageSequencerDisplayABS(& this->module->pan_sequencer, 3);
			pan_sequencer_display->box.pos = Vec(157.778, 75.59056);
			pan_sequencer_display->module = module;
			pan_sequencer_display->hide();
			pan_sequencer_display->draw_horizontal_guide = true;
			addChild(pan_sequencer_display);	

			VoltageToggleSequencerDisplay *reverse_sequencer_display = new VoltageToggleSequencerDisplay(& this->module->reverse_sequencer, 4);
			reverse_sequencer_display->box.pos = Vec(157.778, 75.59056);
			reverse_sequencer_display->module = module;
			reverse_sequencer_display->hide();
			addChild(reverse_sequencer_display);

			VoltageSequencerDisplayABS *ratchet_sequencer_display = new VoltageSequencerDisplayABS(& this->module->ratchet_sequencer, 5);
			ratchet_sequencer_display->box.pos = Vec(157.778, 75.59056);
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
			lcd_tabs_widget->box.pos = Vec(157.778, 50.59056);
			lcd_tabs_widget->box.size = Vec(DRAW_AREA_WIDTH, LCD_TABS_HEIGHT);
			lcd_tabs_widget->module = module;
			addChild(lcd_tabs_widget);

			for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
			{
				WaveformWidget *waveform_widget = new WaveformWidget(WAVEFORM_WIDGET_WIDTH, WAVEFORM_WIDGET_HEIGHT, &this->module->waveform_model[i]);
				waveform_widget->box.pos = Vec(157.778, 232.61225);
				waveform_widget->hide();
				addChild(waveform_widget);
			}
		}
		else
		{
			std::string placeholder_waveform_file_path = "res/autobreak_studio/themes/default/waveform-placeholder.jpg";

			ImageWidget *image_widget = new ImageWidget(placeholder_waveform_file_path, 900.0, 109.0, 1.0, 0.15);
			image_widget->box.pos =Vec(157.778, 232.61225);
			addChild(image_widget);

			LcdTabsWidget *lcd_tabs_widget = new LcdTabsWidget(true);
			lcd_tabs_widget->box.pos = Vec(157.778, 50.59056);
			lcd_tabs_widget->box.size = Vec(DRAW_AREA_WIDTH, LCD_TABS_HEIGHT);
			lcd_tabs_widget->module = module;
			addChild(lcd_tabs_widget);			
		}

		addInput(createInputCentered<VoxglitchInputPort>(Vec(30.12117, 71.68296), module, AutobreakStudio::CLOCK_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(73.53734, 71.68296), module, AutobreakStudio::RESET_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(117.07335, 71.68296), module, AutobreakStudio::RATCHET_INPUT));

		addInput(createInputCentered<VoxglitchInputPort>(Vec(35.5389, 141.75000), module, AutobreakStudio::MEMORY_SELECT_INPUT));

		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(Vec(35.5389, 196.00), module, AutobreakStudio::COPY_BUTTON, AutobreakStudio::COPY_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(Vec(35.5389, 240.00), module, AutobreakStudio::CLEAR_BUTTON, AutobreakStudio::CLEAR_LIGHT));

		// Add inputs for CV sequencer override
		addInput(createInputCentered<VoxglitchInputPort>(Vec(175.6875, 27.12069), module, AutobreakStudio::POSITION_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(242.2439, 27.12069), module, AutobreakStudio::SAMPLE_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(308.4915, 27.12069), module, AutobreakStudio::VOLUME_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(375.3137, 27.12069), module, AutobreakStudio::PAN_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(441.8311, 27.12069), module, AutobreakStudio::REVERSE_CV_INPUT));
		addInput(createInputCentered<VoxglitchInputPort>(Vec(508.3613, 27.12069), module, AutobreakStudio::RATCHET_CV_INPUT));

		// Add outputs for CV sequencers
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(206.7689, 27.12069), module, AutobreakStudio::POSITION_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(273.0165, 27.12069), module, AutobreakStudio::SAMPLE_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(339.8387, 27.12069), module, AutobreakStudio::VOLUME_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(406.3561, 27.12069), module, AutobreakStudio::PAN_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(472.8863, 27.12069), module, AutobreakStudio::REVERSE_CV_OUTPUT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(539.4166, 27.12069), module, AutobreakStudio::RATCHET_CV_OUTPUT));

		addParam(createParamCentered<squareToggle>(Vec(76.0244, 127.28123), module, AutobreakStudio::MEMORY_BUTTONS + 0));
		addParam(createParamCentered<squareToggle>(Vec(76.0244, 157.9642), module, AutobreakStudio::MEMORY_BUTTONS + 1));
		addParam(createParamCentered<squareToggle>(Vec(76.0244, 188.64717), module, AutobreakStudio::MEMORY_BUTTONS + 2));
		addParam(createParamCentered<squareToggle>(Vec(76.0244, 219.33014), module, AutobreakStudio::MEMORY_BUTTONS + 3));
		addParam(createParamCentered<squareToggle>(Vec(76.0244, 250.01311), module, AutobreakStudio::MEMORY_BUTTONS + 4));
		addParam(createParamCentered<squareToggle>(Vec(76.0244, 280.69608), module, AutobreakStudio::MEMORY_BUTTONS + 5));
		addParam(createParamCentered<squareToggle>(Vec(76.0244, 311.37905), module, AutobreakStudio::MEMORY_BUTTONS + 6));
		addParam(createParamCentered<squareToggle>(Vec(76.0244, 342.06202), module, AutobreakStudio::MEMORY_BUTTONS + 7));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 127.28123), module, AutobreakStudio::MEMORY_BUTTONS + 8));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 157.9642), module, AutobreakStudio::MEMORY_BUTTONS + 9));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 188.64717), module, AutobreakStudio::MEMORY_BUTTONS + 10));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 219.33014), module, AutobreakStudio::MEMORY_BUTTONS + 11));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 250.01311), module, AutobreakStudio::MEMORY_BUTTONS + 12));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 280.69608), module, AutobreakStudio::MEMORY_BUTTONS + 13));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 311.37905), module, AutobreakStudio::MEMORY_BUTTONS + 14));
		addParam(createParamCentered<squareToggle>(Vec(109.7851, 342.06202), module, AutobreakStudio::MEMORY_BUTTONS + 15));

		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(178.6329, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 0));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(222.2084, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 1));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(265.7839, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 2));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(309.3594, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 3));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(352.9349, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 4));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(396.5104, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 5));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(440.0859, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 6));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(483.6614, 315.53934), module, AutobreakStudio::LEFT_INDIVIDUAL_OUTPUTS + 7));

		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(178.6329, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 0));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(222.2084, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 1));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(265.7839, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 2));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(309.3594, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 3));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(352.9349, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 4));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(396.5104, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 5));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(440.0859, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 6));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(483.6614, 354.33637), module, AutobreakStudio::RIGHT_INDIVIDUAL_OUTPUTS + 7));

		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(530.0, 315.53934), module, AutobreakStudio::AUDIO_OUTPUT_LEFT));
		addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(530.0, 354.33637), module, AutobreakStudio::AUDIO_OUTPUT_RIGHT));
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
