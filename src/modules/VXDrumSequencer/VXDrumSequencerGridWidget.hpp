#pragma once
//
// VX Drum Sequencer — the pad grid: 16 steps x 7 lanes (BD SD CP PERC CH OH plus
// the ACCENT lane), a lamp row on top that shows the playhead and edits the
// memory's length, and a lane-label column on the left that mutes.
//
// A port of vxsynth's PanelVxDrumsGrid (panel-vxdrumsgrid.js) in the HexDrums
// livery: near-black body, yellow silk, amber pads, the accent lane in red.
// Line numbers in comments below cite that file unless another is named.
//
// Conventions (brief-editor-widget.md):
//   * OpaqueWidget; box from the panel's `grid_area` rect, set in the
//     constructor (PianoRollEditorWidget.hpp:108-112 — an unsized widget passes
//     the hit test for every position in its parent).
//   * dark screen regardless of the panel theme (PianoRollEditorWidget.hpp:9-14).
//   * module may be NULL (module browser): every read is null-safe and the
//     pads show the SEED pattern (CellularAutomatonDisplay.hpp:73-84).
//   * a left press is consumed -> drag events follow; a right press is consumed
//     ONLY over a lit pad, where the pad menu (ratchet, chance) opens; anywhere
//     else it falls through so the module's own context menu still opens
//     (PianoRollEditorWidget.hpp:1622-1642).
//   * the drag position is accumulated from mouseDelta / getAbsoluteZoom()
//     (PianoRollEditorWidget.hpp:1650-1658).
//   * every edit goes bankCopy() -> mutate -> publishBank() and pushes ONE
//     VXDrumSequencerBankAction per gesture; a no-op gesture pushes nothing
//     (PianoRollEditorWidget.hpp:1010-1066; DESIGN §4.7).
//   * CHANCE MODE (module->chance_mode, 2026-09-07): lit pads draw as bars
//     whose fill height is their chance, and the left-button gesture changes:
//     the press only ARMS (DRAG_PENDING); a vertical move past CHANCE_DRAG_PX
//     on a lit pad becomes a chance drag (DRAG_CHANCE), a release without one
//     toggles the pad as a click would. The drag is ABSOLUTE, the Digital
//     Sequencer's editBar idiom (VoltageSequencerDisplay.hpp): the bar follows
//     the pointer's height within the pad, top = 100 %, bottom = 0 %. There is
//     no paint-across in chance mode. Steps mode is unchanged.
//   * lit pads, lamps, the playhead and the lane flashes are drawn in
//     drawLayer(args, 1); the wells and labels in draw(). The module browser
//     gets no layer-1 pass, so a NULL module draws the lit pass from draw()
//     (CellularAutomatonDisplay.hpp:263-284).
//
// Identity rule: lanes, steps and slots are fixed-size indices of a fixed grid
// (structure, not identity). LANE_NAMES is display only.
//

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

namespace vx_drum_sequencer_ui
{
    // ── Geometry (DESIGN §4.6, scaled from panel-vxdrumsgrid.js:35-40) ───────
    // Namespace-scope `static const` rather than class-scope constexpr members:
    // a constexpr member bound to a const reference (std::max, std::min) is
    // ODR-used and needs an out-of-class definition in C++11.
    static const float LANEW = 30.f;        // lane-label / mute column
    static const float LAMPH = 14.f;        // lamp row (length + playhead)
    static const float PAD = 4.f;           // right / bottom padding
    static const float GROUP_GAP = 4.f;     // between 4-step groups
    static const float CELL_GAP = 2.f;      // between pads
    static const float ACCENT_GAP = 4.f;    // extra space setting the accent lane apart
    static const int GROUP = 4;             // steps per shaded group

    // ── HexDrums livery (panel-vxdrumsgrid.js:44-56) ─────────────────────────
    static const unsigned int BODY = 0x141519;
    static const unsigned int WELL_A = 0x24262c;      // OFF pad, even 4-group
    static const unsigned int WELL_B = 0x1b1d22;      // OFF pad, odd 4-group (the cherry-key alternation)
    static const unsigned int WELL_EDGE = 0x0c0d10;
    static const unsigned int PAD_ON = 0xe4c93f;      // lit voice pad (HexDrums yellow)
    static const unsigned int PAD_ON_ACC = 0xd85040;  // lit accent pad (red)
    static const unsigned int LANE_BG = 0x22242a;
    static const unsigned int LANE_TEXT = 0xd8c94a;   // yellow silk
    static const unsigned int LANE_MUTED = 0x6a6046;
    static const unsigned int LANE_MUTED_BG = 0x7a3a30;
    static const unsigned int LANE_EDGE = 0x3a3c42;
    static const unsigned int LEN_TEXT = 0x8a7c3e;
    static const unsigned int LAMP_OFF = 0x3a3c42;
    static const unsigned int LAMP_IN = 0x8a7c3e;
    static const unsigned int LAMP_PLAY = 0xff5a4a;
    static const unsigned int PLAYCOL = 0xffffff;

