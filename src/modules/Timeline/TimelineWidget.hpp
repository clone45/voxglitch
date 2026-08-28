#pragma once
// TimelineWidget.hpp — the panel. 48 HP.
//
// Ported into the voxglitch collection 2026-08-28 and adapted to the house
// panel convention:
//   - control POSITIONS come from named elements in the panel SVG, read
//     through PanelHelper::findNamed. Move a circle in Inkscape and the
//     control follows; the ids are the contract between art and code.
//   - jacks are Voxglitch ports, not stock Rack ones. LANES is a
//     VoxglitchPolyPort because it carries all 16 lanes; the five clock
//     outputs are polyphonic too, but they read as clocks, so they keep the
//     ordinary output socket.
//
// The control LABELS belong to the panel art, as outlined paths: nanosvg
// cannot render <text>, so the collection draws labels in a vector editor.
// The only typography left in code is the wordmark at the top.
//
// The editor, lane tabs and readout are code-drawn widgets, so their geometry
// stays here as constants, the same way the collection's other displays do.

#include "Timeline.hpp"
#include "TimelineEditor.hpp"

namespace timeline_ui
{

static const float PANEL_W = 720.f;    // 48 HP
static const float PANEL_H = 380.f;
static const float MARGIN  = 10.f;

static const float HEADER_Y     = 16.f;
static const float TABS_Y       = 31.f;
static const float TABS_H       = 13.f;
static const float EDITOR_Y     = 47.f;
static const float EDITOR_H     = 268.f;
static const float EDITOR_RIGHT = PANEL_W - MARGIN;

// The position readout: inside the editor, bottom right, above the scrollbar.
static const float READOUT_W = 150.f;
static const float READOUT_H = 14.f;
static const float READOUT_X = PANEL_W - MARGIN - 6.f - READOUT_W;
static const float READOUT_Y = EDITOR_Y + EDITOR_H - 12.f - READOUT_H - 3.f;

inline void tlText(NVGcontext* vg, float x, float y, const char* s, float px,
                   NVGcolor col, int align, float tracking = 0.5f)
{
    std::shared_ptr<Font> font =
        APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
    if (!font) return;
    nvgFontFaceId(vg, font->handle);
    nvgFontSize(vg, px);
    nvgTextLetterSpacing(vg, tracking);
    nvgFillColor(vg, col);
    nvgTextAlign(vg, align);
    nvgText(vg, x, y, s, NULL);
}

// smooth = false is REQUIRED for the half-BPM snapping to be visible. With
// Rack's default smoothing the knob writes a target to the engine, which then
// drives the value every sample and undoes HalfStepQuantity's rounding.
struct BpmKnob : RoundSmallBlackKnob
{
    BpmKnob() { smooth = false; }
};

// Only the wordmark is drawn in code. The control labels belong to the panel
// art, as outlined paths — nanosvg cannot render <text>, so they are drawn in
// a vector editor, not here.
struct TimelineChrome : TransparentWidget
{
    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        tlText(vg, MARGIN, HEADER_Y, "TIMELINE", 13.f, tcol(0x17171a),
               NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, 3.0f);
        tlText(vg, MARGIN + 108.f, HEADER_Y, "AUTOMATION", 6.5f, tcol(0x4a4a50),
               NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, 1.2f);
    }
};

} // namespace timeline_ui

