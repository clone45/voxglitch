struct NoteDetectorWidget : VoxglitchModuleWidget
{
    NoteDetectorWidget(NoteDetector *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("note_detector");
        applyTheme();

        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/note_detector/note_detector_panel.svg"),
            asset::plugin(pluginInstance, "res/note_detector/note_detector_panel-dark.svg")
        ));

        // Inputs
        addInput(createInputCentered<VoxglitchInputPort>(Vec(30.00000, 63.3706), module, NoteDetector::CV_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(Vec(30.00000, 111.4076), module, NoteDetector::CLOCK_INPUT));

        // Parameters
        addParam(createParamCentered<Trimpot>(Vec(30.00000, 208.74016), module, NoteDetector::NOTE_SELECTION_KNOB));
        addParam(createParamCentered<Trimpot>(Vec(30.00000, 263.87501), module, NoteDetector::OCTAVE_SELECTION_KNOB));

        // Outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(Vec(30.00000, 349.837158), module, NoteDetector::DETECTION_OUTPUT));

        // Add display
        NoteReadoutWidget *note_readout_widget = new NoteReadoutWidget("");
        note_readout_widget->box.pos = Vec(7.5, 143.0);
        note_readout_widget->box.size = Vec(45.0, 20.0);
        note_readout_widget->font_size = 12;
        if (module)
            note_readout_widget->display_string_ptr = &module->note_readout;
        addChild(note_readout_widget);
    }

    void appendContextMenu(Menu *menu) override
    {
        NoteDetector *module = dynamic_cast<NoteDetector *>(this->module);
        assert(module);

        // Add horizontal line
        menu->addChild(new MenuEntry);

        menu->addChild(createIndexSubmenuItem("Output Mode",
            {
                "Trigger",
                "Gate"
            },
            [=]() {
                return(module->output_mode);
            },
            [=](int output_mode_index) {
                module->setOutputMode(output_mode_index);
            }
        ));

        menu->addChild(createIndexSubmenuItem("Trigger Length",
            module->getTriggerLengthNames(),
            [=]() {
                return (module->trigger_length_index);
            },
            [=](int index) {
                module->setTriggerLengthIndex(index);
            }
        ));

        // Add submenu for Tolerance Level
        menu->addChild(createIndexSubmenuItem("Tolerance Level",
            module->tolerance_preset_labels,
            [=]() {
                return (module->tolerance_level_index);
            },
            [=](int index) {
                module->setToleranceIndex(index);
            }
        ));

        menu->addChild(createIndexSubmenuItem("Notation",
            {
                "Sharp",
                "Flat"
            },
            [=]() {
                return(module->use_flat_notation);
            },
            [=](int use_flat_notation_index) {
                module->setUseFlatNotation(use_flat_notation_index);
            }
        ));

    }
};
