struct NoteDetectorWidget : VoxglitchModuleWidget
{
    NoteDetectorWidget(NoteDetector *module)
    {
        setModule(module);

        // Load and apply theme
        theme.load("note_detector");
        applyTheme();

        // Inputs
        addInput(createInputCentered<VoxglitchInputPort>(themePos("CV_INPUT"), module, NoteDetector::CV_INPUT));

        // Parameters
        addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("NOTE_SELECTION_KNOB"), module, NoteDetector::NOTE_SELECTION_KNOB));
        addParam(createParamCentered<VoxglitchMediumBlackKnob>(themePos("OCTAVE_SELECTION_KNOB"), module, NoteDetector::OCTAVE_SELECTION_KNOB));

        // Outputs
        addOutput(createOutputCentered<VoxglitchOutputPort>(themePos("DETECTION_OUTPUT"), module, NoteDetector::DETECTION_OUTPUT));

        // Add display
        NoteReadoutWidget *note_readout_widget = new NoteReadoutWidget(" A4");
        note_readout_widget->box.pos = themePos("NOTE_READOUT");
        if (module)
            note_readout_widget->display_string_ptr = &module->note_readout;
        addChild(note_readout_widget);
    }
};