struct TimelineWidget : ModuleWidget
{
    TimelineWidget(Timeline* module)
    {
        using namespace timeline_ui;
        setModule(module);

        PanelHelper panelHelper(this);
        panelHelper.loadPanel(
            asset::plugin(pluginInstance, "res/modules/timeline/timeline_panel.svg"),
            asset::plugin(pluginInstance, "res/modules/timeline/timeline_panel-dark.svg")
        );

        TimelineChrome* chrome = new TimelineChrome();
        chrome->box = Rect(Vec(0, 0), Vec(PANEL_W, PANEL_H));
        addChild(chrome);

        // ── lane tabs + the editor + the readout ──
        TimelineLaneTabs* tabs = new TimelineLaneTabs();
        tabs->module = module;
        tabs->box = Rect(Vec(MARGIN, TABS_Y), Vec(PANEL_W - 2.f * MARGIN, TABS_H));
        addChild(tabs);

        TimelineEditorWidget* editor = new TimelineEditorWidget();
        editor->module = module;
        editor->box = Rect(Vec(MARGIN, EDITOR_Y), Vec(PANEL_W - 2.f * MARGIN, EDITOR_H));
        addChild(editor);

        // Added AFTER the editor so it draws on top of it.
        TimelineReadout* readout = new TimelineReadout();
        readout->module = module;
        readout->box = Rect(Vec(READOUT_X, READOUT_Y), Vec(READOUT_W, READOUT_H));
        addChild(readout);

        // ── inputs: START / STOP / RESET, all polyphonic ──
        struct Anchor { const char* id; int pid; };
        static const Anchor INS[3] = {
            { "start_input", Timeline::START_INPUT },
            { "stop_input",  Timeline::STOP_INPUT  },
            { "reset_input", Timeline::RESET_INPUT },
        };
        for (int i = 0; i < 3; i++)
            addInput(createInputCentered<VoxglitchPolyPort>(
                panelHelper.findNamed(INS[i].id), module, INS[i].pid));

        // ── transport switches and the timing knobs ──
        // No LEDs beside PLAY and LOOP: the switches show their own state, and
        // the readout's moving digits already say "playing".
        // VCVButton, not LEDButton: this collection uses the former
        // (TempestVS1's MidiConfigButton) and the latter nowhere.
        addParam(createParamCentered<VCVButton>(
            panelHelper.findNamed("rewind_button"), module, Timeline::REWIND_PARAM));
        addParam(createParamCentered<squareToggle>(
            panelHelper.findNamed("play_switch"), module, Timeline::PLAY_PARAM));
        addParam(createParamCentered<squareToggle>(
            panelHelper.findNamed("loop_switch"), module, Timeline::LOOP_PARAM));
        addParam(createParamCentered<squareToggle>(
            panelHelper.findNamed("chase_switch"), module, Timeline::CHASE_PARAM));
        addParam(createParamCentered<BpmKnob>(
            panelHelper.findNamed("bpm_knob"), module, Timeline::BPM_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(
            panelHelper.findNamed("snap_knob"), module, Timeline::SNAP_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(
            panelHelper.findNamed("div_knob"), module, Timeline::DIV_PARAM));

        // ── outputs, on the SVG's ink plate; LANES last ──
        static const Anchor OUTS[6] = {
            { "clk_output",   Timeline::CLK_OUTPUT  },
            { "rst_output",   Timeline::RST_OUTPUT  },
            { "rwnd_output",  Timeline::RWND_OUTPUT },
            { "loop_output",  Timeline::LOOP_OUTPUT },
            { "run_output",   Timeline::RUN_OUTPUT  },
            { "lanes_output", Timeline::POLY_OUTPUT },
        };
        for (int i = 0; i < 6; i++)
        {
            Vec p = panelHelper.findNamed(OUTS[i].id);
            if (OUTS[i].pid == Timeline::POLY_OUTPUT)
                addOutput(createOutputCentered<VoxglitchPolyPort>(p, module, OUTS[i].pid));
            else
                addOutput(createOutputCentered<VoxglitchOutputPort>(p, module, OUTS[i].pid));
        }
    }

    void appendContextMenu(Menu* menu) override
    {
        Timeline* m = dynamic_cast<Timeline*>(this->module);
        if (!m) return;

        menu->addChild(new MenuSeparator);
        menu->addChild(createBoolMenuItem("Lock Editor", "",
            [=]() { return m->locked; },
            [=](bool val) { m->locked = val; }
        ));
        // Chase is a panel switch, not a menu item — one control per thing.
    }
};
