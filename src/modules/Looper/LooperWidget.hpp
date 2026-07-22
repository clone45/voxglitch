// BPM knob with "Double" and "Half" context-menu items.  Useful when the
// auto-detector lands an octave off — one click sets the correct tempo.
struct BpmKnob : RoundBlackKnob
{
    void appendContextMenu(Menu *menu) override
    {
        ParamQuantity *pq = getParamQuantity();
        if (!pq) return;
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Double", "", [pq]() {
            pq->setValue(pq->getValue() * 2.0f);
        }));
        menu->addChild(createMenuItem("Half", "", [pq]() {
            pq->setValue(pq->getValue() * 0.5f);
        }));
    }
};

// Temporary text label drawn above a control.  Intended to be replaced by
// SVG typography in the final panel design.  The label adapts to the
// current panel theme so it stays readable in both light and dark modes.
struct LooperLabel : TransparentWidget
{
    std::string text;
    float font_size = 9.0f;
    std::shared_ptr<Font> font;

    void draw(const DrawArgs &args) override
    {
        if (!font)
            font = APP->window->loadFont(
                asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
        if (!font) return;

        NVGcolor color = rack::settings::preferDarkPanels
            ? nvgRGBA(200, 200, 200, 255)
            : nvgRGBA( 40,  40,  40, 255);

        nvgFontFaceId(args.vg, font->handle);
        nvgFontSize(args.vg, font_size);
        nvgFillColor(args.vg, color);
        nvgTextLetterSpacing(args.vg, 1.0f);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE);
        nvgText(args.vg, 0, 0, text.c_str(), NULL);
    }
};

// Helper: create a label whose baseline sits `y_offset` units above a
// control center.  `y_offset` is negative (move up).
static LooperLabel* makeLooperLabel(Vec control_center, const std::string& text,
                                     float y_offset = -14.0f)
{
    LooperLabel* lbl = new LooperLabel();
    lbl->text = text;
    lbl->box.pos = Vec(control_center.x, control_center.y + y_offset);
    lbl->box.size = Vec(0, 0);
    return lbl;
}

struct LooperWidget : VoxglitchSamplerModuleWidget
{
    struct OutputRangeValueItem : MenuItem
    {
        Looper *module;
        int range_index = 0;

        void onAction(const event::Action &e) override
        {
            module->voltage_range_index = range_index;
        }
    };

    struct OutputRangeItem : MenuItem
    {
        Looper *module;

        Menu *createChildMenu() override
        {
            Menu *menu = new Menu;

            for (unsigned int i = 0; i < 8; i++)
            {
                OutputRangeValueItem *output_range_value_item = createMenuItem<OutputRangeValueItem>(module->voltage_range_names[i], CHECKMARK(module->voltage_range_index == i));
                output_range_value_item->module = module;
                output_range_value_item->range_index = i;
                menu->addChild(output_range_value_item);
            }

            return menu;
        }
    };

    LooperWidget(Looper *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/looper/looper_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/looper/looper_panel-dark.svg")
        );

        // --- Waveform strip (only when module is present) ---
        // Place the widget exactly where the SVG's `waveform_backing`
        // placeholder rectangle sits, so the waveform fits the drawn
        // frame. Using the vertical strip variant since the panel was
        // narrowed and the placeholder is now tall-and-narrow; switch
        // back to LooperWaveformGrid if you ever widen the placeholder
        // to a horizontal layout again.
        if (module)
        {
            rack::math::Rect wf = panelHelper.findBounds("waveform_backing");
            LooperWaveformStrip *waveform = new LooperWaveformStrip(
                wf.pos.x, wf.pos.y,
                wf.size.x, wf.size.y,
                &module->track_model,
                module
            );
            // Tighter side padding since the strip is narrow; vertical
            // padding stays at 4 so the time axis has clear margins.
            waveform->setContainerPadding(4.0f, 2.0f);
            addChild(waveform);
        }

        // --- Inputs ---
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, Looper::RESET_INPUT));

        // --- Params ---
        addParam(createParamCentered<BpmKnob>(panelHelper.findNamed("bpm_knob"), module, Looper::BPM_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("nudge_knob"), module, Looper::NUDGE_KNOB));

        addParam(createParamCentered<RoundBlackKnob>(panelHelper.findNamed("volume_knob"), module, Looper::VOLUME_KNOB));

        // --- Trigger outputs ---
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("bar_output"),    module, Looper::BAR_TRIG_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("beat_output"),   module, Looper::BEAT_TRIG_OUTPUT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("subdiv_output"), module, Looper::SUBDIV_TRIG_OUTPUT));

        // --- Audio outputs ---
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("left_output"),  module, Looper::AUDIO_OUTPUT_LEFT));
        addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("right_output"), module, Looper::AUDIO_OUTPUT_RIGHT));

        // --- Temporary programmatic labels above each control.
        //     Replace with SVG typography in the final panel design.
        addChild(makeLooperLabel(panelHelper.findNamed("reset_input"),   "RESET"));
        addChild(makeLooperLabel(panelHelper.findNamed("bpm_knob"),      "BPM"));
        addChild(makeLooperLabel(panelHelper.findNamed("nudge_knob"),    "NUDGE"));
        addChild(makeLooperLabel(panelHelper.findNamed("volume_knob"),   "VOL"));
        addChild(makeLooperLabel(panelHelper.findNamed("bar_output"),    "BAR"));
        addChild(makeLooperLabel(panelHelper.findNamed("beat_output"),   "BEAT"));
        addChild(makeLooperLabel(panelHelper.findNamed("subdiv_output"), "16TH"));
        addChild(makeLooperLabel(panelHelper.findNamed("left_output"),   "L"));
        addChild(makeLooperLabel(panelHelper.findNamed("right_output"),  "R"));
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

        menu->addChild(new MenuEntry); // For spacing
        menu->addChild(createMenuLabel("Output"));

        // Add output range selector
        OutputRangeItem *output_range_item = createMenuItem<OutputRangeItem>("Output Range", RIGHT_ARROW);
        output_range_item->module = module;
        menu->addChild(output_range_item);
    }
};
