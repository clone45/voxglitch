struct NoteDetectorWidget : ModuleWidget
{
    NoteDetectorWidget(NoteDetector *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/note_detector/note_detector_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/note_detector/note_detector_panel-dark.svg")
        );

        // Inputs
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("cv_input"), module, NoteDetector::CV_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clock_input"), module, NoteDetector::CLOCK_INPUT));

        // Parameters
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("note_knob"), module, NoteDetector::NOTE_SELECTION_KNOB));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("octave_knob"), module, NoteDetector::OCTAVE_SELECTION_KNOB));

        // Outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("output"), module, NoteDetector::DETECTION_OUTPUT));

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
