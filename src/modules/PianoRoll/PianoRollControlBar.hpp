//
// PianoRollControlBar — the strip above the editor: SNAP, the eight track
// squares, and REC.
//
// One widget covering the whole bar with an internal zone dispatcher, the same
// shape as the editor. Three sibling widgets would mean three sets of box
// arithmetic to keep aligned with the panel rect.
//

using namespace piano_roll;

//
// A tinted plate behind one track's output pair, in that track's colour.
//
// Drawn in code rather than placed in the panel SVG so the colours come from
// PianoRollPalette — one source of truth — and stay correct on both panel themes.
// Kept faint: this is a wayfinding cue tying a jack pair to its notes, not a
// decoration that should compete with the editor.
//
struct PianoRollTrackLane : TransparentWidget
{
    int track = 0;

    PianoRollTrackLane(int track, rack::Rect bounds)
    {
        this->track = track;
        box.pos = bounds.pos;
        box.size = bounds.size;
    }

    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;
        const TrackColors &colors = trackColors(track);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 4.5f);
        nvgFillColor(vg, withAlpha(colors.fill, 0.22f));
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, 0.5f, 0.5f, box.size.x - 1.0f, box.size.y - 1.0f, 4.5f);
        nvgStrokeWidth(vg, 1.0f);
        nvgStrokeColor(vg, withAlpha(colors.edge, 0.55f));
        nvgStroke(vg);

        TransparentWidget::draw(args);
    }
};

//
// TransparentWidget, deliberately NOT OpaqueWidget: an OpaqueWidget consumes every
// button press that lands anywhere in its box, so the empty space around these
// controls would swallow the drag that moves the module. Only a press that
// actually hits a control is consumed here; everything else propagates to the
// ModuleWidget as if this widget were not there.
//
struct PianoRollControlBar : TransparentWidget
{
    PianoRoll *module = NULL;

    // Layout within the bar, all derived from its height so the bar can be
    // resized in the panel SVG without touching this.
    static constexpr float PAD = 8.0f;
    static constexpr float SNAP_BUTTON_W = 64.0f;
    // Space left clear for the "SNAP" label, which is drawn in the panel SVG.
    static constexpr float SNAP_LABEL_SPACE = 34.0f;
    // Nudge right off the label, a fifth of the button's own width.
    static constexpr float SNAP_NUDGE = SNAP_BUTTON_W / 5.0f;
    static constexpr float TRACK_GAP = 6.0f;
    static constexpr float REC_W = 46.0f;
    static constexpr float LOCK_W = 46.0f;

    // The control bar is panel chrome, not a display, so unlike the editor screen
    // it FOLLOWS THE PANEL THEME. rack::settings::preferDarkPanels is the same flag
    // ThemedSvgPanel switches on.
    bool darkPanel() const { return rack::settings::preferDarkPanels; }

    NVGcolor barBackground() const { return darkPanel() ? nvgRGB(0x18, 0x1c, 0x22) : nvgRGB(0xdc, 0xdf, 0xe3); }
    NVGcolor barEdge() const { return darkPanel() ? nvgRGB(0x0c, 0x0e, 0x12) : nvgRGB(0xb4, 0xb8, 0xbd); }
    NVGcolor labelText() const { return darkPanel() ? nvgRGB(0x8f, 0xa8, 0xa8) : nvgRGB(0x3a, 0x42, 0x48); }
    // Deliberately LIGHTER than the bar in dark mode: the editor's screen colour
    // (#102020) is almost the same value as the dark bar, so a readout painted in
    // it disappeared into the background.
    NVGcolor valueBackground() const { return darkPanel() ? nvgRGB(0x2c, 0x34, 0x3a) : nvgRGB(0xf6, 0xf8, 0xfa); }
    NVGcolor valueEdge() const { return darkPanel() ? nvgRGB(0x4a, 0x56, 0x5e) : nvgRGB(0xb0, 0xb6, 0xbc); }
    NVGcolor valueText() const { return darkPanel() ? nvgRGB(0xd8, 0xdf, 0xdf) : nvgRGB(0x1a, 0x22, 0x26); }
    NVGcolor rec_armed_color = nvgRGB(0xd2, 0x3b, 0x3b);
    NVGcolor rec_idle_color = nvgRGB(0x3a, 0x42, 0x42);

