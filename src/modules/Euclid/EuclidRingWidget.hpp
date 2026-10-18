#pragma once
//
// Euclid — the ring: a circular display of the pattern AND the rotation
// control. A port of vxsynth's PanelRing (panel-ring.js); line numbers in
// comments below cite that file unless another is named.
//
// Drawing (_redraw): N step dots around a circle from 12 o'clock clockwise,
// hits filled in the accent colour, rests hollow on the well colour, a faint
// guide circle, a short tick outside the ring at step 0, the playhead as a
// white ring around the current dot, and `hits/steps` above the centre with
// `ROT r` below it. The JS ring is a fixed 112 px square with a 44 px ring;
// this widget is sized from the panel's `ring_display` rect (84 px, plan §7)
// and every radius is scaled from 44 by the same factor, so the proportions
// are the source's at any plate size. The plate is dark in BOTH themes (the
// `_display` rule in the panel generator), so the colours are fixed, taken
// from panel-theme.js, not theme-mapped.
//
// What it draws is the EFFECTIVE pattern (knob + CV: decision 6), read from
// the module's Snapshot (Euclid.hpp) in draw(); a torn read costs one frame.
// Without a module (the browser preview) it draws the default 4/16 pattern
// with the playhead on step 1 (PianoRollWidget.hpp:101-107 idiom), computed
// by the engine's own Pattern so the preview can never disagree with the DSP.
// A module that has not run process() yet (added or loaded while bypassed:
// Rack does not process a bypassed module) draws the pattern its own knobs
// describe, the same way, and a rim drag divides the circle by those steps.
//
// Interaction (decision 5):
//   * a LEFT press inside the rim (ring radius + 9 px, hitTest :62-66) is
//     consumed, so Rack does not start a module drag, and begins a rotation
//     drag; outside the rim it is left alone and the module drags as usual.
//   * onDragMove turns the angular delta of the pointer about the centre
//     (atan2, unwrapped to ±π, updateDragAbsolute :99-107) into whole steps
//     and writes ROT_PARAM through its ParamQuantity, wrapped into
//     0..steps-1 (_emit :115-125). The drag position is accumulated from
//     mouseDelta / getAbsoluteZoom() (PianoRollEditorWidget.hpp:1650-1658).
//   * DIRECTION: the sign is NEGATIVE, the one place this is not a copy of
//     the JS (design §6.2). pattern[i] = base[(i + rot) % s], so a larger ROT
//     pulls the hits to EARLIER indices, counter-clockwise on a clockwise
//     ring; the JS adds a clockwise delta to rot and the dots run backwards
//     under the pointer. Here dragging clockwise moves the dots clockwise.
//     The DSP sign is the contract and does not change.
//   * ONE history::ParamChange per gesture, pushed at drag end only if the
//     value changed (VXDrumsWidget.hpp:425-430 idiom), so undo works like a
//     knob.
//   * double-click inside the rim sets ROT to 0 (doubleClick :76-81), one
//     history entry. Rack sends the DoubleClick after the second press's
//     Button event, so the drag that press began is cancelled here and its
//     release pushes nothing.
//   * a RIGHT press is never consumed, so the module's own context menu opens
//     over the ring (VXDrumsKitDisplay, VXDrumsWidget.hpp:246-256).
//   * the hover is consumed only while the pointer is inside the rim, which
//     is what makes Rack send the Leave that clears the hot state
//     (Widget.hpp:336-338); the guide circle brightens while hot or dragging
//     (hover :68-75, _redraw :161-162).
//

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

struct EuclidRingWidget : TransparentWidget
{
    // ── panel-theme.js colours, fixed (the plate is dark in both themes) ─────
    static const unsigned int WELL = 0x17171a;            // the plate's own fill (generate_euclid_panel.py)
    static const unsigned int ACCENT = 0xffb454;          // panel-theme.js:12
    static const unsigned int ACCENT_BRIGHT = 0xffd9a0;   // :13, while dragging
    static const unsigned int TICK = 0x717d8d;            // :26, guide circle, rests, the step-0 tick
    static const unsigned int TEXT_LABEL = 0x97a3b2;      // :25, the ROT line
    static const unsigned int PLAYHEAD = 0xffffff;

