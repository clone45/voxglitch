struct PianoRollWidget : ModuleWidget
{
    // The editor's position and size are NOT constants here — they are read from
    // the panel SVG at construction, so the artwork stays the single source of
    // truth. Move or resize the `editor_area` rect in Inkscape and the widget
    // follows, with no constant to keep in sync.
    //
    // Everything the editor draws INSIDE that rect (row height, keys column,
    // scrollbar, zoom range, hot zones) lives in PianoRollGeometry.hpp.
    piano_roll::Layout layout;
    rack::Rect control_bar;
    PianoRollEditorWidget *editor = NULL;
    PianoRollControlBar *control_bar_widget = NULL;

    PianoRollWidget(PianoRoll *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/piano_roll/piano_roll_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/piano_roll/piano_roll_panel-dark.svg"));

        layout = piano_roll::Layout(panelHelper.findNamedRect("editor_area"));
        control_bar = panelHelper.findNamedRect("control_bar");

        if (!layout.isValid())
        {
            WARN("PianoRoll: panel is missing an 'editor_area' rect; the editor will not be shown");
        }

        // Screws. The editor column is x-disjoint from every screw box (screws
        // occupy x 15-30 and 750-765 only), which is what lets the editor span
        // nearly the full panel height.
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Clock and reset
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("clk_input"), module, PianoRoll::CLOCK_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("rst_input"), module, PianoRoll::RESET_INPUT));

        // Recording input pair — polyphonic, so a chord records as a chord
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("rec_voct_input"), module, PianoRoll::REC_VOCT_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("rec_gate_input"), module, PianoRoll::REC_GATE_INPUT));

        // Eight tracks, each a polyphonic V/OCT + GATE pair.
        //
        // Each pair sits on a plate tinted with that track's colour, so a cable
        // can be traced back to the notes that drive it. The plates are added
        // BEFORE the jacks so they draw underneath, and are sized from the jack
        // positions rather than a panel rect — the panel no longer carries one.
        for (int track = 0; track < PianoRoll::TRACKS; track++)
        {
            std::string number = std::to_string(track + 1);

            Vec voct_position = panelHelper.findNamed("voct_" + number + "_output");
            Vec gate_position = panelHelper.findNamed("gate_" + number + "_output");

            if (voct_position.y > 0.0f)
            {
                float lane_left = voct_position.x - 22.0f;
                float lane_right = gate_position.x + 22.0f;

                rack::Rect lane;
                lane.pos = Vec(lane_left, voct_position.y - 18.0f);
                lane.size = Vec(lane_right - lane_left, 36.0f);

                addChild(new PianoRollTrackLane(track, lane));
            }

            addOutput(createOutputCentered<VoxglitchOutputPort>(
                panelHelper.findNamed("voct_" + number + "_output"), module, PianoRoll::VOCT_OUTPUTS + track));

            addOutput(createOutputCentered<VoxglitchOutputPort>(
                panelHelper.findNamed("gate_" + number + "_output"), module, PianoRoll::GATE_OUTPUTS + track));
        }

        // The editor surface. Drawn even without a module so the browser preview
        // shows the grid rather than a blank plate.
        if (layout.isValid())
        {
            editor = new PianoRollEditorWidget(module, layout);
            addChild(editor);
        }

        // SNAP, the eight track squares and REC.
        if (control_bar.size.x > 0.0f && control_bar.size.y > 0.0f)
        {
            control_bar_widget = new PianoRollControlBar(module, control_bar);
            addChild(control_bar_widget);
        }
    }

    //
    // Give the editor first refusal on keys.
    //
    // ModuleWidget handles Delete/Backspace itself by removing the module, and it
    // does so BEFORE recursing to its children — so without this, pressing Delete
    // over the grid deletes the whole module instead of the selected notes.
    //
    // Recursing to children first and bailing out if one consumed the event is the
    // only way a child widget can shadow a ModuleWidget key command.
    //
    void onHoverKey(const HoverKeyEvent &e) override
    {
        OpaqueWidget::onHoverKey(e);
        if (e.isConsumed()) return;

        ModuleWidget::onHoverKey(e);
    }

    //
    // Module-wide settings. Note operations (quantize, shift, MIDI import/export,
    // select all, delete) live on the EDITOR's own right-click menu instead, since
    // they act on notes rather than on the module.
    //
    void appendContextMenu(Menu *menu) override
    {
        PianoRoll *piano_roll = dynamic_cast<PianoRoll *>(this->module);
        if (!piano_roll) return;

        menu->addChild(new MenuSeparator);

        // Snap governs create, move, resize and paste alignment. "Off" is 1 step
        // (a 16th) — note positions are integers, so that is the finest the model
        // has. The loop marker ignores this and is always bar-locked.
        static const int SNAP_VALUES[6] = {1, 2, 4, 8, 16, 1};
        menu->addChild(createBoolPtrMenuItem("Lock Editor", "", &piano_roll->locked));
        menu->addChild(createBoolPtrMenuItem("Record armed", "", &piano_roll->rec_armed));

        menu->addChild(createIndexSubmenuItem("Snap",
            {"1/16", "1/8", "1/4", "1/2", "Bar", "Off"},
            [piano_roll]() {
                for (int i = 0; i < 5; i++) if (SNAP_VALUES[i] == piano_roll->snap_steps) return i;
                return 0;
            },
            [piano_roll](int index) { piano_roll->snap_steps = SNAP_VALUES[index]; }));

        menu->addChild(createIndexSubmenuItem("Loop length",
            {"1 bar", "2 bars", "4 bars", "8 bars", "16 bars"},
            [piano_roll]() {
                int bars = piano_roll->loop_steps / piano_roll::STEPS_PER_BAR;
                if (bars <= 1) return 0;
                if (bars <= 2) return 1;
                if (bars <= 4) return 2;
                if (bars <= 8) return 3;
                return 4;
            },
            [piano_roll](int index) {
                static const int BARS[5] = {1, 2, 4, 8, 16};
                piano_roll->loop_steps = BARS[index] * piano_roll::STEPS_PER_BAR;
                piano_roll->notesChanged();
            }));

    }
};