    PianoRollControlBar(PianoRoll *module, rack::Rect bounds)
    {
        this->module = module;
        box.pos = bounds.pos;
        box.size = bounds.size;
    }

    // In the module browser there is no Module, only the widget. Rather than draw
    // nothing, fall back to the values a freshly-added module would have, so the
    // thumbnail shows what the controls actually look like.
    int snapSteps() const { return module ? module->snap_steps : 1; }
    int activeTrack() const { return module ? module->active_track : 0; }
    bool recArmed() const { return module && module->rec_armed; }
    bool isLocked() const { return module && module->locked; }

    float rowCenterY() const { return box.size.y * 0.5f; }
    float buttonHeight() const { return std::min(22.0f, box.size.y - 12.0f); }

    rack::Rect snapRect() const
    {
        float h = buttonHeight();
        return rack::Rect(Vec(PAD + SNAP_LABEL_SPACE + SNAP_NUDGE, rowCenterY() - h * 0.5f),
                          Vec(SNAP_BUTTON_W, h));
    }

    float trackSquareSize() const { return std::min(26.0f, box.size.y - 8.0f); }

    rack::Rect trackRect(int track) const
    {
        float size = trackSquareSize();
        float total = PianoRoll::TRACKS * size + (PianoRoll::TRACKS - 1) * TRACK_GAP;
        float left = (box.size.x - total) * 0.5f;

        return rack::Rect(Vec(left + track * (size + TRACK_GAP), rowCenterY() - size * 0.5f),
                          Vec(size, size));
    }

    rack::Rect recRect() const
    {
        float h = buttonHeight();
        return rack::Rect(Vec(box.size.x - PAD - REC_W, rowCenterY() - h * 0.5f), Vec(REC_W, h));
    }

    rack::Rect lockRect() const
    {
        float h = buttonHeight();
        return rack::Rect(Vec(box.size.x - PAD - REC_W - 6.0f - LOCK_W, rowCenterY() - h * 0.5f), Vec(LOCK_W, h));
    }

    static std::string snapLabel(int steps)
    {
        switch (steps)
        {
            case 1:  return "1/16";
            case 2:  return "1/8";
            case 4:  return "1/4";
            case 8:  return "1/2";
            case 16: return "Bar";
            default: return std::to_string(steps);
        }
    }

    // ── Drawing ──────────────────────────────────────────────────────────────

    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;