    // 0xRRGGBB + alpha -> NVGcolor (TimelineEditor.hpp:53-56 `tcol` idiom).
    inline NVGcolor vxdColor(unsigned int rgb, float alpha = 1.f)
    {
        return nvgRGBA((unsigned char)((rgb >> 16) & 0xffu),
                       (unsigned char)((rgb >> 8) & 0xffu),
                       (unsigned char)(rgb & 0xffu),
                       (unsigned char)(rack::math::clamp(alpha, 0.f, 1.f) * 255.f + 0.5f));
    }

    // Loaded every draw, never cached across frames (brief-rack-sdk-api §11;
    // PianoRollEditorWidget.hpp:583-605). Null-check the result.
    inline std::shared_ptr<Font> displayFont()
    {
        return APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
    }

    // The module-browser preview: the SEED beat (vxdrums.js:80-87), read-only.
    inline const vx_drum_sequencer::Memory& previewMemory()
    {
        static vx_drum_sequencer::Bank bank;
        static bool seeded = false;
        if (!seeded)
        {
            vx_drum_sequencer::seedBank(bank);
            seeded = true;
        }
        return bank.memories[0];
    }

    // ── Undo (DESIGN §4.7) ───────────────────────────────────────────────────
    //
    // The edit has ALREADY been applied to the module (publishBank / mute);
    // `before` was captured before the gesture's first mutation. Pushes nothing
    // when nothing changed (PianoRollEditorWidget.hpp:1042-1043 "no-op: push
    // nothing"). Shared by the grid, the memory buttons and the module widget.
    //
    inline void pushBankEdit(VXDrumSequencer* module, const std::string& name,
                             const vx_drum_sequencer::Bank& before, uint8_t mute_before)
    {
        if (!module) return;

        VXDrumSequencerBankAction* action = new VXDrumSequencerBankAction;
        action->name = name;
        action->moduleId = module->id;
        action->before = before;
        action->mute_before = mute_before;
        action->after = module->liveBank();
        action->mute_after = module->mute;

        if (action->isNoop())
        {
            delete action;
            return;
        }

        APP->history->push(action);
    }

    // A single-shot edit: apply `after` and push it as one undo step.
    inline void commitBankEdit(VXDrumSequencer* module, const std::string& name,
                               const vx_drum_sequencer::Bank& after, uint8_t mute_after)
    {
        if (!module) return;

        vx_drum_sequencer::Bank before = module->bankCopy();
        uint8_t mute_before = module->mute;

        module->publishBank(after);
        module->mute = mute_after;

        pushBankEdit(module, name, before, mute_before);
    }

    // Right-click ratchet: the "extra hits" of one voice pad (source menuAt
    // :264-285). Captured by the menu lambdas with the module pointer and plain
    // indices only — never a widget pointer, which an undo of a module delete
    // could invalidate (brief-rack-sdk-api §9).
    inline void setRatchet(VXDrumSequencer* module, int slot, int lane, int step, int extra)
    {
        if (!module) return;
        if (slot < 0 || slot >= vx_drum_sequencer::SLOTS) return;
        if (lane < 0 || lane >= vx_drum_sequencer::VOICES) return;
        if (step < 0 || step >= vx_drum_sequencer::STEPS) return;

        vx_drum_sequencer::Bank b = module->bankCopy();
        b.memories[slot].at(lane, step).ratchet = (uint8_t)rack::math::clamp(extra, 0, vx_drum_sequencer::RATCHET_MAX);

        commitBankEdit(module, "ratchet", b, module->mute);
    }

    // Right-click chance: the percent probability of one pad (any lane, the
    // accent lane included). Same capture rule as setRatchet.
    inline void setChance(VXDrumSequencer* module, int slot, int lane, int step, int percent)
    {
        if (!module) return;
        if (slot < 0 || slot >= vx_drum_sequencer::SLOTS) return;
        if (lane < 0 || lane >= vx_drum_sequencer::LANES) return;
        if (step < 0 || step >= vx_drum_sequencer::STEPS) return;

        vx_drum_sequencer::Bank b = module->bankCopy();
        b.memories[slot].at(lane, step).chance = (uint8_t)rack::math::clamp(percent, 0, vx_drum_sequencer::CHANCE_MAX);

        commitBankEdit(module, "chance", b, module->mute);
    }

    // The right-click menu's chance presets.
    static const int CHANCE_PRESETS[6] = { 100, 75, 50, 25, 10, 0 };
}

struct VXDrumSequencerGridWidget : OpaqueWidget
{
    VXDrumSequencer* module = NULL;

    // ── Lane flashes (source: _flash, applyReport :242-257) ──────────────────
    // Keyed on fired_serial, not on a step change: the source keys on the step
    // and can flash the previous step's lanes on a swung offbeat (brief-vxsynth-
    // semantics §8); the serial is bumped by fire() itself, so it cannot.
    float flash[vx_drum_sequencer::LANES] = {};
    uint32_t last_serial = 0;

    // ── Paint / chance gesture ───────────────────────────────────────────────
    enum DragMode { DRAG_NONE, DRAG_PAINT, DRAG_PENDING, DRAG_CHANCE };

