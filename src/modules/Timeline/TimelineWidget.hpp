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
// PARTIAL adoption, deliberately: the LABELS and the wordmark are still drawn
// in code below, not baked into the SVG. nanosvg cannot render <text>, so the
// collection's panels carry their labels as outlined paths, which has to
// happen in a vector editor. When the art is redrawn with outlined text,
// delete TimelineChrome's label loop and the labels vector — nothing else
// depends on them.
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

// Labels sit above whatever the SVG anchor placed, so they follow the art.
static const float LABEL_DY = -23.f;

// The output plate's ink is drawn in the SVG; this is only where the light
// knocked-out labels go, and it must match the SVG's `output_plate` rect.
static const float PLATE_X = 452.f;

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

struct TimelineChrome : TransparentWidget
{
    // `onPlate` labels sit on the SVG's ink plate and are knocked out light.
    struct Lbl { float x, y; const char* text; bool onPlate; };
    std::vector<Lbl> labels;

    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        tlText(vg, MARGIN, HEADER_Y, "TIMELINE", 13.f, tcol(0x17171a),
               NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, 3.0f);
        tlText(vg, MARGIN + 108.f, HEADER_Y, "AUTOMATION", 6.5f, tcol(0x4a4a50),
               NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE, 1.2f);
        for (size_t i = 0; i < labels.size(); i++)
            tlText(vg, labels[i].x, labels[i].y, labels[i].text, 7.f,
                   labels[i].onPlate ? tcol(0xe9e7e1) : tcol(0x4a4a50),
                   NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE, 0.6f);
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
        struct Anchor { const char* id; int pid; const char* label; };
        static const Anchor INS[3] = {
            { "start_input", Timeline::START_INPUT, "START" },
            { "stop_input",  Timeline::STOP_INPUT,  "STOP" },
            { "reset_input", Timeline::RESET_INPUT, "RESET" },
        };
        for (int i = 0; i < 3; i++)
        {
            Vec p = panelHelper.findNamed(INS[i].id);
            addInput(createInputCentered<VoxglitchPolyPort>(p, module, INS[i].pid));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, INS[i].label, false });
        }

        // ── transport switches and the timing knobs ──
        // No LEDs beside PLAY and LOOP: the switches show their own state, and
        // the readout's moving digits already say "playing".
        {
            Vec p = panelHelper.findNamed("rewind_button");
            // VCVButton, not LEDButton: this collection uses the former
            // (TempestVS1's MidiConfigButton) and the latter nowhere.
            addParam(createParamCentered<VCVButton>(p, module, Timeline::REWIND_PARAM));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, "RWND", false });

            p = panelHelper.findNamed("play_switch");
            addParam(createParamCentered<squareToggle>(p, module, Timeline::PLAY_PARAM));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, "PLAY", false });

            p = panelHelper.findNamed("loop_switch");
            addParam(createParamCentered<squareToggle>(p, module, Timeline::LOOP_PARAM));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, "LOOP", false });

            p = panelHelper.findNamed("chase_switch");
            addParam(createParamCentered<squareToggle>(p, module, Timeline::CHASE_PARAM));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, "CHASE", false });

            p = panelHelper.findNamed("bpm_knob");
            addParam(createParamCentered<BpmKnob>(p, module, Timeline::BPM_PARAM));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, "BPM", false });

            p = panelHelper.findNamed("snap_knob");
            addParam(createParamCentered<RoundSmallBlackKnob>(p, module, Timeline::SNAP_PARAM));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, "SNAP", false });

            p = panelHelper.findNamed("div_knob");
            addParam(createParamCentered<RoundSmallBlackKnob>(p, module, Timeline::DIV_PARAM));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, "DIV", false });
        }

        // ── outputs, on the SVG's ink plate; LANES last ──
        static const Anchor OUTS[6] = {
            { "clk_output",   Timeline::CLK_OUTPUT,  "CLK" },
            { "rst_output",   Timeline::RST_OUTPUT,  "RST" },
            { "rwnd_output",  Timeline::RWND_OUTPUT, "RWND" },
            { "loop_output",  Timeline::LOOP_OUTPUT, "LOOP" },
            { "run_output",   Timeline::RUN_OUTPUT,  "RUN" },
            { "lanes_output", Timeline::POLY_OUTPUT, "LANES" },
        };
        for (int i = 0; i < 6; i++)
        {
            Vec p = panelHelper.findNamed(OUTS[i].id);
            if (OUTS[i].pid == Timeline::POLY_OUTPUT)
                addOutput(createOutputCentered<VoxglitchPolyPort>(p, module, OUTS[i].pid));
            else
                addOutput(createOutputCentered<VoxglitchOutputPort>(p, module, OUTS[i].pid));
            chrome->labels.push_back({ p.x, p.y + LABEL_DY, OUTS[i].label, true });
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
