// Placeholder panel lettering drawn in nvg (STEP / RESET / MEM / LEN, and the
// colored gate chips tying each output jack to its trigger-group hue) until
// final SVG artwork replaces it.
struct GlitchSequencerPanelLabels : TransparentWidget
{
  Vec step_pos, reset_pos, mem_pos, len_pos;
  Vec gate_pos[NUMBER_OF_TRIGGER_GROUPS];

  void drawLabel(NVGcontext *vg, float x, float y, const char *text, float size, NVGcolor color, int align = NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE)
  {
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
    if(!font) return;

    nvgFontSize(vg, size);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0);
    nvgTextAlign(vg, align);
    nvgFillColor(vg, color);
    nvgText(vg, x, y, text, NULL);
  }

  void draw(const DrawArgs &args) override
  {
    const auto vg = args.vg;
    nvgSave(vg);

    NVGcolor label_color = settings::preferDarkPanels ? nvgRGB(0xc0, 0xc0, 0xc0) : nvgRGB(0x3c, 0x3f, 0x45);

    drawLabel(vg, step_pos.x, step_pos.y - 21.0, "STEP", 9.0, label_color);
    drawLabel(vg, reset_pos.x, reset_pos.y - 21.0, "RESET", 9.0, label_color);
    drawLabel(vg, mem_pos.x, mem_pos.y - 21.0, "MEM", 9.0, label_color);
    drawLabel(vg, len_pos.x, len_pos.y - 21.0, "LEN", 9.0, label_color);

    // Colored chips above the gate outputs, matching the screen's tab hues
    for(unsigned int i = 0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
    {
      nvgBeginPath(vg);
      nvgRoundedRect(vg, gate_pos[i].x - 8.0, gate_pos[i].y - 27.0, 16.0, 11.0, 2.5);
      nvgFillColor(vg, gs_rgb(GS_GROUP_COLORS[i]));
      nvgFill(vg);

      drawLabel(vg, gate_pos[i].x, gate_pos[i].y - 21.0, std::to_string(i + 1).c_str(), 9.0, nvgRGB(0x10, 0x15, 0x1a));
    }

    nvgRestore(vg);
  }
};

struct GlitchSequencerWidget : ModuleWidget
{
    GlitchSequencerWidget(GlitchSequencer *module)
    {
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/glitch_sequencer/glitch_sequencer_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/glitch_sequencer/glitch_sequencer_panel-dark.svg")
        );

        // Screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("step_input"), module, GlitchSequencer::STEP_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("reset_input"), module, GlitchSequencer::RESET_INPUT));
        addInput(createInputCentered<VoxglitchInputPort>(panelHelper.findNamed("mem_input"), module, GlitchSequencer::MEM_INPUT));
        addParam(createParamCentered<Trimpot>(panelHelper.findNamed("length_knob"), module, GlitchSequencer::LENGTH_KNOB));

        for(unsigned int i = 0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
        {
            addOutput(createOutputCentered<VoxglitchOutputPort>(panelHelper.findNamed("gate_output_" + std::to_string(i + 1)), module, GlitchSequencer::GATE_OUTPUTS + i));
        }

        // The screen: position and size come from the panel artwork, so the
        // SVG stays the single source of truth for the layout.
        CellularAutomatonDisplay *ca_display = new CellularAutomatonDisplay();
        rack::Rect screen_rect = panelHelper.findNamedRect("screen");
        ca_display->box.pos = screen_rect.pos;
        ca_display->box.size = screen_rect.size;
        ca_display->module = module;
        addChild(ca_display);

        // Placeholder lettering (see GlitchSequencerPanelLabels)
        GlitchSequencerPanelLabels *labels = new GlitchSequencerPanelLabels();
        labels->box.pos = Vec(0, 0);
        labels->box.size = box.size;
        labels->step_pos = panelHelper.findNamed("step_input");
        labels->reset_pos = panelHelper.findNamed("reset_input");
        labels->mem_pos = panelHelper.findNamed("mem_input");
        labels->len_pos = panelHelper.findNamed("length_knob");
        for(unsigned int i = 0; i < NUMBER_OF_TRIGGER_GROUPS; i++)
        {
            labels->gate_pos[i] = panelHelper.findNamed("gate_output_" + std::to_string(i + 1));
        }
        addChild(labels);
    }

    void appendContextMenu(Menu *menu) override
    {
        GlitchSequencer *module = dynamic_cast<GlitchSequencer *>(this->module);
        assert(module);
    }
};