    DragMode drag_mode = DRAG_NONE;
    Vec drag_position;              // accumulated pointer position, widget-local
    bool paint_value = false;       // decided on the press, applied to every pad crossed
    int paint_slot = 0;             // the edit target, latched at press (source :326)
    int last_lane = -1;
    int last_step = -1;

    // Chance mode's gesture: the pad pressed and where the press landed, so
    // the threshold is measured from it (widget pixels).
    static constexpr float CHANCE_DRAG_PX = 3.f;
    int chance_lane = -1;
    int chance_step = -1;
    float press_y = 0.f;

    // The gesture's undo bracket (PianoRollEditorWidget.hpp:1010-1066).
    bool edit_open = false;
    vx_drum_sequencer::Bank edit_before;
    uint8_t edit_mute_before = 0;

    VXDrumSequencerGridWidget(VXDrumSequencer* module, rack::Rect bounds)
    {
        this->module = module;
        box.pos = bounds.pos;
        box.size = bounds.size;

        // A widget rebuilt around a live module (undo of a delete, a reloaded
        // patch) must not flash every lane of the last fire on its first frame.
        if (module) last_serial = module->report.fired_serial;
    }

    // ── Geometry (source :95-113) ────────────────────────────────────────────
    float gridX() const { return vx_drum_sequencer_ui::LANEW; }
    float gridY() const { return vx_drum_sequencer_ui::LAMPH; }
    float gridW() const { return box.size.x - vx_drum_sequencer_ui::LANEW - vx_drum_sequencer_ui::PAD; }
    float gridH() const { return box.size.y - vx_drum_sequencer_ui::LAMPH - vx_drum_sequencer_ui::PAD; }

    float cellW() const
    {
        return (gridW() - (float)(vx_drum_sequencer::STEPS / vx_drum_sequencer_ui::GROUP - 1) * vx_drum_sequencer_ui::GROUP_GAP)
               / (float)vx_drum_sequencer::STEPS;
    }

    // Lane height: the accent gap comes out of the shared pool.
    float laneH() const
    {
        return (gridH() - vx_drum_sequencer_ui::ACCENT_GAP) / (float)vx_drum_sequencer::LANES;
    }

    float stepX(int step) const
    {
        return gridX() + (float)step * cellW() + (float)(step / vx_drum_sequencer_ui::GROUP) * vx_drum_sequencer_ui::GROUP_GAP;
    }

    float laneY(int lane) const
    {
        return gridY() + (float)lane * laneH() + (lane == vx_drum_sequencer::ACCENT_LANE ? vx_drum_sequencer_ui::ACCENT_GAP : 0.f);
    }