    Euclid* module = NULL;

    bool hot = false;                  // pointer inside the rim (guide circle alpha)
    bool dragging = false;
    bool last_press_inside = false;    // for onDoubleClick, which carries no position
    Vec drag_position;                 // accumulated pointer position, widget coordinates
    float drag_start_angle = 0.f;      // atan2 of the press about the centre
    float drag_start_value = 0.f;      // ROT_PARAM as the gesture began (the history entry's oldValue)
    int drag_start_rot = 0;

    EuclidRingWidget(Euclid* module, rack::Rect bounds)
    {
        this->module = module;
        box.pos = bounds.pos;
        box.size = bounds.size;
    }

    // ── geometry ─────────────────────────────────────────────────────────────

    static float tau() { return 6.28318530717958647692f; }

    Vec centre() const { return box.size.mult(0.5f); }

    // RING_R / SIZE = 44 / 112 (:24-25).
    float ringRadius() const { return 0.39f * std::min(box.size.x, box.size.y); }

    // Every JS radius below is in 44-px-ring units; multiply by this.
    float scale() const { return ringRadius() / 44.f; }

    // hitTest :62-66: within the ring plus a 9 px margin.
    bool hitTest(Vec p) const
    {
        const Vec c = centre();
        const float dx = p.x - c.x;
        const float dy = p.y - c.y;
        const float r = ringRadius() + 9.f;
        return dx * dx + dy * dy <= r * r;
    }

    // _angleAt :83-85.
    float angleAt(Vec p) const
    {
        const Vec c = centre();
        return std::atan2(p.y - c.y, p.x - c.x);
    }

    // 0xRRGGBB + alpha -> NVGcolor (VXDrumSequencerGridWidget.hpp vxdColor idiom).
    static NVGcolor colour(unsigned int rgb, float alpha = 1.f)
    {
        return nvgRGBA((unsigned char)((rgb >> 16) & 0xffu),
                       (unsigned char)((rgb >> 8) & 0xffu),
                       (unsigned char)(rgb & 0xffu),
                       (unsigned char)(rack::math::clamp(alpha, 0.f, 1.f) * 255.f + 0.5f));
    }

    // ── what to draw this frame ──────────────────────────────────────────────

    // The module's snapshot when it has one. Until its first process() call
    // (a module added or loaded while bypassed) the snapshot is not valid, so
    // the pattern is computed from the module's own knobs, CV aside, with the
    // playhead on step 1; the browser (no module) gets the defaults. Both go
    // through the engine's Pattern, so neither can disagree with the DSP.
    Euclid::Snapshot view() const
    {
        if (module && module->snapshot.valid) return module->snapshot;

        static euclid::Pattern preview;
        preview.compute(knobOrDefault(Euclid::HITS_PARAM, euclid::DEFAULT_HITS),
                        knobOrDefault(Euclid::STEPS_PARAM, euclid::DEFAULT_STEPS),
                        knobOrDefault(Euclid::ROT_PARAM, euclid::DEFAULT_ROT));

        Euclid::Snapshot s;
        s.hits = preview.hits;
        s.steps = preview.steps;
        s.rot = preview.rot;
        s.position = 0;
        for (int i = 0; i < euclid::MAX_STEPS; i++) s.hit[i] = preview.hit[i];
        s.valid = true;
        return s;
    }

    // A knob's integer value when there is a module, else the browser default.
    int knobOrDefault(int param_id, int fallback) const
    {
        if (!module) return fallback;
        return (int)std::round(module->params[param_id].getValue());
    }

    // The step count a drag divides the circle by: the effective steps the
    // ring shows (the JS uses its live `steps`, syncData :127-134). view()
    // already resolves the snapshot, the knobs and the browser default.
    int dragSteps() const
    {
        return rack::math::clamp(view().steps, 1, euclid::MAX_STEPS);
    }

