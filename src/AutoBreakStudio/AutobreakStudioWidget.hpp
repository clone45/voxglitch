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

		if(module)
		{
			VoltageSequencerDisplayABS *position_sequencer = new VoltageSequencerDisplayABS(this->module->selected_position_sequencer);
			position_sequencer->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			position_sequencer->module = module;
			addChild(position_sequencer);

			VoltageSequencerDisplayABS *volume_sequencer = new VoltageSequencerDisplayABS(this->module->selected_volume_sequencer);
			volume_sequencer->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			volume_sequencer->module = module;
			volume_sequencer->hide();
			addChild(volume_sequencer);

			VoltageSequencerDisplayABS *sample_sequencer = new VoltageSequencerDisplayABS(this->module->selected_sample_sequencer);
			sample_sequencer->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			sample_sequencer->module = module;
			sample_sequencer->hide();
			addChild(sample_sequencer);		

			VoltageSequencerDisplayABS *pan_sequencer = new VoltageSequencerDisplayABS(this->module->selected_pan_sequencer);
			pan_sequencer->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			pan_sequencer->module = module;
			pan_sequencer->hide();
			addChild(pan_sequencer);	

			VoltageSequencerDisplayABS *ratchet_sequencer = new VoltageSequencerDisplayABS(this->module->selected_ratchet_sequencer);
			ratchet_sequencer->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			ratchet_sequencer->module = module;
			ratchet_sequencer->hide();
			addChild(ratchet_sequencer);

			VoltageToggleSequencerDisplay *reverse_sequencer = new VoltageToggleSequencerDisplay(this->module->selected_reverse_sequencer);
			reverse_sequencer->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y);
			reverse_sequencer->module = module;
			reverse_sequencer->hide();
			addChild(reverse_sequencer);

			// Here, how about creating an array of pointers to the sequencer displays, and pass that array into LcdTabsWidget, which 
			// will give it the ability to call the sequencer displays' show and hide methods?

			SequencerDisplayABS *sequencer_displays[NUMBER_OF_SEQUENCERS] = {
				position_sequencer,
				sample_sequencer,
				volume_sequencer,
				pan_sequencer,
				reverse_sequencer,
				ratchet_sequencer
			};

			LcdTabsWidget *lcd_tabs_widget = new LcdTabsWidget(sequencer_displays);
			lcd_tabs_widget->box.pos = Vec(DRAW_AREA_POSITION_X, DRAW_AREA_POSITION_Y - 25);
			lcd_tabs_widget->module = module;
			addChild(lcd_tabs_widget);

			for(unsigned int i=0; i<NUMBER_OF_SAMPLES; i++)
			{
				WaveformWidget *waveform_widget = new WaveformWidget(this->module, i);
				waveform_widget->box.pos = Vec(DRAW_AREA_POSITION_X, 222.0);
				waveform_widget->hide();
				addChild(waveform_widget);
			}

		}

		addInput(createInputCentered<VoxglitchInputPort>(themePos("CLOCK_INPUT"), module, AutobreakStudio::CLOCK_INPUT));
		// addInput(createInputCentered<VoxglitchInputPort>(themePos("RESET_INPUT"), module, AutobreakStudio::RESET_INPUT));

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
		SampleInterpolationMenuItem *sample_interpolation_menu_item = createMenuItem<SampleInterpolationMenuItem>("Interpolation", RIGHT_ARROW);
		sample_interpolation_menu_item->module = module;
		menu->addChild(sample_interpolation_menu_item);
	}
};