    // ── Hit testing (source cellAt / laneAt / lampAt :115-143) ───────────────
    // The gaps between groups and above the accent lane hit nothing.
    bool cellAt(Vec p, int& lane, int& step) const
    {
        if (p.x < gridX() || p.y < gridY()) return false;

        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            const float y = laneY(l);
            if (p.y < y || p.y >= y + laneH()) continue;

            for (int c = 0; c < vx_drum_sequencer::STEPS; c++)
            {
                const float x = stepX(c);
                if (p.x >= x && p.x < x + cellW())
                {
                    lane = l;
                    step = c;
                    return true;
                }
            }
        }
        return false;
    }

    int laneAt(Vec p) const
    {
        if (p.x >= vx_drum_sequencer_ui::LANEW || p.y < gridY()) return -1;

        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            const float y = laneY(l);
            if (p.y >= y && p.y < y + laneH()) return l;
        }
        return -1;
    }

    int lampAt(Vec p) const
    {
        if (p.y >= vx_drum_sequencer_ui::LAMPH || p.x < gridX()) return -1;

        for (int c = 0; c < vx_drum_sequencer::STEPS; c++)
        {
            const float x = stepX(c);
            if (p.x >= x && p.x < x + cellW()) return c;
        }
        return -1;
    }

    // ── Null-safe reads (module browser) ─────────────────────────────────────

    // The memory shown AND edited: the EFFECTIVE slot (MEM CV override else the
    // buttons, source :100-103), latched for the length of a paint drag so a CV
    // change cannot redirect the edit mid-gesture (source :326).
    int slot() const
    {
        if (drag_mode == DRAG_PAINT) return paint_slot;
        return module ? rack::math::clamp(module->report.slot, 0, vx_drum_sequencer::SLOTS - 1) : 0;
    }

    const vx_drum_sequencer::Memory& memory() const
    {
        return module ? module->liveBank().memories[slot()] : vx_drum_sequencer_ui::previewMemory();
    }

    int length() const { return rack::math::clamp(memory().length, 1, vx_drum_sequencer::STEPS); }
    int position() const { return module ? module->report.position : 0; }   // the preview parks on step 1 (CellularAutomatonDisplay.hpp:79)
    uint8_t muteMask() const { return module ? module->mute : 0; }

    bool padOn(int lane, int step) const { return memory().at(lane, step).on; }
    bool laneMuted(int lane) const { return ((muteMask() >> lane) & 1u) != 0u; }

    // 1..4 total hits for a voice pad; the accent lane has no ratchets (:182-183).
    int hitsAt(int lane, int step) const
    {
        if (lane >= vx_drum_sequencer::VOICES) return 1;
        return memory().at(lane, step).hits();
    }

    int chanceAt(int lane, int step) const { return memory().at(lane, step).chance; }
    bool chanceMode() const { return module && module->chance_mode; }

    // ── Per-frame sync (the UI thread; brief-rack-sdk-api §7 "put sync logic in step") ──
    void step() override
    {
        if (module)
        {
            const uint32_t serial = module->report.fired_serial;
            if (serial != last_serial)
            {
                last_serial = serial;
                const uint32_t fired = module->report.fired_mask;   // one word behind the serial at worst: one frame misdrawn
                for (int l = 0; l < vx_drum_sequencer::LANES; l++)
                {
                    if ((fired >> l) & 1u) flash[l] = 1.f;
                }
            }
        }

        // Decay x0.80 per frame, clear below 0.02 (source :254).
        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            if (flash[l] > 0.f)
            {
                flash[l] *= 0.80f;
                if (flash[l] < 0.02f) flash[l] = 0.f;
            }
        }

        OpaqueWidget::step();
    }

    // ── Drawing ──────────────────────────────────────────────────────────────
    void draw(const DrawArgs& args) override
    {
        const auto vg = args.vg;
        nvgSave(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y, 5.f);
        nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::BODY));
        nvgFill(vg);

        drawWells(vg);
        drawLabels(vg);

        // The browser preview gets no layer-1 pass (CellularAutomatonDisplay.hpp:263-268).
        if (!module) drawLit(vg);

        nvgRestore(vg);
        OpaqueWidget::draw(args);
    }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        if (layer == 1 && module)
        {
            nvgSave(args.vg);
            drawLit(args.vg);
            nvgRestore(args.vg);
        }

        OpaqueWidget::drawLayer(args, layer);
    }

    // The unlit pass: every pad's well. A lit pad's dark rim is drawn here too,
    // so the lit fill in layer 1 sits on it (source :180-181, :190-194).
    void drawWells(NVGcontext* vg)
    {
        const float cw = cellW();
        const float lh = laneH();
        const int len = length();

        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            const float y = laneY(l) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
            const float h = lh - vx_drum_sequencer_ui::CELL_GAP;

            for (int c = 0; c < vx_drum_sequencer::STEPS; c++)
            {
                const float x = stepX(c) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
                const float w = cw - vx_drum_sequencer_ui::CELL_GAP;
                const bool in_len = c < len;

                if (padOn(l, c))
                {
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x, y, w, h, 3.f);
                    nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::WELL_EDGE));
                    nvgFill(vg);
                }
                else
                {
                    const unsigned int well = ((c / vx_drum_sequencer_ui::GROUP) % 2 == 0)
                        ? vx_drum_sequencer_ui::WELL_A : vx_drum_sequencer_ui::WELL_B;

                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x, y, w, h, 3.f);
                    nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(well, in_len ? 1.f : 0.45f));
                    nvgFill(vg);

                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x, y, w, h, 3.f);
                    nvgStrokeWidth(vg, 1.f);
                    nvgStrokeColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::WELL_EDGE, 0.8f));
                    nvgStroke(vg);
                }
            }
        }
    }

    // LEN + the length number above the label column, then the lane labels
    // (source _redrawLane :219-240). The flash wash is lit, so it is in drawLit().
    void drawLabels(NVGcontext* vg)
    {
        std::shared_ptr<Font> font = vx_drum_sequencer_ui::displayFont();

        const float lh = laneH();
        const float label_x = 1.5f;
        const float label_w = vx_drum_sequencer_ui::LANEW - 5.f;

        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            const float y = laneY(l) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
            const float h = lh - vx_drum_sequencer_ui::CELL_GAP;
            const bool muted = laneMuted(l);

            nvgBeginPath(vg);
            nvgRoundedRect(vg, label_x, y, label_w, h, 3.f);
            nvgFillColor(vg, muted ? vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LANE_MUTED_BG, 0.6f)
                                   : vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LANE_BG));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRoundedRect(vg, label_x, y, label_w, h, 3.f);
            nvgStrokeWidth(vg, 1.f);
            nvgStrokeColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LANE_EDGE, 0.9f));
            nvgStroke(vg);
        }

        if (!font) return;

        // Set the face explicitly in the first text call of a draw
        // (PianoRollControlBar.hpp:215-217).
        nvgFontFaceId(vg, font->handle);
        nvgTextLetterSpacing(vg, 0.f);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        const float len_x = vx_drum_sequencer_ui::LANEW * 0.5f;

        nvgFontSize(vg, 6.f);
        nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LEN_TEXT));
        nvgText(vg, len_x, 4.f, "LEN", NULL);

        nvgFontSize(vg, 8.f);
        nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LANE_TEXT));
        nvgText(vg, len_x, 10.f, std::to_string(length()).c_str(), NULL);

        nvgFontSize(vg, 8.f);
        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            const float y = laneY(l) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
            const float h = lh - vx_drum_sequencer_ui::CELL_GAP;
            const bool muted = laneMuted(l);
            const bool accent = (l == vx_drum_sequencer::ACCENT_LANE);

            const unsigned int ink = muted ? vx_drum_sequencer_ui::LANE_MUTED
                                   : (accent ? vx_drum_sequencer_ui::PAD_ON_ACC : vx_drum_sequencer_ui::LANE_TEXT);

            nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(ink));
            nvgText(vg, (vx_drum_sequencer_ui::LANEW - 3.f) * 0.5f, y + h * 0.5f + 0.5f,
                    vx_drum_sequencer::LANE_NAMES[l], NULL);
        }
    }

    // The lit pass: lamps, lit pads (with their ratchet slivers and the playing
    // stroke), the playhead column outline and the lane flashes.
    void drawLit(NVGcontext* vg)
    {
        const float cw = cellW();
        const float lh = laneH();
        const int len = length();
        const int pos = position();

        // Lamp row (source _redrawLamps :205-217): red at the playhead, dim
        // yellow inside the length, off beyond it; the last in-length lamp is
        // outlined so the length reads at a glance.
        const float lamp_y = 3.f;
        const float lamp_h = vx_drum_sequencer_ui::LAMPH - 7.f;
        for (int c = 0; c < vx_drum_sequencer::STEPS; c++)
        {
            const float x = stepX(c) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
            const float w = cw - vx_drum_sequencer_ui::CELL_GAP;
            const bool in_len = c < len;
            const bool playing = (c == pos);

            const unsigned int col = playing ? vx_drum_sequencer_ui::LAMP_PLAY
                                   : (in_len ? vx_drum_sequencer_ui::LAMP_IN : vx_drum_sequencer_ui::LAMP_OFF);

            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, lamp_y, w, lamp_h, 2.f);
            nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(col, playing ? 1.f : (in_len ? 0.9f : 0.4f)));
            nvgFill(vg);

            if (c == len - 1)
            {
                nvgBeginPath(vg);
                nvgRoundedRect(vg, x, lamp_y, w, lamp_h, 2.f);
                nvgStrokeWidth(vg, 1.f);
                nvgStrokeColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LANE_TEXT, 0.7f));
                nvgStroke(vg);
            }
        }

        // With TWEAK up, the grid shows what is PLAYING: the tweaked copy,
        // computed here from the same inputs the head uses (deterministic, so
        // it matches). A pad the tweak ADDED gets a ring in the lane colour; a
        // pad it REMOVED stays as a faint ghost of the drawn pattern.
        const vx_drum_sequencer::Memory& base = memory();
        const int tweak_level = module ? module->tweak_level : 0;
        const uint8_t tweak_lanes = module ? module->tweak_lanes : 0;
        const bool tweaking = tweak_level > 0 && tweak_lanes != 0;
        const vx_drum_sequencer::Memory shown = tweaking
            ? vx_drum_sequencer::tweakMemory(base, tweak_lanes, tweak_level) : base;

        // Lit pads (source :180-189).
        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            const float y = laneY(l) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
            const float h = lh - vx_drum_sequencer_ui::CELL_GAP;
            const bool muted = laneMuted(l);
            const unsigned int col = (l == vx_drum_sequencer::ACCENT_LANE)
                ? vx_drum_sequencer_ui::PAD_ON_ACC : vx_drum_sequencer_ui::PAD_ON;

            for (int c = 0; c < vx_drum_sequencer::STEPS; c++)
            {
                const vx_drum_sequencer::Step& st = shown.at(l, c);
                const bool base_on = base.at(l, c).on;
                const bool added = st.on && !base_on;
                const bool removed = !st.on && base_on;
                if (!st.on && !removed) continue;

                const float x = stepX(c) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
                const float w = cw - vx_drum_sequencer_ui::CELL_GAP;
                const bool in_len = c < len;
                const bool playing = (c == pos);

                if (removed)
                {
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x + 1.f, y + 1.f, w - 2.f, h - 2.f, 2.5f);
                    nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(col, 0.16f));
                    nvgFill(vg);
                    continue;
                }

                const int chance = st.chance;
                const float state_alpha = muted ? 0.28f : (in_len ? (playing ? 1.f : 0.9f) : 0.35f);

                if (chanceMode())
                {
                    // Chance mode: the pad is a bar. A faint full-height
                    // ghost says "this pad is on"; the solid fill rises from
                    // the bottom to the pad's chance, never below a 2 px
                    // sliver so a 0 % pad is still visibly lit.
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x + 1.f, y + 1.f, w - 2.f, h - 2.f, 2.5f);
                    nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(col, state_alpha * 0.22f));
                    nvgFill(vg);

                    const float inner_h = h - 2.f;
                    const float bar_h = std::max(2.f, inner_h * (float)chance / (float)vx_drum_sequencer::CHANCE_MAX);
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x + 1.f, y + 1.f + (inner_h - bar_h), w - 2.f, bar_h, 2.f);
                    nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(col, state_alpha));
                    nvgFill(vg);
                }
                else
                {
                    // Steps mode: a pad below 100 % chance is dimmed toward
                    // its probability, so a hit that only sometimes plays
                    // reads that way at a glance (never below 35 % of the lit
                    // alpha: the pad must still read as ON).
                    const float certainty = (chance >= vx_drum_sequencer::CHANCE_MAX)
                        ? 1.f : 0.35f + 0.65f * ((float)chance / (float)vx_drum_sequencer::CHANCE_MAX);
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x + 1.f, y + 1.f, w - 2.f, h - 2.f, 2.5f);
                    nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(col, state_alpha * certainty));
                    nvgFill(vg);
                }

                if (added)
                {
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1.f, h - 1.f, 3.f);
                    nvgStrokeWidth(vg, 1.5f);
                    nvgStrokeColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LANE_TEXT, 0.9f));
                    nvgStroke(vg);
                }

                // Ratchet: the pad splits into n slivers — the visual is the meaning.
                const int hits = (l < vx_drum_sequencer::VOICES) ? st.hits() : 1;
                for (int d = 1; d < hits; d++)
                {
                    const float dx = x + (w * (float)d) / (float)hits;
                    nvgBeginPath(vg);
                    nvgMoveTo(vg, dx, y + 1.f);
                    nvgLineTo(vg, dx, y + h - 1.f);
                    nvgStrokeWidth(vg, 1.5f);
                    nvgStrokeColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::WELL_EDGE, 0.95f));
                    nvgStroke(vg);
                }

                if (!muted && playing)
                {
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg, x, y, w, h, 3.f);
                    nvgStrokeWidth(vg, 1.5f);
                    nvgStrokeColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::PLAYCOL, 0.85f));
                    nvgStroke(vg);
                }

                // The pad being chance-dragged shows its percent.
                if (drag_mode == DRAG_CHANCE && l == chance_lane && c == chance_step)
                {
                    std::shared_ptr<Font> font = vx_drum_sequencer_ui::displayFont();
                    if (font)
                    {
                        nvgFontFaceId(vg, font->handle);
                        nvgFontSize(vg, 8.f);
                        nvgTextLetterSpacing(vg, 0.f);
                        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                        nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::LANE_TEXT, 1.f));
                        nvgText(vg, x + w * 0.5f, y + h * 0.5f, std::to_string(chance).c_str(), NULL);
                    }
                }
            }
        }

        // Playhead: a white outline around the whole column (source :198-202).
        if (pos >= 0 && pos < vx_drum_sequencer::STEPS)
        {
            const float x = stepX(pos);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x - 1.f, gridY() - 1.f, cw + 2.f, gridH() + 2.f, 3.f);
            nvgStrokeWidth(vg, 1.5f);
            nvgStrokeColor(vg, vx_drum_sequencer_ui::vxdColor(vx_drum_sequencer_ui::PLAYCOL, 0.45f));
            nvgStroke(vg);
        }

        // Lane flashes: the label cell washes white (accent: red) on a fire,
        // never for a muted lane (source :232).
        const float label_x = 1.5f;
        const float label_w = vx_drum_sequencer_ui::LANEW - 5.f;
        for (int l = 0; l < vx_drum_sequencer::LANES; l++)
        {
            if (flash[l] <= 0.01f || laneMuted(l)) continue;

            const float y = laneY(l) + vx_drum_sequencer_ui::CELL_GAP * 0.5f;
            const float h = lh - vx_drum_sequencer_ui::CELL_GAP;
            const unsigned int col = (l == vx_drum_sequencer::ACCENT_LANE)
                ? vx_drum_sequencer_ui::PAD_ON_ACC : vx_drum_sequencer_ui::PLAYCOL;

            nvgBeginPath(vg);
            nvgRoundedRect(vg, label_x, y, label_w, h, 3.f);
            nvgFillColor(vg, vx_drum_sequencer_ui::vxdColor(col, 0.6f * flash[l]));
            nvgFill(vg);
        }
    }

    // ── Edits (UI thread; DESIGN §4.6) ───────────────────────────────────────

    // Lamp c -> this memory's length = c + 1 (source :305-313).
    void setLength(int steps)
    {
        if (!module) return;

        vx_drum_sequencer::Bank b = module->bankCopy();
        b.memories[slot()].length = rack::math::clamp(steps, 1, vx_drum_sequencer::STEPS);
        vx_drum_sequencer_ui::commitBankEdit(module, "length", b, module->mute);
    }

    // Lane label -> toggle that lane's mute bit; the accent lane mutes too
    // (source :314-322). Mute travels in the same action type as the bank.
    void toggleMute(int lane)
    {
        if (!module) return;

        const uint8_t after = (uint8_t)(module->mute ^ (1u << lane));
        vx_drum_sequencer_ui::commitBankEdit(module, "mute", module->bankCopy(), after);
    }

    // One pad of the paint gesture. Published immediately so the change is
    // audible while the drag is still in progress (PianoRollEditorWidget.hpp
    // :2023-2027); the undo action waits for the drag to end.
    void paintPad(int lane, int step)
    {
        if (!module) return;

        vx_drum_sequencer::Bank b = module->bankCopy();
        vx_drum_sequencer::Step& pad = b.memories[paint_slot].at(lane, step);
        if (pad.on == paint_value) return;

        pad.on = paint_value;

        module->publishBank(b);
    }

    void beginPaint(int lane, int step)
    {
        if (!module) return;

        edit_open = true;
        edit_before = module->bankCopy();
        edit_mute_before = module->mute;

        // ONE read of the audio-written report.slot per press: latch it, then
        // decide the value through slot(), which now resolves to paint_slot. A
        // padOn() read before the latch could name a different memory than the
        // one the drag paints (the MEM CV rewrites report.slot every sample).
        paint_slot = rack::math::clamp(module->report.slot, 0, vx_drum_sequencer::SLOTS - 1);
        drag_mode = DRAG_PAINT;
        paint_value = !padOn(lane, step);

        paintPad(lane, step);
        last_lane = lane;
        last_step = step;
    }

    void endPaint(const char* name = "VX Drum Sequencer edit")
    {
        if (module && edit_open)
        {
            vx_drum_sequencer_ui::pushBankEdit(module, name, edit_before, edit_mute_before);
        }

        edit_open = false;
        drag_mode = DRAG_NONE;
        last_lane = -1;
        last_step = -1;
        chance_lane = -1;
        chance_step = -1;
    }

    // ── Chance mode's gesture ────────────────────────────────────────────────

    // The press: open the undo bracket and latch the slot like beginPaint,
    // but decide nothing yet. Whether this is a click (toggle) or a drag
    // (chance) is settled by the first vertical move, or by the release.
    void beginPending(int lane, int step)
    {
        if (!module) return;

        edit_open = true;
        edit_before = module->bankCopy();
        edit_mute_before = module->mute;

        paint_slot = rack::math::clamp(module->report.slot, 0, vx_drum_sequencer::SLOTS - 1);
        drag_mode = DRAG_PENDING;
        chance_lane = lane;
        chance_step = step;
        press_y = drag_position.y;
    }

    // One step of the chance drag: the pad's chance is the pointer's height
    // within the pad's inner rect — the top edge is 100 %, the bottom 0 %,
    // and past either edge it pins (Digital Sequencer's editBar rule, so the
    // bar sits under the cursor). Published immediately so the odds change
    // while the drag is in progress; the undo action waits for the release.
    void dragChance()
    {
        if (!module) return;

        const float top = laneY(chance_lane) + vx_drum_sequencer_ui::CELL_GAP * 0.5f + 1.f;
        const float inner_h = laneH() - vx_drum_sequencer_ui::CELL_GAP - 2.f;
        const float frac = (inner_h > 0.f) ? 1.f - (drag_position.y - top) / inner_h : 1.f;
        const int chance = rack::math::clamp((int)std::lround(frac * (float)vx_drum_sequencer::CHANCE_MAX),
                                             0, vx_drum_sequencer::CHANCE_MAX);

        vx_drum_sequencer::Bank b = module->bankCopy();
        vx_drum_sequencer::Step& pad = b.memories[paint_slot].at(chance_lane, chance_step);
        if (pad.chance == chance) return;

        pad.chance = (uint8_t)chance;
        module->publishBank(b);
    }

    // The release of a press that never became a drag: a click, so toggle.
    void togglePending()
    {
        if (!module) return;

        paint_value = !module->liveBank().memories[paint_slot].at(chance_lane, chance_step).on;
        paintPad(chance_lane, chance_step);
    }

    // ── Ratchet menu (source menuAt :264-285) ────────────────────────────────
    // Only a LIT voice pad has one: a ratchet is a property of a hit, and the
    // accent lane has none. Returns whether it opened, so the press is consumed
    // only then (anywhere else the module's own menu must open).
    //
    // The slot is read ONCE for the whole gesture: the lit test, the checked
    // item and the slot the lambdas write to all come from that one memory.
    // The audio thread rewrites report.slot every sample, so separate reads
    // through padOn() / slot() / hitsAt() could each name a different memory.
    //
    // A lit VOICE pad gets the ratchet entries and a Chance submenu; a lit
    // ACCENT pad gets the Chance submenu alone (the accent lane has no
    // ratchets). An unlit pad opens nothing.
    bool openPadMenu(int lane, int step)
    {
        if (!module) return false;
        if (lane < 0 || lane >= vx_drum_sequencer::LANES) return false;

        VXDrumSequencer* m = module;
        const int s = slot();
        const vx_drum_sequencer::Step pad = module->liveBank().memories[s].at(lane, step);   // a copy: one read
        if (!pad.on) return false;

        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel(std::string(vx_drum_sequencer::LANE_NAMES[lane]) + " · step " + std::to_string(step + 1)));

        if (lane < vx_drum_sequencer::VOICES)
        {
            const int current = pad.ratchet;
            for (int n = 0; n <= vx_drum_sequencer::RATCHET_MAX; n++)
            {
                const std::string label = (n == 0) ? std::string("Single") : ("Ratchet ×" + std::to_string(n + 1));
                menu->addChild(createMenuItem(label, CHECKMARK(n == current),
                    [m, s, lane, step, n]() { vx_drum_sequencer_ui::setRatchet(m, s, lane, step, n); },
                    n == current));
            }
            menu->addChild(new MenuSeparator);
        }

        // Chance: the percent probability the pad plays when its step is
        // reached, rolled once per step (Bret, 2026-09-07).
        const int chance = pad.chance;
        menu->addChild(createSubmenuItem("Chance", std::to_string(chance) + " %", [m, s, lane, step, chance](ui::Menu* sub) {
            for (int i = 0; i < 6; i++)
            {
                const int p = vx_drum_sequencer_ui::CHANCE_PRESETS[i];
                sub->addChild(createMenuItem(std::to_string(p) + " %", CHECKMARK(p == chance),
                    [m, s, lane, step, p]() { vx_drum_sequencer_ui::setChance(m, s, lane, step, p); },
                    p == chance));
            }
        }));
        return true;
    }

    // Shift-click on a LIT voice pad steps its ratchet Single -> x2 -> x3 -> x4
    // -> Single (Bret, 2026-09-02: a faster gesture than the right-click menu).
    // One slot() read for the whole gesture, like openPadMenu. Returns true
    // when it handled a pad (lit or not), so the press never falls through to
    // painting. The accent lane has no ratchets and is left alone.
    bool cycleRatchet(int lane, int step)
    {
        if (!module) return false;
        if (lane < 0 || lane >= vx_drum_sequencer::VOICES) return false;

        const int s = slot();
        const vx_drum_sequencer::Step pad = module->liveBank().memories[s].at(lane, step);   // a copy: one read
        if (!pad.on) return true;   // unlit: swallow, do nothing

        vx_drum_sequencer_ui::setRatchet(module, s, lane, step, (pad.ratchet + 1) % (vx_drum_sequencer::RATCHET_MAX + 1));
        return true;
    }

    // ── Events ───────────────────────────────────────────────────────────────
    void onButton(const ButtonEvent& e) override
    {
        // The browser preview is not interactive.
        if (!module)
        {
            OpaqueWidget::onButton(e);
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS)
        {
            int lane = -1;
            int step = -1;
            if (cellAt(e.pos, lane, step) && openPadMenu(lane, step))
            {
                e.consume(this);
                return;
            }

            // Not ours: leave it alone so the module's own context menu opens.
            OpaqueWidget::onButton(e);
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            e.consume(this);   // required, or no drag events follow
            drag_position = e.pos;
            drag_mode = DRAG_NONE;

            const int lamp = lampAt(e.pos);
            if (lamp >= 0)
            {
                setLength(lamp + 1);
                return;
            }

            const int lane = laneAt(e.pos);
            if (lane >= 0)
            {
                toggleMute(lane);
                return;
            }

            int cell_lane = -1;
            int cell_step = -1;
            if (cellAt(e.pos, cell_lane, cell_step))
            {
                // Shift held: cycle the pad's ratchet instead of painting.
                if ((e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT && cycleRatchet(cell_lane, cell_step))
                {
                    return;
                }
                if (chanceMode()) beginPending(cell_lane, cell_step);
                else beginPaint(cell_lane, cell_step);
            }
            return;
        }

        OpaqueWidget::onButton(e);
    }

    void onDragMove(const DragMoveEvent& e) override
    {
        if (module && drag_mode == DRAG_PAINT)
        {
            // DragMoveEvent carries only a delta, so the absolute position is
            // accumulated; dividing by the zoom keeps one screen pixel equal to
            // one widget pixel at any rack zoom (PianoRollEditorWidget.hpp:1650-1658).
            float zoom = getAbsoluteZoom();
            if (zoom <= 0.f) zoom = 1.f;   // TimelineEditor.hpp:577-579
            drag_position = drag_position.plus(e.mouseDelta.div(zoom));

            // A pointer over a gap or outside the grid paints nothing and the
            // gesture stays open (source updateDragAbsolute :330-334).
            int lane = -1;
            int step = -1;
            if (cellAt(drag_position, lane, step) && (lane != last_lane || step != last_step))
            {
                paintPad(lane, step);
                last_lane = lane;
                last_step = step;
            }
        }
        else if (module && (drag_mode == DRAG_PENDING || drag_mode == DRAG_CHANCE))
        {
            float zoom = getAbsoluteZoom();
            if (zoom <= 0.f) zoom = 1.f;
            drag_position = drag_position.plus(e.mouseDelta.div(zoom));

            // A pending press on a LIT pad becomes a chance drag once the
            // pointer has clearly moved vertically; on an unlit pad it stays a
            // click, whatever the pointer does.
            if (drag_mode == DRAG_PENDING
                && std::fabs(drag_position.y - press_y) > CHANCE_DRAG_PX
                && module->liveBank().memories[paint_slot].at(chance_lane, chance_step).on)
            {
                drag_mode = DRAG_CHANCE;
            }
            if (drag_mode == DRAG_CHANCE) dragChance();
        }

        OpaqueWidget::onDragMove(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        if (drag_mode == DRAG_PAINT) endPaint();
        else if (drag_mode == DRAG_PENDING) { togglePending(); endPaint(); }
        else if (drag_mode == DRAG_CHANCE) endPaint("chance");

        drag_mode = DRAG_NONE;
        OpaqueWidget::onDragEnd(e);
    }
};