    // ── drawing (_redraw :147-201) ───────────────────────────────────────────
    void draw(const DrawArgs& args) override
    {
        NVGcontext* vg = args.vg;
        nvgSave(vg);

        // The well. The panel's plate is the same colour and shape; painting
        // it again keeps the widget whole on its own.
        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.f, 0.f, box.size.x, box.size.y, 3.f);
        nvgFillColor(vg, colour(WELL));
        nvgFill(vg);

        const Euclid::Snapshot v = view();
        const int steps = rack::math::clamp(v.steps, 1, euclid::MAX_STEPS);
        const Vec c = centre();
        const float ring_r = ringRadius();
        const float k = scale();
        const bool live = hot || dragging;

        // Guide circle (:161-162).
        nvgBeginPath(vg);
        nvgCircle(vg, c.x, c.y, ring_r);
        nvgStrokeWidth(vg, 1.f);
        nvgStrokeColor(vg, colour(TICK, live ? 0.4f : 0.25f));
        nvgStroke(vg);

        // Dots (:164-190): 4.4 px, or 3.2 px when the ring is crowded.
        const float dot_r = (steps > 16 ? 3.2f : 4.4f) * k;
        for (int i = 0; i < steps; i++)
        {
            const float a = -tau() / 4.f + ((float)i / (float)steps) * tau();   // 12 o'clock, clockwise
            const float x = c.x + std::cos(a) * ring_r;
            const float y = c.y + std::sin(a) * ring_r;

            if (v.hit[i])
            {
                nvgBeginPath(vg);
                nvgCircle(vg, x, y, dot_r);
                nvgFillColor(vg, colour(dragging ? ACCENT_BRIGHT : ACCENT));
                nvgFill(vg);
            }
            else
            {
                nvgBeginPath(vg);
                nvgCircle(vg, x, y, dot_r - k);
                nvgFillColor(vg, colour(WELL));
                nvgFill(vg);
                nvgStrokeWidth(vg, 1.f);
                nvgStrokeColor(vg, colour(TICK, 0.5f));
                nvgStroke(vg);
            }

            if (i == 0)
            {
                // Step-0 marker: a small tick outside the ring (:178-183).
                const float r0 = ring_r + dot_r + 2.f * k;
                const float r1 = ring_r + dot_r + 5.5f * k;
                nvgBeginPath(vg);
                nvgMoveTo(vg, c.x + std::cos(a) * r0, c.y + std::sin(a) * r0);
                nvgLineTo(vg, c.x + std::cos(a) * r1, c.y + std::sin(a) * r1);
                nvgStrokeWidth(vg, 1.2f);
                nvgStrokeColor(vg, colour(TICK, 0.8f));
                nvgStroke(vg);
            }

            if (i == v.position)
            {
                // The playhead (:185-188).
                nvgBeginPath(vg);
                nvgCircle(vg, x, y, dot_r + 2.6f * k);
                nvgStrokeWidth(vg, 1.6f);
                nvgStrokeColor(vg, colour(PLAYHEAD, 0.9f));
                nvgStroke(vg);
            }
        }

        // Centre text (:44-48, :193-196): the count above the centre line,
        // the rotation below it. Loaded every draw, never cached across
        // frames (OneZeroReadoutWidget.hpp:40-48).
        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
        if (font)
        {
            nvgFontFaceId(vg, font->handle);
            nvgTextLetterSpacing(vg, 0.f);

            const std::string count = std::to_string(std::min(v.hits, steps)) + "/" + std::to_string(steps);
            nvgFontSize(vg, 9.f);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);
            nvgFillColor(vg, colour(dragging ? ACCENT_BRIGHT : ACCENT));
            nvgText(vg, c.x, c.y + 2.f * k, count.c_str(), NULL);