        // No background is drawn here on purpose. The bar's backing plate lives in
        // the panel SVG (the `control_bar` rect), so its look is controlled in
        // Inkscape alongside the rest of the artwork. This widget draws only the
        // controls that sit on top of it.
        //
        // barBackground() is still used to pick the track numeral ink, since that
        // depends on what the squares composite against.

        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));

        drawSnap(vg, font);
        drawTracks(vg, font);
        drawLock(vg, font);
        drawRec(vg, font);

        TransparentWidget::draw(args);
    }

    void drawSnap(NVGcontext *vg, std::shared_ptr<Font> font)
    {
        rack::Rect r = snapRect();

        // The "SNAP" caption is part of the panel artwork, not drawn here.

        nvgBeginPath(vg);
        nvgRoundedRect(vg, r.pos.x, r.pos.y, r.size.x, r.size.y, 3.0f);
        nvgFillColor(vg, valueBackground());
        nvgFill(vg);

        if (font)
        {
            // nvgFontFaceId is set explicitly: drawSnap runs FIRST, so unlike the
            // later draws it cannot inherit a face from a previous call.
            nvgFontSize(vg, 12.5f);
            nvgFontFaceId(vg, font->handle);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFillColor(vg, valueText());
            nvgText(vg, r.pos.x + r.size.x * 0.5f, rowCenterY(),
                    snapLabel(snapSteps()).c_str(), NULL);
        }
    }

    //
    // Eight squares in their track colours, the active one lit.
    //
    // Each carries its NUMBER: eight hues on a dark ground is at the edge of what
    // colour-vision-deficient viewers can separate, and the numeral is the
    // non-colour channel that makes the control usable regardless.
    //
    void drawTracks(NVGcontext *vg, std::shared_ptr<Font> font)
    {
        for (int track = 0; track < PianoRoll::TRACKS; track++)
        {
            rack::Rect r = trackRect(track);
            const TrackColors &colors = trackColors(track);
            bool active = (track == activeTrack());

            nvgBeginPath(vg);
            nvgRoundedRect(vg, r.pos.x, r.pos.y, r.size.x, r.size.y, 2.5f);
            nvgFillColor(vg, active ? colors.selected_fill : withAlpha(colors.fill, 0.45f));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRoundedRect(vg, r.pos.x + 0.5f, r.pos.y + 0.5f, r.size.x - 1.0f, r.size.y - 1.0f, 2.5f);
            nvgStrokeWidth(vg, active ? 1.5f : 1.0f);
            nvgStrokeColor(vg, active ? colors.selected_edge : withAlpha(colors.edge, 0.7f));
            nvgStroke(vg);

            if (font)
            {
                // Pick the ink from what the square ACTUALLY ends up looking like —
                // the fill composited over the bar at its drawn opacity — rather
                // than from the track index. Hard-coding "the first four are dark"
                // silently becomes wrong the moment the palette is retuned.
                NVGcolor square = active ? colors.selected_fill : colors.fill;
                float alpha = active ? 1.0f : 0.45f;
                NVGcolor bar = barBackground();

                float luminance = 0.2126f * (square.r * alpha + bar.r * (1.0f - alpha))
                                + 0.7152f * (square.g * alpha + bar.g * (1.0f - alpha))
                                + 0.0722f * (square.b * alpha + bar.b * (1.0f - alpha));

                bool light_square = luminance > 0.5f;

                nvgFontSize(vg, 13.0f);
                nvgFontFaceId(vg, font->handle);
                nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                nvgFillColor(vg, light_square ? nvgRGBA(0x10, 0x18, 0x1c, active ? 0xFF : 0xC8)
                                              : nvgRGBA(0xF0, 0xF4, 0xF4, active ? 0xFF : 0xC8));
                nvgText(vg, r.pos.x + r.size.x * 0.5f, r.pos.y + r.size.y * 0.5f,
                        std::to_string(track + 1).c_str(), NULL);
            }
        }
    }

    void drawLock(NVGcontext *vg, std::shared_ptr<Font> font)
    {
        rack::Rect r = lockRect();
        bool locked = isLocked();

        nvgBeginPath(vg);
        nvgRoundedRect(vg, r.pos.x, r.pos.y, r.size.x, r.size.y, 3.0f);
        nvgFillColor(vg, locked ? nvgRGB(0xb4, 0x1e, 0x1e) : valueBackground());
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, r.pos.x + 0.5f, r.pos.y + 0.5f, r.size.x - 1.0f, r.size.y - 1.0f, 3.0f);
        nvgStrokeWidth(vg, 1.0f);
        nvgStrokeColor(vg, locked ? nvgRGB(0xb4, 0x1e, 0x1e) : valueEdge());
        nvgStroke(vg);

        if (font)
        {
            nvgFontSize(vg, 11.5f);
            nvgFontFaceId(vg, font->handle);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFillColor(vg, locked ? nvgRGB(0xFF, 0xFF, 0xFF) : labelText());
            nvgText(vg, r.pos.x + r.size.x * 0.5f, r.pos.y + r.size.y * 0.5f, "LOCK", NULL);
        }
    }

    void drawRec(NVGcontext *vg, std::shared_ptr<Font> font)
    {
        rack::Rect r = recRect();
        bool armed = recArmed();

        nvgBeginPath(vg);
        nvgRoundedRect(vg, r.pos.x, r.pos.y, r.size.x, r.size.y, 3.0f);
        nvgFillColor(vg, armed ? rec_armed_color : valueBackground());
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, r.pos.x + 0.5f, r.pos.y + 0.5f, r.size.x - 1.0f, r.size.y - 1.0f, 3.0f);
        nvgStrokeWidth(vg, 1.0f);
        nvgStrokeColor(vg, armed ? rec_armed_color : rec_idle_color);
        nvgStroke(vg);

        if (font)
        {
            nvgFontSize(vg, 11.5f);
            nvgFontFaceId(vg, font->handle);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgFillColor(vg, armed ? nvgRGB(0xFF, 0xFF, 0xFF) : labelText());
            nvgText(vg, r.pos.x + r.size.x * 0.5f, r.pos.y + r.size.y * 0.5f, "REC", NULL);
        }
    }

    // ── Interaction ──────────────────────────────────────────────────────────

    // True only over an actual control, never over the bar's empty space.
    bool hitsControl(Vec position) const
    {
        if (snapRect().contains(position)) return true;
        if (lockRect().contains(position)) return true;
        if (recRect().contains(position)) return true;

        for (int track = 0; track < PianoRoll::TRACKS; track++)
        {
            if (trackRect(track).contains(position)) return true;
        }
        return false;
    }

    void onButton(const ButtonEvent &e) override
    {
        if (!module || e.action != GLFW_PRESS)
        {
            TransparentWidget::onButton(e);
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_LEFT)
        {
            for (int track = 0; track < PianoRoll::TRACKS; track++)
            {
                if (trackRect(track).contains(e.pos))
                {
                    // Switching tracks drops the selection: it belonged to the
                    // track being left, and only active-track notes are editable.
                    module->active_track = track;
                    module->selection.clear();
                    e.consume(this);
                    return;
                }
            }

            if (lockRect().contains(e.pos))
            {
                module->locked = !module->locked;
                e.consume(this);
                return;
            }

            if (recRect().contains(e.pos))
            {
                module->rec_armed = !module->rec_armed;
                e.consume(this);
                return;
            }

            if (snapRect().contains(e.pos)) { openSnapMenu(); e.consume(this); return; }
        }

        // Nothing was hit: leave the event alone so the module can be dragged.
        TransparentWidget::onButton(e);
    }

    void openSnapMenu()
    {
        static const int VALUES[5] = {1, 2, 4, 8, 16};

        ui::Menu *menu = createMenu();
        menu->addChild(createMenuLabel("Snap"));

        PianoRoll *piano_roll = module;
        for (int i = 0; i < 5; i++)
        {
            int steps = VALUES[i];
            menu->addChild(createCheckMenuItem(snapLabel(steps), "",
                [piano_roll, steps]() { return piano_roll->snap_steps == steps; },
                [piano_roll, steps]() { piano_roll->snap_steps = steps; }));
        }
    }

    // Created once. glfwCreateStandardCursor allocates, so calling it per hover
    // frame leaks a cursor every time the pointer moves.
    GLFWcursor *hand_cursor = NULL;
    bool cursor_set = false;

    void onHover(const HoverEvent &e) override
    {
        // The hand cursor marks the controls, not the whole strip — the empty
        // space is module-drag territory and should not advertise otherwise.
        bool over_control = module && hitsControl(e.pos);

        if (over_control && !hand_cursor) hand_cursor = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);

        if (over_control != cursor_set)
        {
            glfwSetCursor(APP->window->win, over_control ? hand_cursor : NULL);
            cursor_set = over_control;
        }

        TransparentWidget::onHover(e);
    }

    void onLeave(const LeaveEvent &e) override
    {
        if (cursor_set) { glfwSetCursor(APP->window->win, NULL); cursor_set = false; }
        TransparentWidget::onLeave(e);
    }
};