            const std::string rot = "ROT " + std::to_string(v.rot);
            nvgFontSize(vg, 6.f);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
            nvgFillColor(vg, colour(TEXT_LABEL));
            nvgText(vg, c.x, c.y + 4.f * k, rot.c_str(), NULL);
        }

        nvgRestore(vg);
        TransparentWidget::draw(args);
    }

    // ── the rotation control ─────────────────────────────────────────────────

    ParamQuantity* rotQuantity() const
    {
        return module ? module->getParamQuantity(Euclid::ROT_PARAM) : NULL;
    }

    // One undo step for a completed gesture (VXDrumsWidget.hpp:425-430).
    // Actions never hold module pointers; ParamChange resolves through
    // APP->engine->getModule(moduleId) each time.
    void pushChange(float old_value, float new_value, const char* name)
    {
        if (!module || old_value == new_value) return;

        history::ParamChange* change = new history::ParamChange;
        change->name = name;
        change->moduleId = module->id;
        change->paramId = Euclid::ROT_PARAM;
        change->oldValue = old_value;
        change->newValue = new_value;
        APP->history->push(change);
    }

    void onHover(const HoverEvent& e) override
    {
        hot = hitTest(e.pos);
        if (hot) e.consume(this);   // so the Leave below arrives when the pointer moves off
    }

    void onLeave(const LeaveEvent& e) override
    {
        hot = false;
        TransparentWidget::onLeave(e);
    }

    void onButton(const ButtonEvent& e) override
    {
        // Anything but a left press falls through: a right press must reach
        // the module widget for its context menu.
        if (e.button != GLFW_MOUSE_BUTTON_LEFT || e.action != GLFW_PRESS)
        {
            TransparentWidget::onButton(e);
            return;
        }

        last_press_inside = hitTest(e.pos);

        // Outside the rim, or in the browser: not ours, Rack drags the module.
        ParamQuantity* pq = rotQuantity();
        if (!last_press_inside || !pq)
        {
            TransparentWidget::onButton(e);
            return;
        }

        e.consume(this);   // required, or no drag events follow
        dragging = true;
        drag_position = e.pos;
        drag_start_angle = angleAt(e.pos);
        drag_start_value = pq->getValue();
        // The knob can sit above steps-1 (ROT turned to 20, then STEPS turned
        // down to 8) while the ring shows the clamped effective value
        // (euclid.c's dsp_iclamp). Seed the gesture from what the ring shows,
        // clamped rather than wrapped, or its first step would jump the dots
        // several slots instead of one.
        drag_start_rot = rack::math::clamp((int)std::round(drag_start_value), 0, dragSteps() - 1);
    }

    void onDragMove(const DragMoveEvent& e) override
    {
        if (!dragging) return;
        ParamQuantity* pq = rotQuantity();
        if (!pq) return;

        // DragMoveEvent carries only a delta, so the absolute position is
        // accumulated; dividing by the zoom keeps one screen pixel equal to
        // one widget pixel at any rack zoom.
        float zoom = getAbsoluteZoom();
        if (zoom <= 0.f) zoom = 1.f;
        drag_position = drag_position.plus(e.mouseDelta.div(zoom));

        // Angular delta about the centre, unwrapped to ±π (:101-103).
        float delta = angleAt(drag_position) - drag_start_angle;
        while (delta > tau() / 2.f) delta -= tau();
        while (delta < -tau() / 2.f) delta += tau();

        // Whole steps (:104-105), SUBTRACTED so a clockwise drag moves the
        // dots clockwise (design §6.2), wrapped like _emit :118-121.
        const int steps = dragSteps();
        const float step_angle = tau() / (float)steps;
        const int delta_steps = (int)std::round(delta / step_angle);
        int rot = drag_start_rot - delta_steps;
        rot = ((rot % steps) + steps) % steps;

        if ((int)std::round(pq->getValue()) != rot)
        {
            pq->setValue((float)rot);
        }
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        if (!dragging) return;
        dragging = false;

        ParamQuantity* pq = rotQuantity();
        if (!pq) return;
        pushChange(drag_start_value, pq->getValue(), "change rotation");
    }

    // doubleClick :76-81: rotation back to 0. The second press of the pair
    // has already begun a drag (onButton above); cancel it so its release
    // pushes nothing, and push this one entry instead.
    void onDoubleClick(const DoubleClickEvent& e) override
    {
        if (!last_press_inside) return;
        ParamQuantity* pq = rotQuantity();
        if (!pq) return;

        dragging = false;

        const float old_value = pq->getValue();
        pq->setValue((float)euclid::DEFAULT_ROT);
        pushChange(old_value, (float)euclid::DEFAULT_ROT, "reset rotation");
    }
};
