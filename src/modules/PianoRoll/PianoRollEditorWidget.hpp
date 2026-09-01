//
// PianoRollEditorWidget — the editor surface: piano keys column, bar ruler, note
// grid and vertical scrollbar.
//
// Draws the grid, keys, ruler, scrollbar and notes, and handles navigation and
// note editing. The playhead, keyboard commands, the keys-column preview and the
// context menu are not wired up yet.
//
// SCREEN, NOT PAPER. The web original drew a light "paper" grid, but Voxglitch
// displays are dark regardless of the panel theme — CueResearch's TrackWidget
// fills itself nvgRGB(0x10, 0x20, 0x20) on a light panel, and GrooveBox's LCD
// schemes are all near-black. This follows that convention, which also means the
// eight-track note palette has to be designed against a dark ground rather than
// the light lanes assumed in rack-port-design.md section 8.
//
// VIEW STATE (zoom, scroll, top pitch) lives here and is deliberately NOT
// persisted: a pan is not an edit, and it should not travel in the patch file.
//

using namespace piano_roll;

//
// Note clipboard, shared by every Piano Roll instance in the patch — copy in one
// module, paste into another. Notes are stored RELATIVE to the earliest start in
// the copied block, and WITHOUT a track, so paste can place the block anywhere and
// always lands it on the receiving editor's active track. That is the only way to
// move material between tracks.
//
struct ClipboardNote { int pitch; int start_offset; int length; int velocity; };
static std::vector<ClipboardNote> note_clipboard;

//
// One undo step for a note edit.
//
// Native vectors rather than JSON snapshots: ~32 bytes a note, so a full 2048-note
// pattern is ~64 KB, against several hundred KB for the equivalent jansson tree.
// History never persists to the patch file, so there is no schema to keep stable.
//
// The module is referenced BY ID, never by pointer — Rack may delete and restore
// the object (undoing a module delete recreates it with the same id), so a stored
// pointer would dangle.
//
struct NoteEditAction : history::ModuleAction
{
    std::vector<Note> old_notes, new_notes;
    std::set<NoteId> old_selection, new_selection;
    int old_loop = DEFAULT_LOOP_STEPS, new_loop = DEFAULT_LOOP_STEPS;

    void apply(const std::vector<Note> &notes, const std::set<NoteId> &selection, int loop)
    {
        engine::Module *found = APP->engine->getModule(moduleId);
        if (!found) return;

        PianoRoll *piano_roll = dynamic_cast<PianoRoll *>(found);
        if (piano_roll) piano_roll->setNotes(notes, selection, loop);
    }

    void undo() override { apply(old_notes, old_selection, old_loop); }
    void redo() override { apply(new_notes, new_selection, new_loop); }
};

struct PianoRollEditorWidget : OpaqueWidget
{
    PianoRoll *module = NULL;
    Layout layout;

    // ── View state (transient) ───────────────────────────────────────────────
    float pixels_per_step = DEFAULT_PPS;
    float scroll_steps = 0.0f;               // leftmost visible step
    int top_pitch = DEFAULT_TOP_PITCH;       // pitch of the top row

    // ── Screen palette ───────────────────────────────────────────────────────
    // One place to retune the whole display.
    NVGcolor screen_background = nvgRGB(0x10, 0x20, 0x20);
    NVGcolor lane_natural = nvgRGB(0x18, 0x2a, 0x2a);
    NVGcolor lane_sharp = nvgRGB(0x12, 0x22, 0x22);
    // Darker than the screen itself, so a locked-out row reads as recessed.
    NVGcolor lane_out_of_scale = nvgRGB(0x0b, 0x15, 0x15);
    NVGcolor line_step = nvgRGB(0x22, 0x38, 0x38);
    NVGcolor line_beat = nvgRGB(0x2e, 0x48, 0x48);
    NVGcolor line_bar = nvgRGB(0x46, 0x66, 0x66);
    NVGcolor line_octave = nvgRGB(0x50, 0x74, 0x74);
    NVGcolor past_loop_dim = nvgRGBA(0x00, 0x00, 0x00, 0x59);
    NVGcolor loop_flag = nvgRGB(0xd0, 0x8a, 0x1e);
    NVGcolor playhead = nvgRGB(0x3f, 0xb9, 0x50);

    NVGcolor ruler_background = nvgRGB(0x0c, 0x18, 0x18);
    NVGcolor ruler_text = nvgRGB(0x8f, 0xa8, 0xa8);

    NVGcolor key_white = nvgRGB(0xd8, 0xdf, 0xdf);
    NVGcolor key_white_separator = nvgRGB(0x6a, 0x7a, 0x7a);
    NVGcolor key_black = nvgRGB(0x1a, 0x24, 0x24);
    NVGcolor key_black_edge = nvgRGB(0x08, 0x10, 0x10);
    NVGcolor key_gutter = nvgRGB(0x08, 0x12, 0x12);
    NVGcolor key_label = nvgRGB(0x62, 0x76, 0x76);

    NVGcolor scrollbar_trough = nvgRGB(0x14, 0x24, 0x24);
    NVGcolor scrollbar_thumb = nvgRGB(0x3a, 0x52, 0x52);

    static constexpr float KEY_GUTTER_W = 3.0f;
    static constexpr float BLACK_KEY_W_FRACTION = 0.6f;

    PianoRollEditorWidget(PianoRoll *module, Layout layout)
    {
        this->module = module;
        this->layout = layout;

        // Widget::box defaults to Vec(INFINITY, INFINITY), and Rect::contains has an
        // INFINITY branch — a widget that does not set its size passes the hit test
        // for every position in its parent and swallows all positional events.
        box.pos = layout.editor.pos;
        box.size = layout.editor.size;

        top_pitch = clampTopPitch(DEFAULT_TOP_PITCH);
    }

    // ── Coordinate mapping ───────────────────────────────────────────────────
    // All in widget-local pixels.

    float xOfStep(float step) const { return KEYS_W + (step - scroll_steps) * pixels_per_step; }
    float yOfPitch(int pitch) const { return RULER_H + (top_pitch - pitch) * ROW_H; }

    float stepAtX(float x) const { return scroll_steps + (x - KEYS_W) / pixels_per_step; }
    int pitchAtY(float y) const { return top_pitch - (int)std::floor((y - RULER_H) / ROW_H); }

    // The lowest pitch the view may be anchored at: far enough down that the bottom
    // row is pitch 0, and no further.
    int clampTopPitch(int pitch) const
    {
        return rack::math::clamp(pitch, layout.rows - 1, MAX_PITCH);
    }

    static bool isSharp(int pitch)
    {
        int pitch_class = ((pitch % 12) + 12) % 12;
        return pitch_class == 1 || pitch_class == 3 || pitch_class == 6
            || pitch_class == 8 || pitch_class == 10;
    }

    int loopSteps() const { return module ? std::max(1, module->loop_steps) : DEFAULT_LOOP_STEPS; }

    // Editing is frozen, but looking around is not.
    bool isLocked() const { return module && module->locked; }

    // ── Scale lock ───────────────────────────────────────────────────────────
    //
    // While active, a placement or move that targets an out-of-scale pitch
    // lands on the nearest in-scale pitch instead — the gesture always
    // succeeds, it just cannot produce a wrong note. Recording, paste and MIDI
    // import bypass this by design: they carry existing material.

    bool scaleLockActive() const
    {
        return module && module->scale_lock_enabled && module->scale_index >= 0
            && module->scale_index < (int)scaleDefinitions().size();
    }

    int restrictPitch(int pitch) const
    {
        if (!scaleLockActive()) return pitch;
        return nearestDegree(pitch, module->scale_root,
                             scaleDefinitions()[module->scale_index].degrees);
    }

    //
    // Drain notes captured on the audio thread. Runs once per frame on the UI
    // thread, which is the only thread allowed to touch the note list.
    //
    // Each captured note is its own undo step, unlike a mouse gesture — a recorded
    // phrase is therefore several steps rather than one. That follows from capture
    // being event-driven with no gesture boundary to close on.
    //
    void step() override
    {
        if (module)
        {
            unsigned int write = module->capture_write.load();

            // If the UI stalled long enough to overflow the ring, take the most
            // recent entries and say nothing rather than invent the lost ones.
            if (write - module->capture_read > (unsigned int)PianoRoll::CAPTURE_RING)
            {
                module->capture_read = write - PianoRoll::CAPTURE_RING;
            }

            if (module->locked) module->capture_read = write;   // discard, do not commit

            while (module->capture_read != write)
            {
                const PianoRoll::CapturedNote &captured =
                    module->capture_ring[module->capture_read % PianoRoll::CAPTURE_RING];
                module->capture_read++;

                beginEdit();
                module->addNote(captured.pitch, captured.start, captured.length, captured.track,
                                captured.velocity);
                endEdit("record note");
            }
        }

        updateFollow();
        OpaqueWidget::step();
    }

    // ── Drawing ──────────────────────────────────────────────────────────────

    void draw(const DrawArgs &args) override
    {
        const auto vg = args.vg;
        if (!layout.isValid()) return;

        nvgSave(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, box.size.y);
        nvgFillColor(vg, screen_background);
        nvgFill(vg);

        drawGrid(vg);
        drawNotes(vg);
        drawKeys(vg);
        drawScrollbar(vg);
        drawRuler(vg);
        drawRecordingNotes(vg);
        drawPlayhead(vg);
        drawOverlay(vg);
        drawVelocityLane(vg);
        drawCorner(vg);
        drawPreviewKey(vg);

        // A locked editor gets a red wash, the same signal the Tracks canvas uses.
        if (isLocked())
        {
            nvgBeginPath(vg);
            nvgRect(vg, 0, 0, box.size.x, box.size.y);
            nvgFillColor(vg, nvgRGBA(180, 30, 30, 25));
            nvgFill(vg);
        }

        nvgRestore(vg);

        OpaqueWidget::draw(args);
    }

    // Lanes, time lines and the past-the-loop dimming, clipped to the grid so
    // nothing bleeds over the keys column, the ruler or the scrollbar.
    void drawGrid(NVGcontext *vg)
    {
        nvgSave(vg);
        nvgScissor(vg, layout.grid.pos.x, layout.grid.pos.y, layout.grid.size.x, layout.grid.size.y);

        // Pitch lanes
        for (int row = 0; row < layout.rows; row++)
        {
            int pitch = top_pitch - row;
            if (pitch < MIN_PITCH) break;

            float y = RULER_H + row * ROW_H;

            // With the scale lock on, the sharp/natural zebra striping is
            // REPLACED, not overlaid: two alternating darks plus a third
            // "disabled" dark were unreadable together. Locked, the only
            // distinction that matters is in-scale (uniform, light) against
            // out-of-scale (recessed), and the striping returns when the lock
            // is off.
            NVGcolor row_fill;
            if (scaleLockActive())
            {
                bool in_scale = inScale(pitch, module->scale_root,
                                        scaleDefinitions()[module->scale_index].degrees);
                row_fill = in_scale ? lane_natural : lane_out_of_scale;
            }
            else
            {
                row_fill = isSharp(pitch) ? lane_sharp : lane_natural;
            }

            nvgBeginPath(vg);
            nvgRect(vg, layout.grid.pos.x, y, layout.grid.size.x, ROW_H);
            nvgFillColor(vg, row_fill);
            nvgFill(vg);

            // The octave boundary reads stronger than an ordinary lane edge.
            if (((pitch % 12) + 12) % 12 == 0)
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, layout.grid.pos.x, y + ROW_H);
                nvgLineTo(vg, layout.grid.pos.x + layout.grid.size.x, y + ROW_H);
                nvgStrokeWidth(vg, 1.0f);
                nvgStrokeColor(vg, line_octave);
                nvgStroke(vg);
            }
        }

        // Vertical time lines: step < beat < bar. Step lines are suppressed when
        // zoomed out, or they turn into a solid wash.
        int first_step = (int)std::floor(scroll_steps);
        int last_step = (int)std::ceil(scroll_steps + layout.visibleSteps(pixels_per_step));

        for (int step = first_step; step <= last_step; step++)
        {
            if (step < 0) continue;

            bool is_bar = (step % STEPS_PER_BAR) == 0;
            bool is_beat = (step % STEPS_PER_BEAT) == 0;

            if (!is_beat && pixels_per_step < STEP_LINE_MIN_PPS) continue;

            float x = xOfStep((float)step);

            nvgBeginPath(vg);
            nvgMoveTo(vg, x, layout.grid.pos.y);
            nvgLineTo(vg, x, layout.grid.pos.y + layout.grid.size.y);
            nvgStrokeWidth(vg, 1.0f);
            nvgStrokeColor(vg, is_bar ? line_bar : (is_beat ? line_beat : line_step));
            nvgStroke(vg);
        }

        // Everything past the loop end is dimmed: it is stored and drawn, but it
        // never plays.
        float loop_x = xOfStep((float)loopSteps());

        if (loop_x < layout.grid.pos.x + layout.grid.size.x)
        {
            float x = std::max(loop_x, layout.grid.pos.x);

            nvgBeginPath(vg);
            nvgRect(vg, x, layout.grid.pos.y, layout.grid.pos.x + layout.grid.size.x - x, layout.grid.size.y);
            nvgFillColor(vg, past_loop_dim);
            nvgFill(vg);
        }

        nvgRestore(vg);
    }

    //
    // Notes, in two passes: the other seven tracks first as dimmed context, then
    // the active track on top at full opacity. Drawing inactive tracks first is
    // what makes the active one sit visually above them where they overlap.
    //
    //
    // A pattern shown ONLY in the module browser, where there is no Module and so
    // no notes. An empty grid tells a browsing user nothing about what this module
    // does; a few bars across three tracks show it at a glance — a bass line, a
    // chord pad and a melody, with the active track at full strength and the
    // others dimmed, exactly as the real editor renders them.
    //
    // Drawn only. These never enter a patch and never reach the DSP.
    //
    static const std::vector<Note> &previewNotes()
    {
        static std::vector<Note> notes;

        if (notes.empty())
        {
            struct Seed { int pitch, start, length, track; };
            static const Seed SEEDS[] = {
                {48, 0, 6, 0}, {48, 8, 6, 0}, {55, 16, 6, 0}, {53, 24, 6, 0}, {48, 32, 6, 0},

                {60, 0, 15, 1}, {64, 0, 15, 1}, {67, 0, 15, 1},
                {59, 16, 15, 1}, {62, 16, 15, 1}, {67, 16, 15, 1},

                {72, 0, 3, 2}, {74, 4, 3, 2}, {76, 8, 3, 2}, {74, 12, 3, 2},
                {72, 20, 3, 2}, {69, 24, 3, 2}, {71, 28, 7, 2},
            };

            for (size_t i = 0; i < sizeof(SEEDS) / sizeof(SEEDS[0]); i++)
            {
                notes.push_back(Note((NoteId)(i + 1), SEEDS[i].pitch, SEEDS[i].start,
                                     SEEDS[i].length, SEEDS[i].track));
            }
        }

        return notes;
    }

    void drawNotes(NVGcontext *vg)
    {
        const std::vector<Note> &notes = module ? module->notes : previewNotes();
        int active = module ? module->active_track : 0;

        nvgSave(vg);
        nvgScissor(vg, layout.grid.pos.x, layout.grid.pos.y, layout.grid.size.x, layout.grid.size.y);

        int loop = loopSteps();

        for (size_t i = 0; i < notes.size(); i++)
        {
            if (notes[i].track != active) drawNote(vg, notes[i], loop, false, true);
        }

        for (size_t i = 0; i < notes.size(); i++)
        {
            if (notes[i].track != active) continue;
            bool selected = module && module->selection.count(notes[i].id) > 0;
            drawNote(vg, notes[i], loop, selected, false);
        }

        nvgRestore(vg);
    }

    void drawNote(NVGcontext *vg, const Note &note, int loop, bool selected, bool dimmed)
    {
        float y = yOfPitch(note.pitch);

        // Cull by pitch before doing any span arithmetic.
        if (y + ROW_H < layout.grid.pos.y) return;
        if (y > layout.grid.pos.y + layout.grid.size.y) return;

        const TrackColors &colors = trackColors(note.track);
        float alpha = dimmed ? INACTIVE_TRACK_ALPHA : 1.0f;

        // A note at or past the loop end can never play: the step counter only ever
        // takes values in [0, loop). Draw it outline-only so it reads as present but
        // silent — it is not merely dimmed, it is dead, and a Shift can still
        // resurrect it at a scrambled position.
        if (note.start >= loop)
        {
            drawNoteSpan(vg, (float)note.start, (float)note.end(), y, colors,
                         selected, OUT_OF_LOOP_ALPHA, true, false);
            return;
        }

        // A note longer than the loop re-triggers itself once per pass rather than
        // stacking, so it never draws longer than the loop.
        int length = std::min(note.length, loop);
        int end = note.start + length;

        if (end <= loop)
        {
            drawNoteSpan(vg, (float)note.start, (float)end, y, colors, selected, alpha, false, !dimmed);
        }
        else
        {
            // The note wraps. Draw the head, then the tail at step 0 — those steps
            // really do sound on the next pass, and the web original left them
            // invisible, which made the audible result impossible to read.
            drawNoteSpan(vg, (float)note.start, (float)loop, y, colors, selected, alpha, false, false);
            drawNoteSpan(vg, 0.0f, (float)(end - loop), y, colors, selected, alpha, false, !dimmed);
        }
    }

    void drawNoteSpan(NVGcontext *vg, float start_step, float end_step, float y,
                      const TrackColors &colors, bool selected, float alpha,
                      bool outline_only, bool show_grip)
    {
        float x0 = xOfStep(start_step);
        float x1 = xOfStep(end_step);

        // Cull horizontally.
        if (x1 < layout.grid.pos.x || x0 > layout.grid.pos.x + layout.grid.size.x) return;

        float width = std::max(NOTE_MIN_W, x1 - x0 - 1.0f);
        float top = y + 0.5f;
        float height = ROW_H - NOTE_INSET_Y;

        NVGcolor fill = withAlpha(selected ? colors.selected_fill : colors.fill, alpha);

        if (outline_only)
        {
            // Notes past the loop end are drawn as an outline and nothing else, so
            // they read as present but silent.
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x0, top, width, height, NOTE_CORNER_R);
            nvgStrokeWidth(vg, 1.0f);
            nvgStrokeColor(vg, withAlpha(colors.fill, alpha));
            nvgStroke(vg);
        }
        else
        {
            // Flat fill, no border. A 1 px stroke straddles the rectangle's edge at
            // half coverage, which softens exactly the boundary that makes a note
            // read as crisp — and at small note sizes the border was eating a good
            // part of the block. Selection is carried by the fill colour instead.
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x0, top, width, height, NOTE_CORNER_R);
            nvgFillColor(vg, fill);
            nvgFill(vg);
        }

        // Right-edge grip hint, once the note is wide enough to show one.
        if (show_grip && !outline_only && width > NOTE_GRIP_MIN_W)
        {
            nvgBeginPath(vg);
            nvgMoveTo(vg, x0 + width - 3.0f, top + 2.0f);
            nvgLineTo(vg, x0 + width - 3.0f, top + height - 2.0f);
            nvgStrokeWidth(vg, 1.0f);
            nvgStrokeColor(vg, nvgRGBA(0xFF, 0xFF, 0xFF, 0x59));
            nvgStroke(vg);
        }
    }

    //
    // The keys column: a real vertical keyboard, not a stack of uniform cells.
    //
    // Lanes are uniform (one semitone each), but WHITE keys are full-width
    // rectangles of NON-UNIFORM height: a white key's edge lands on the CENTRE of
    // an adjacent black key where one sits between two whites, and on the true lane
    // boundary where none does (E|F, B|C). Black keys are short and drawn on top.
    // That single rule produces the familiar interlocked keyboard.
    //
    // Note this is drawing only — hit testing uses the flat 9 px lane, so the drawn
    // key shapes and the clickable regions deliberately disagree at the edges.
    //
    void drawKeys(NVGcontext *vg)
    {
        nvgSave(vg);
        nvgScissor(vg, 0, RULER_H, KEYS_W, box.size.y - RULER_H);

        float keyboard_top = RULER_H;
        float keyboard_bottom = RULER_H + layout.grid.size.y;

        // The backs of the keys
        nvgBeginPath(vg);
        nvgRect(vg, 0, keyboard_top, KEYS_W, keyboard_bottom - keyboard_top);
        nvgFillColor(vg, key_gutter);
        nvgFill(vg);

        float key_left = KEY_GUTTER_W;
        float key_right = KEYS_W - 1.0f;
        float key_width = key_right - key_left;
        float black_key_width = std::floor(key_width * BLACK_KEY_W_FRACTION);

        // White keys first: they tile, with non-uniform heights.
        for (int row = 0; row < layout.rows; row++)
        {
            int pitch = top_pitch - row;
            if (pitch < MIN_PITCH) break;
            if (isSharp(pitch)) continue;

            float y = RULER_H + row * ROW_H;
            float top = isSharp(pitch + 1) ? y - ROW_H * 0.5f : y;
            float bottom = isSharp(pitch - 1) ? y + ROW_H * 1.5f : y + ROW_H;

            top = std::max(top, keyboard_top);
            bottom = std::min(bottom, keyboard_bottom);
            if (bottom <= top) continue;

            nvgBeginPath(vg);
            nvgRect(vg, key_left, top, key_width, bottom - top);
            nvgFillColor(vg, key_white);
            nvgFill(vg);

            if (bottom < keyboard_bottom)
            {
                nvgBeginPath(vg);
                nvgMoveTo(vg, key_left, bottom - 0.5f);
                nvgLineTo(vg, key_right, bottom - 0.5f);
                nvgStrokeWidth(vg, 1.0f);
                nvgStrokeColor(vg, key_white_separator);
                nvgStroke(vg);
            }
        }

        // Black keys on top of the white bed, one per lane.
        for (int row = 0; row < layout.rows; row++)
        {
            int pitch = top_pitch - row;
            if (pitch < MIN_PITCH) break;
            if (!isSharp(pitch)) continue;

            float y = RULER_H + row * ROW_H + 1.0f;
            float height = ROW_H - 2.0f;

            nvgBeginPath(vg);
            nvgRoundedRect(vg, key_left, y, black_key_width, height, 1.5f);
            nvgFillColor(vg, key_black);
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRoundedRect(vg, key_left + 0.5f, y + 0.5f, black_key_width - 1.0f, height - 1.0f, 1.5f);
            nvgStrokeWidth(vg, 1.0f);
            nvgStrokeColor(vg, key_black_edge);
            nvgStroke(vg);
        }

        drawOctaveLabels(vg, key_right);

        nvgRestore(vg);
    }

    // C labels, so the pitch range reads at a glance.
    void drawOctaveLabels(NVGcontext *vg, float key_right)
    {
        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
        if (!font) return;

        nvgFontSize(vg, 7.0f);
        nvgFontFaceId(vg, font->handle);
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgFillColor(vg, key_label);

        for (int row = 0; row < layout.rows; row++)
        {
            int pitch = top_pitch - row;
            if (pitch < MIN_PITCH) break;
            if (((pitch % 12) + 12) % 12 != 0) continue;

            // C4 = MIDI 60, the Rack/VCV convention that matches 0 V.
            std::string label = "C" + std::to_string(pitch / 12 - 1);
            float y = RULER_H + row * ROW_H + ROW_H * 0.5f;

            nvgText(vg, key_right - 2.0f, y, label.c_str(), NULL);
        }
    }

    // Bar numbers and the loop-end marker.
    void drawRuler(NVGcontext *vg)
    {
        nvgSave(vg);

        nvgBeginPath(vg);
        nvgRect(vg, 0, 0, box.size.x, RULER_H);
        nvgFillColor(vg, ruler_background);
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, RULER_H);
        nvgLineTo(vg, box.size.x, RULER_H);
        nvgStrokeWidth(vg, 1.0f);
        nvgStrokeColor(vg, line_bar);
        nvgStroke(vg);

        nvgScissor(vg, KEYS_W, 0, layout.grid.size.x, RULER_H);

        // Thin the bar numbers out as bars shrink, or they collide.
        float bar_width = STEPS_PER_BAR * pixels_per_step;
        int label_every = (bar_width >= 30.0f) ? 1 : ((bar_width >= 12.0f) ? 4 : 8);

        int first_bar = (int)std::floor(scroll_steps / STEPS_PER_BAR);
        int last_bar = (int)std::ceil((scroll_steps + layout.visibleSteps(pixels_per_step)) / STEPS_PER_BAR);

        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));

        if (font)
        {
            nvgFontSize(vg, 8.0f);
            nvgFontFaceId(vg, font->handle);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        }

        for (int bar = std::max(0, first_bar); bar <= last_bar; bar++)
        {
            float x = xOfStep((float)(bar * STEPS_PER_BAR));

            nvgBeginPath(vg);
            nvgMoveTo(vg, x, RULER_H - 5.0f);
            nvgLineTo(vg, x, RULER_H);
            nvgStrokeWidth(vg, 1.0f);
            nvgStrokeColor(vg, line_bar);
            nvgStroke(vg);

            if (font && (bar % label_every) == 0)
            {
                nvgFillColor(vg, ruler_text);
                nvgText(vg, x + 3.0f, 2.0f, std::to_string(bar + 1).c_str(), NULL);
            }
        }

        nvgRestore(vg);

        drawLoopMarker(vg);
    }

    void drawLoopMarker(NVGcontext *vg)
    {
        float x = xOfStep((float)loopSteps());
        if (x < KEYS_W - 6.0f || x > box.size.x + 6.0f) return;

        nvgSave(vg);
        nvgScissor(vg, KEYS_W, 0, layout.grid.size.x + VSB_W, box.size.y);

        nvgBeginPath(vg);
        nvgMoveTo(vg, x, 0);
        nvgLineTo(vg, x, box.size.y);
        nvgStrokeWidth(vg, 1.5f);
        nvgStrokeColor(vg, loop_flag);
        nvgStroke(vg);

        // The flag itself, pointing back over the region that plays.
        nvgBeginPath(vg);
        nvgMoveTo(vg, x, 1.0f);
        nvgLineTo(vg, x - 9.0f, 1.0f);
        nvgLineTo(vg, x, 9.0f);
        nvgClosePath(vg);
        nvgFillColor(vg, loop_flag);
        nvgFill(vg);

        nvgRestore(vg);
    }

    // The thumb's LENGTH shows how much of the 128-pitch range is on screen and its
    // POSITION shows where. Both derived, never stored.
    void drawScrollbar(NVGcontext *vg)
    {
        float x = layout.scrollbarX();

        nvgBeginPath(vg);
        nvgRect(vg, x, RULER_H, VSB_W, box.size.y - RULER_H);
        nvgFillColor(vg, screen_background);
        nvgFill(vg);

        rack::Rect track = scrollbarTrack();

        nvgBeginPath(vg);
        nvgRoundedRect(vg, track.pos.x, track.pos.y, track.size.x, track.size.y, track.size.x * 0.5f);
        nvgFillColor(vg, scrollbar_trough);
        nvgFill(vg);

        rack::Rect thumb = scrollbarThumb();

        nvgBeginPath(vg);
        nvgRoundedRect(vg, thumb.pos.x, thumb.pos.y, thumb.size.x, thumb.size.y, thumb.size.x * 0.5f);
        nvgFillColor(vg, scrollbar_thumb);
        nvgFill(vg);
    }

    rack::Rect scrollbarTrack() const
    {
        rack::Rect track;
        track.pos = Vec(layout.scrollbarX() + VSB_PAD, RULER_H + VSB_PAD);
        track.size = Vec(VSB_W - VSB_PAD * 2.0f, layout.grid.size.y - VSB_PAD * 2.0f);
        return track;
    }

    // The note being recorded right now: an outline growing from its snapped start
    // to the playhead, so what is about to be committed is visible while it is
    // still being played.
    void drawRecordingNotes(NVGcontext *vg)
    {
        if (!module || !module->rec_armed) return;

        nvgSave(vg);
        nvgScissor(vg, layout.grid.pos.x, layout.grid.pos.y, layout.grid.size.x, layout.grid.size.y);

        for (int channel = 0; channel < rack::engine::PORT_MAX_CHANNELS; channel++)
        {
            const PianoRoll::RecordVoice &voice = module->record_voices[channel];
            if (!voice.holding) continue;

            float y = yOfPitch(voice.pitch);
            if (y + ROW_H < layout.grid.pos.y || y > layout.grid.pos.y + layout.grid.size.y) continue;

            float x0 = xOfStep((float)voice.start_step);
            float x1 = module->playhead_position >= 0
                     ? xOfStep((float)(module->playhead_position + 1))
                     : x0 + pixels_per_step;

            float left = std::max(x0, layout.grid.pos.x);
            float width = std::max(NOTE_MIN_W, x1 - left);

            nvgBeginPath(vg);
            nvgRoundedRect(vg, left, y + 0.5f, width, ROW_H - NOTE_INSET_Y, NOTE_CORNER_R);
            nvgFillColor(vg, nvgRGBA(0xD2, 0x3B, 0x3B, 0x47));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRoundedRect(vg, left, y + 0.5f, width, ROW_H - NOTE_INSET_Y, NOTE_CORNER_R);
            nvgStrokeWidth(vg, 1.5f);
            nvgStrokeColor(vg, nvgRGBA(0xD2, 0x3B, 0x3B, 0xF2));
            nvgStroke(vg);
        }

        nvgRestore(vg);
    }

    void drawPlayhead(NVGcontext *vg)
    {
        if (!module || module->playhead_position < 0) return;

        float x = xOfStep((float)module->playhead_position);
        if (x < layout.grid.pos.x || x > layout.grid.pos.x + layout.grid.size.x) return;

        nvgSave(vg);
        nvgScissor(vg, layout.grid.pos.x, RULER_H, layout.grid.size.x, box.size.y - RULER_H);

        nvgBeginPath(vg);
        nvgMoveTo(vg, x, RULER_H);
        nvgLineTo(vg, x, RULER_H + layout.grid.size.y);
        nvgStrokeWidth(vg, 1.5f);
        nvgStrokeColor(vg, playhead);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    // The Follow toggle, in the corner above the keys column.
    void drawCorner(NVGcontext *vg)
    {
        float w = KEYS_W - 5.0f;
        float h = RULER_H - 5.0f;
        float x = 2.0f, y = 2.0f;

        NVGcolor ink = follow ? nvgRGB(0x3f, 0xb9, 0x50) : nvgRGB(0x5b, 0x66, 0x75);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, 3.0f);
        nvgFillColor(vg, follow ? nvgRGB(0x1d, 0x3a, 0x24) : nvgRGB(0x14, 0x18, 0x1d));
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, w, h, 3.0f);
        nvgStrokeWidth(vg, 1.0f);
        nvgStrokeColor(vg, follow ? nvgRGB(0x3f, 0xb9, 0x50) : nvgRGB(0x2b, 0x33, 0x3d));
        nvgStroke(vg);

        // Two chase arrows.
        float cx = x + w * 0.5f, cy = y + h * 0.5f;
        for (int i = 0; i < 2; i++)
        {
            float dx = (i == 0) ? -4.5f : 1.5f;
            nvgBeginPath(vg);
            nvgMoveTo(vg, cx + dx, cy - 3.5f);
            nvgLineTo(vg, cx + dx, cy + 3.5f);
            nvgLineTo(vg, cx + dx + 4.0f, cy);
            nvgClosePath(vg);
            nvgFillColor(vg, ink);
            nvgFill(vg);
        }
    }

    //
    // The key currently being auditioned, in the ACTIVE TRACK'S colour so it also
    // says which voice spoke.
    //
    // NOTE this is drawn on the UNMASKED path, not inside the grid scissor. The web
    // original drew its equivalent on a layer masked to the grid while the
    // highlight itself sits at x 0..KEYS_W — entirely outside that mask — so it has
    // never once rendered in that codebase.
    //
    void drawPreviewKey(NVGcontext *vg)
    {
        if (!module || !module->preview_active) return;

        float y = yOfPitch(module->preview_pitch);
        if (y < RULER_H || y > box.size.y) return;

        nvgBeginPath(vg);
        nvgRect(vg, 0, y, KEYS_W, ROW_H);
        nvgFillColor(vg, withAlpha(trackColors(module->active_track).fill, 0.75f));
        nvgFill(vg);
    }

    // The selection box and the marquee rectangle, both clipped to the grid.
    void drawOverlay(NVGcontext *vg)
    {
        nvgSave(vg);
        nvgScissor(vg, layout.grid.pos.x, layout.grid.pos.y, layout.grid.size.x, layout.grid.size.y);

        if (drag_mode == DRAG_MARQUEE)
        {
            // While marqueeing, the live rectangle stands in for the selection box.
            float x = std::min(drag_start_position.x, drag_position.x);
            float y = std::min(drag_start_position.y, drag_position.y);
            float w = std::fabs(drag_position.x - drag_start_position.x);
            float h = std::fabs(drag_position.y - drag_start_position.y);

            nvgBeginPath(vg);
            nvgRect(vg, x, y, w, h);
            nvgFillColor(vg, nvgRGBA(0x6B, 0xA6, 0xFF, 0x2E));
            nvgFill(vg);

            nvgBeginPath(vg);
            nvgRect(vg, x, y, w, h);
            nvgStrokeWidth(vg, 1.0f);
            nvgStrokeColor(vg, nvgRGBA(0x6B, 0xA6, 0xFF, 0xCC));
            nvgStroke(vg);
        }
        else
        {
            // The group box: one handle for moving the whole module->selection. It only
            // exists at two or more notes — a lone note is its own handle.
            rack::Rect bounds;
            if (selectionBounds(bounds))
            {
                const TrackColors &colors = trackColors(module ? module->active_track : 0);

                nvgBeginPath(vg);
                nvgRoundedRect(vg, bounds.pos.x, bounds.pos.y, bounds.size.x, bounds.size.y, 3.0f);
                nvgFillColor(vg, withAlpha(colors.selected_edge, 0.10f));
                nvgFill(vg);

                nvgBeginPath(vg);
                nvgRoundedRect(vg, bounds.pos.x, bounds.pos.y, bounds.size.x, bounds.size.y, 3.0f);
                nvgStrokeWidth(vg, 1.25f);
                nvgStrokeColor(vg, withAlpha(colors.selected_edge, 0.9f));
                nvgStroke(vg);
            }
        }

        nvgRestore(vg);
    }

    // ── Velocity lane ────────────────────────────────────────────────────────
    //
    // Overlay strip along the bottom edge. Collapsed it is a peek: miniature
    // bars, purely informational, and one click opens it. Open, it edits: one
    // bar per note start on the ACTIVE track, bar height = velocity — the same
    // track-scoping rule as every other edit surface here.

    void drawVelocityLane(NVGcontext *vg)
    {
        float top = velLaneTop();
        float height = velLaneHeight();
        bool open = velLaneOpen();

        // Near-opaque scrim: the covered pitch rows should read as covered, not
        // blend into the bars.
        nvgBeginPath(vg);
        nvgRect(vg, 0, top, box.size.x, height);
        nvgFillColor(vg, nvgRGBA(0x0c, 0x18, 0x18, open ? 0xEA : 0xD0));
        nvgFill(vg);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0, top + 0.5f);
        nvgLineTo(vg, box.size.x, top + 0.5f);
        nvgStrokeWidth(vg, 1.0f);
        nvgStrokeColor(vg, line_bar);
        nvgStroke(vg);

        // Chevron in the keys column: points up when the lane can expand, down
        // when it can collapse. Pure paths — NanoSVG-style, no font needed.
        {
            float cx = KEYS_W * 0.5f;
            float cy = top + (open ? 8.0f : height * 0.5f);
            float direction = open ? 1.0f : -1.0f;

            nvgBeginPath(vg);
            nvgMoveTo(vg, cx - 4.0f, cy - 2.0f * direction);
            nvgLineTo(vg, cx, cy + 2.0f * direction);
            nvgLineTo(vg, cx + 4.0f, cy - 2.0f * direction);
            nvgStrokeWidth(vg, 1.6f);
            nvgStrokeColor(vg, ruler_text);
            nvgStroke(vg);
        }

        // Label, top-right. Open only: the peek strip is too short for even a
        // 7px face to sit clear of the bars.
        if (open)
        {
            std::shared_ptr<Font> font = APP->window->loadFont(
                asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
            if (font)
            {
                nvgFontSize(vg, 7.0f);
                nvgFontFaceId(vg, font->handle);
                nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgFillColor(vg, ruler_text);
                nvgText(vg, box.size.x - 4.0f, top + 3.0f, "VELOCITY", NULL);
            }
        }

        // Bars, clipped to the grid's x range so they scroll and zoom with it.
        nvgSave(vg);
        nvgScissor(vg, layout.grid.pos.x, top, layout.grid.size.x, height);

        const std::vector<Note> &notes = module ? module->notes : previewNotes();
        int active = module ? module->active_track : 0;
        const TrackColors &colors = trackColors(active);

        // Scoped: unselected bars ghost right down, so the bars that CAN be
        // grabbed are the only ones with visual weight.
        bool scoped = velLaneScoped();

        float bar_w = velBarWidth();
        float area = height - VEL_LANE_PAD_TOP;
        float bottom = box.size.y;

        for (size_t i = 0; i < notes.size(); i++)
        {
            const Note &note = notes[i];
            if (note.track != active) continue;

            float x = xOfStep((float)note.start);
            if (x + bar_w < layout.grid.pos.x) continue;
            if (x > layout.grid.pos.x + layout.grid.size.x) continue;

            float bar_h = std::max(1.0f,
                area * sanitizeVelocity(note.velocity) / (float)MAX_VELOCITY);
            bool selected = module && module->selection.count(note.id);

            float alpha = open ? 0.95f : 0.65f;
            if (scoped && !selected) alpha = open ? 0.18f : 0.12f;

            nvgBeginPath(vg);
            nvgRect(vg, x, bottom - bar_h, bar_w, bar_h);
            nvgFillColor(vg, withAlpha(selected ? colors.selected_fill : colors.fill, alpha));
            nvgFill(vg);

            if (open && selected)
            {
                nvgBeginPath(vg);
                nvgRect(vg, x + 0.5f, bottom - bar_h + 0.5f, bar_w - 1.0f, bar_h - 1.0f);
                nvgStrokeWidth(vg, 1.0f);
                nvgStrokeColor(vg, withAlpha(colors.selected_edge, 0.9f));
                nvgStroke(vg);
            }
        }

        nvgRestore(vg);
    }

    // ── Undo ─────────────────────────────────────────────────────────────────
    //
    // Every edit is bracketed: beginEdit() before, endEdit() after. endEdit
    // compares the two states and pushes NOTHING if the edit was a no-op, so a
    // gesture that lands back where it started never pollutes the history.
    //
    // One action per completed gesture, never per drag frame.

    bool edit_open = false;
    std::vector<Note> edit_notes_before;
    std::set<NoteId> edit_selection_before;
    int edit_loop_before = DEFAULT_LOOP_STEPS;

    void beginEdit()
    {
        if (!module || edit_open) return;

        edit_open = true;
        edit_notes_before = module->notes;
        edit_selection_before = module->selection;
        edit_loop_before = module->loop_steps;
    }

    void endEdit(const std::string &name)
    {
        if (!module || !edit_open) return;
        edit_open = false;

        bool notes_changed = (edit_notes_before.size() != module->notes.size());
        if (!notes_changed)
        {
            for (size_t i = 0; i < module->notes.size(); i++)
            {
                if (edit_notes_before[i] != module->notes[i]) { notes_changed = true; break; }
            }
        }

        bool loop_changed = (edit_loop_before != module->loop_steps);
        if (!notes_changed && !loop_changed) return;   // no-op: push nothing

        // Publish the gesture's result to the audio thread. The drag paths
        // mutate notes/loop_steps per-frame with only recomputeTrackChannels(),
        // so without this the snapshot keeps playing the pre-drag pattern (and
        // the pre-drag loop length) until some unrelated edit republishes it.
        // Every completed gesture passes through here, so it cannot be
        // forgotten per-caller.
        module->patternChanged();

        NoteEditAction *action = new NoteEditAction;
        action->name = name;
        action->moduleId = module->id;
        action->old_notes = edit_notes_before;
        action->old_selection = edit_selection_before;
        action->old_loop = edit_loop_before;
        action->new_notes = module->notes;
        action->new_selection = module->selection;
        action->new_loop = module->loop_steps;

        APP->history->push(action);
    }

    void abandonEdit() { edit_open = false; }

    // ── Keyboard commands ────────────────────────────────────────────────────
    //
    // onHoverKey fires without focus, routed by pointer position — matching the
    // web original, where the commands work while the cursor is over the roll.
    // Consuming is SELECTIVE, key by key: a blanket consume would swallow Ctrl+Z
    // and break undo.

    void onHoverKey(const HoverKeyEvent &e) override
    {
        if (module && (e.action == GLFW_PRESS || e.action == GLFW_REPEAT))
        {
            // No select-all keystroke, deliberately: Rack claims plain Ctrl+A
            // before this widget sees it, and modified variants proved
            // unreliable too. The grid context menu carries the command.
            if (e.isKeyCommand(GLFW_KEY_C, RACK_MOD_CTRL)) { copySelection(); e.consume(this); return; }
            if (e.isKeyCommand(GLFW_KEY_V, RACK_MOD_CTRL)) { pasteClipboard(); e.consume(this); return; }

            if (e.isKeyCommand(GLFW_KEY_DELETE) || e.isKeyCommand(GLFW_KEY_BACKSPACE))
            {
                deleteAtCursor(e.pos);
                // ALWAYS consumed, even when nothing was deleted: otherwise a
                // Delete aimed at a note but landing on empty space falls through
                // to the rack and removes the module being edited.
                e.consume(this);
                return;
            }

            if (e.isKeyCommand(GLFW_KEY_ESCAPE))
            {
                // Only consume when there IS a selection, so a second Escape falls
                // through and closes/deselects at the rack level.
                if (!module->selection.empty()) { module->selection.clear(); e.consume(this); return; }
            }
        }

        OpaqueWidget::onHoverKey(e);
    }

    void selectAll()
    {
        module->selection.clear();
        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (note.track == module->active_track) module->selection.insert(note.id);
        }
    }

    void deleteSelection()
    {
        if (isLocked()) return;
        if (module->selection.empty()) return;
        beginEdit();

        std::vector<Note> kept;
        kept.reserve(module->notes.size());

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            if (!module->selection.count(module->notes[i].id)) kept.push_back(module->notes[i]);
        }

        module->notes.swap(kept);
        module->selection.clear();
        endEdit("delete notes");
    }

    // Clears the ACTIVE track regardless of selection. Other tracks are never
    // touched — same scoping rule as every other track-wide operation here.
    void deleteAllOnTrack()
    {
        if (isLocked()) return;
        beginEdit();

        std::vector<Note> kept;
        kept.reserve(module->notes.size());

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (note.track != module->active_track) kept.push_back(note);
            else module->selection.erase(note.id);
        }

        module->notes.swap(kept);
        endEdit("delete all on track");   // a no-op on an empty track pushes nothing
    }

    // The selection wins; only with nothing selected does Delete fall back to the
    // note under the pointer.
    void deleteAtCursor(Vec position)
    {
        if (isLocked()) return;
        if (!module->selection.empty()) { deleteSelection(); return; }
        if (zoneAt(position) != ZONE_GRID) return;

        int index = noteIndexAt(stepAtX(position.x), pitchAtY(position.y));
        if (index < 0) return;

        beginEdit();
        module->notes.erase(module->notes.begin() + index);
        endEdit("delete note");
    }

    void copySelection()
    {
        if (module->selection.empty()) return;

        int earliest = 0;
        bool first = true;

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (!module->selection.count(note.id)) continue;
            if (first) { earliest = note.start; first = false; }
            else earliest = std::min(earliest, note.start);
        }

        note_clipboard.clear();
        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (!module->selection.count(note.id)) continue;
            note_clipboard.push_back({note.pitch, note.start - earliest, note.length, note.velocity});
        }
    }

    // Paste lands the block just past the active track's last note, snapped up —
    // not at the cursor and not where it was copied from. Repeated pastes append.
    void pasteClipboard()
    {
        if (isLocked()) return;
        if (note_clipboard.empty()) return;
        beginEdit();

        int right_end = 0;
        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (note.track == module->active_track) right_end = std::max(right_end, note.end());
        }

        int snap = snapSteps();
        int start = ((right_end + snap - 1) / snap) * snap;

        module->selection.clear();
        for (size_t i = 0; i < note_clipboard.size(); i++)
        {
            const ClipboardNote &source = note_clipboard[i];

            NoteId id = module->addNote(source.pitch, start + source.start_offset,
                                        source.length, module->active_track, source.velocity);
            if (id == NOTE_ID_NONE) break;   // hit the note cap: partial paste

            module->selection.insert(id);
        }

        endEdit("paste notes");
    }

    // ── Grid context menu ────────────────────────────────────────────────────
    //
    // Target rule for both operations: the selection if there is one, otherwise
    // every note on the ACTIVE track. Never other tracks.

    std::vector<size_t> targetIndices() const
    {
        std::vector<size_t> targets;

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (!module->selection.empty()) { if (module->selection.count(note.id)) targets.push_back(i); }
            else if (note.track == module->active_track) targets.push_back(i);
        }
        return targets;
    }

    // Rotates note starts around the loop: a note pushed past the end reappears at
    // the start. That is what makes this different from dragging, which slides.
    void shiftNotes(int delta)
    {
        if (isLocked()) return;
        std::vector<size_t> targets = targetIndices();
        if (targets.empty()) return;

        beginEdit();
        int loop = loopSteps();

        for (size_t i = 0; i < targets.size(); i++)
        {
            Note &note = module->notes[targets[i]];
            note.start = ((note.start + delta) % loop + loop) % loop;
        }

        endEdit("shift notes");
    }

    // Snaps PITCH to the nearest degree of a scale. Timing is never touched.
    void quantizeToScale(int root, const std::vector<int> &degrees)
    {
        if (isLocked()) return;
        std::vector<size_t> targets = targetIndices();

        beginEdit();
        for (size_t i = 0; i < targets.size(); i++)
        {
            Note &note = module->notes[targets[i]];
            note.pitch = nearestDegree(note.pitch, root, degrees);
        }

        endEdit("quantize notes");
    }

    static bool inScale(int pitch, int root, const std::vector<int> &degrees)
    {
        int interval = ((pitch - root) % 12 + 12) % 12;
        for (size_t i = 0; i < degrees.size(); i++) if (degrees[i] == interval) return true;
        return false;
    }

    static int nearestDegree(int pitch, int root, const std::vector<int> &degrees)
    {
        if (inScale(pitch, root, degrees)) return pitch;

        // Search outward, DOWN before up, so ties resolve downward.
        for (int distance = 1; distance <= 6; distance++)
        {
            int lower = pitch - distance;
            int upper = pitch + distance;
            if (lower >= MIN_PITCH && inScale(lower, root, degrees)) return lower;
            if (upper <= MAX_PITCH && inScale(upper, root, degrees)) return upper;
        }
        return pitch;
    }

    // ── Zones ────────────────────────────────────────────────────────────────
    //
    // One widget covering keys + ruler + grid + scrollbar + corner, with an
    // internal dispatcher — not sibling widgets. The press-precedence chain has to
    // be one function, or "what does this click do" fragments across five places.

    enum Zone { ZONE_CORNER, ZONE_KEYS, ZONE_RULER, ZONE_GRID, ZONE_SCROLLBAR, ZONE_VEL_LANE };

    // ── Velocity lane geometry ───────────────────────────────────────────────
    //
    // The lane is an overlay along the bottom edge — it borrows screen space
    // from the grid without reflowing it, so nothing jumps when it opens.

    bool velLaneOpen() const { return module && module->velocity_lane_open; }
    float velLaneHeight() const { return velLaneOpen() ? VEL_LANE_OPEN_H : VEL_LANE_PEEK_H; }
    float velLaneTop() const { return box.size.y - velLaneHeight(); }

    // Widget-local y -> velocity, measured from the lane's bottom edge.
    int velocityAtY(float y) const
    {
        float area = velLaneHeight() - VEL_LANE_PAD_TOP;
        float fraction = (box.size.y - y) / std::max(1.0f, area);
        return rack::math::clamp((int)std::lround(fraction * MAX_VELOCITY),
                                 MIN_VELOCITY, MAX_VELOCITY);
    }

    // Bar width tracks the zoom so bars neither collide when zoomed out nor
    // look lost inside wide steps when zoomed in.
    float velBarWidth() const
    {
        return rack::math::clamp(pixels_per_step - 1.0f, VEL_LANE_BAR_MIN_W, VEL_LANE_BAR_MAX_W);
    }

    // Whether the lane is SCOPED to the selection: true while any selected note
    // lives on the active track. Scoped, the lane draws unselected marks as
    // ghosts and refuses to grab them — so editing a chord is "select it in the
    // grid, where the notes are stacked vertically and trivially clickable,
    // then edit in the lane without ambiguity". Overlapping bars stop being a
    // grabbing problem because the grid does the disambiguation.
    bool velLaneScoped() const
    {
        if (!module) return false;

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (note.track == module->active_track && module->selection.count(note.id))
                return true;
        }
        return false;
    }

    // The active-track note whose bar sits under x, nearest wins. Reverse
    // iteration matches noteIndexAt, so overlapping bars resolve to the same
    // note the grid would pick. While scoped, only selected notes are
    // candidates.
    int velBarIndexAt(float x) const
    {
        if (!module) return -1;

        bool scoped = velLaneScoped();
        float w = velBarWidth();
        int best = -1;
        float best_distance = 0.0f;

        for (int i = (int)module->notes.size() - 1; i >= 0; i--)
        {
            const Note &note = module->notes[i];
            if (note.track != module->active_track) continue;
            if (scoped && module->selection.count(note.id) == 0) continue;

            float center = xOfStep((float)note.start) + w * 0.5f;
            float distance = std::fabs(center - x);

            if (distance > w * 0.5f + VEL_LANE_HIT_SLOP) continue;
            if (best < 0 || distance < best_distance) { best = i; best_distance = distance; }
        }
        return best;
    }

    Zone zoneAt(Vec position) const
    {
        // The lane overlays everything beneath its top edge, so it wins first.
        if (position.y >= velLaneTop()) return ZONE_VEL_LANE;

        if (position.x < KEYS_W) return position.y < RULER_H ? ZONE_CORNER : ZONE_KEYS;
        if (position.x >= layout.scrollbarX()) return position.y < RULER_H ? ZONE_CORNER : ZONE_SCROLLBAR;
        return position.y < RULER_H ? ZONE_RULER : ZONE_GRID;
    }

    int snapSteps() const { return module ? std::max(1, module->snap_steps) : 1; }

    // ── Hit testing ──────────────────────────────────────────────────────────
    //
    // Two DIFFERENT questions, deliberately. noteIndexAt asks "is this musical
    // position inside a note", using the half-open span so a note's own end step is
    // not inside it. edgeGrabAt asks "is the POINTER near an edge", in pixels — and
    // that is the only reason the right edge is reachable at all.

    int noteIndexAt(float step, int pitch) const
    {
        if (!module) return -1;

        for (int i = (int)module->notes.size() - 1; i >= 0; i--)
        {
            const Note &note = module->notes[i];
            if (note.track != module->active_track) continue;
            if (note.pitch != pitch) continue;
            if (step >= note.start && step < note.end()) return i;
        }
        return -1;
    }

    struct EdgeGrab { int index; bool left_edge; };

    bool edgeGrabAt(Vec position, EdgeGrab &out) const
    {
        if (!module) return false;

        int pitch = pitchAtY(position.y);

        for (int i = (int)module->notes.size() - 1; i >= 0; i--)
        {
            const Note &note = module->notes[i];
            if (note.track != module->active_track) continue;
            if (note.pitch != pitch) continue;

            float x0 = xOfStep((float)note.start);
            float x1 = xOfStep((float)note.end());

            // Scale the hot zone down for short notes, or a fixed one swallows a
            // narrow note whole and it can never be moved.
            float zone = std::min(EDGE_GRAB_W, std::max(1.0f, (x1 - x0) / 3.0f));

            if (std::fabs(position.x - x1) <= zone) { out.index = i; out.left_edge = false; return true; }

            // The left edge needs a note wide enough that grabbing it cannot be
            // confused with grabbing the right one.
            if (std::fabs(position.x - x0) <= zone && (x1 - x0) > EDGE_GRAB_MIN_NOTE_W)
            {
                out.index = i; out.left_edge = true; return true;
            }
        }
        return false;
    }

    // Bounding box of the selection, only once there are two or more notes in it —
    // a lone note is its own handle.
    bool selectionBounds(rack::Rect &out) const
    {
        if (!module || module->selection.size() < 2) return false;

        bool any = false;
        int first = 0, last = 0, highest = 0, lowest = 0;

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (!module->selection.count(note.id)) continue;

            if (!any) { first = note.start; last = note.end(); highest = lowest = note.pitch; any = true; }
            else
            {
                first = std::min(first, note.start);
                last = std::max(last, note.end());
                highest = std::max(highest, note.pitch);
                lowest = std::min(lowest, note.pitch);
            }
        }
        if (!any) return false;

        out.pos = Vec(xOfStep((float)first) - SELECTION_BOX_PAD, yOfPitch(highest) - SELECTION_BOX_PAD);
        out.size = Vec(xOfStep((float)last) - out.pos.x + SELECTION_BOX_PAD,
                       yOfPitch(lowest) + ROW_H + SELECTION_BOX_PAD - out.pos.y);
        return true;
    }

    // ── Cursor ───────────────────────────────────────────────────────────────
    //
    // There is no Rack cursor API — GLFW directly. Cursors are created once and
    // cached; creating one per hover frame (as one other module in this repo does)
    // leaks a GLFW cursor every frame the pointer moves.

    GLFWcursor *cursor_arrow = NULL;
    GLFWcursor *cursor_hand = NULL;
    GLFWcursor *cursor_resize_ew = NULL;
    GLFWcursor *cursor_move = NULL;
    GLFWcursor *cursor_crosshair = NULL;
    GLFWcursor *current_cursor = NULL;
    bool cursors_created = false;

    void createCursors()
    {
        if (cursors_created) return;
        cursors_created = true;
        cursor_hand = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
        cursor_resize_ew = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
        cursor_move = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        cursor_crosshair = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    }

    void setCursor(GLFWcursor *cursor)
    {
        if (cursor == current_cursor) return;   // only touch GLFW on a real change
        current_cursor = cursor;
        glfwSetCursor(APP->window->win, cursor);
    }

    // ── Drag state ───────────────────────────────────────────────────────────

    enum DragMode
    {
        DRAG_NONE, DRAG_PAN_X, DRAG_SCROLLBAR, DRAG_LOOP,
        DRAG_CREATE, DRAG_MOVE, DRAG_RESIZE, DRAG_MARQUEE, DRAG_PREVIEW,
        DRAG_VELOCITY
    };

    struct OriginalNote { NoteId id; int start; int pitch; };

    DragMode drag_mode = DRAG_NONE;
    Vec drag_position;              // accumulated pointer position, widget-local
    Vec drag_start_position;
    float drag_scroll_origin = 0.0f;
    float drag_thumb_grab = 0.0f;
    NoteId drag_note = NOTE_ID_NONE;
    bool drag_left_edge = false;
    int drag_origin_start = 0;
    int drag_origin_length = 0;
    // The press position in FRACTIONAL steps. It must not be rounded here: the
    // move delta is measured against the live float position, so flooring this
    // would make a zero-movement click read as a fraction of a step and snap the
    // note to a neighbouring column.
    float drag_press_step = 0.0f;
    int drag_press_pitch = 0;
    bool drag_changed = false;
    NoteId drag_collapse_to = NOTE_ID_NONE;
    std::vector<OriginalNote> drag_originals;
    std::set<NoteId> marquee_base;

    // Velocity-lane drag state: the grabbed note and the pre-drag velocities of
    // every note the gesture may rewrite (one note, or the whole selection).
    struct OriginalVelocity { NoteId id; int velocity; };
    std::vector<OriginalVelocity> drag_vel_originals;
    NoteId drag_vel_note = NOTE_ID_NONE;
    int drag_vel_grabbed_original = DEFAULT_VELOCITY;

    int last_note_length = DEFAULT_NOTE_LENGTH;

    // Chase the playhead while the transport runs. Transient view state — never
    // persisted, never undone.
    bool follow = true;

    rack::Rect scrollbarThumb() const
    {
        rack::Rect track = scrollbarTrack();

        float visible_fraction = std::min(1.0f, (float)layout.rows / (float)PITCH_COUNT);
        float height = std::max(VSB_MIN_THUMB_H, track.size.y * visible_fraction);

        int lowest_top = clampTopPitch(layout.rows - 1);
        float span = (float)std::max(1, MAX_PITCH - lowest_top);
        float position = (float)(MAX_PITCH - rack::math::clamp(top_pitch, lowest_top, MAX_PITCH)) / span;

        rack::Rect thumb;
        thumb.pos = Vec(track.pos.x, track.pos.y + std::round(position * (track.size.y - height)));
        thumb.size = Vec(track.size.x, height);
        return thumb;
    }

    // ── Events ───────────────────────────────────────────────────────────────

    void onHover(const HoverEvent &e) override
    {
        createCursors();

        switch (zoneAt(e.pos))
        {
            case ZONE_CORNER:    setCursor(cursor_hand); break;
            case ZONE_KEYS:      setCursor(cursor_hand); break;
            case ZONE_SCROLLBAR: setCursor(cursor_hand); break;
            case ZONE_RULER:
                setCursor(loopHandleAt(e.pos.x) ? cursor_resize_ew : cursor_move);
                break;
            case ZONE_GRID:
            {
                rack::Rect box_bounds;
                EdgeGrab grab;

                // Mirrors the press chain below: box, then edge, then body, then
                // empty. It cannot see modifiers, so a shift-click still shows the
                // plain cursor.
                if (selectionBounds(box_bounds) && box_bounds.contains(e.pos)) setCursor(cursor_move);
                else if (edgeGrabAt(e.pos, grab)) setCursor(cursor_resize_ew);
                else if (noteIndexAt(stepAtX(e.pos.x), pitchAtY(e.pos.y)) >= 0) setCursor(cursor_hand);
                else setCursor(cursor_crosshair);
                break;
            }
            case ZONE_VEL_LANE:
                setCursor(cursor_hand);
                break;
        }

        OpaqueWidget::onHover(e);
    }

    void onLeave(const LeaveEvent &e) override
    {
        setCursor(NULL);
        if (drag_mode != DRAG_PREVIEW) releasePreview();
        OpaqueWidget::onLeave(e);
    }

    bool loopHandleAt(float x) const
    {
        return std::fabs(x - xOfStep((float)loopSteps())) <= LOOP_HANDLE_GRAB_W;
    }

    void onButton(const ButtonEvent &e) override
    {
        // Right-click over the GRID opens the note menu. Elsewhere it is left
        // alone, so the module's own context menu still opens over the chrome.
        if (e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS
            && module && zoneAt(e.pos) == ZONE_GRID)
        {
            openGridMenu();
            e.consume(this);
            return;
        }

        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS)
        {
            beginDrag(e.pos, e.mods);
            e.consume(this);   // required, or no drag events follow
            return;
        }

        OpaqueWidget::onButton(e);
    }

    void onDragStart(const DragStartEvent &e) override
    {
        drag_position = drag_start_position;
        OpaqueWidget::onDragStart(e);
    }

    void onDragMove(const DragMoveEvent &e) override
    {
        // DragMoveEvent carries only a delta, so absolute position is accumulated.
        // Dividing by the zoom keeps one screen pixel equal to one widget pixel at
        // any rack zoom — this is the idiom every other Voxglitch drag uses.
        drag_position = drag_position.plus(e.mouseDelta.div(getAbsoluteZoom()));
        updateDrag(drag_position);

        OpaqueWidget::onDragMove(e);
    }

    void onDragEnd(const DragEndEvent &e) override
    {
        endDrag();
        OpaqueWidget::onDragEnd(e);
    }

    //
    // Wheel. NOTE the sign: Rack zooms IN on a positive scrollDelta.y, the opposite
    // of the web original's convention. Porting its branch conditions literally
    // reverses every gesture in the editor.
    //
    void onHoverScroll(const HoverScrollEvent &e) override
    {
        // A locked editor does not claim the wheel at all: if the roll cannot be
        // edited there is no reason for it to intercept scrolling, and holding on
        // to it just fights anyone navigating past. Same rule as the Tracks canvas.
        if (isLocked()) return;

        // Let the wheel pass through to Rack when the rack is zoomed out.
        //
        // Without this the editor steals the wheel from anyone merely scrolling
        // past it, which is worse than useless at a zoom level where the grid is
        // too small to aim at. Same threshold as the Tracks canvas in
        // voxglitch_devices, so the two modules behave alike.
        if (getAbsoluteZoom() < RACK_ZOOM_FOR_WHEEL) return;

        float delta = e.scrollDelta.y;
        if (delta == 0.0f) { OpaqueWidget::onHoverScroll(e); return; }

        switch (zoneAt(e.pos))
        {
            case ZONE_GRID:
            {
                // Zoom anchored at the cursor: the step under the pointer stays
                // under the pointer.
                float step_at_cursor = stepAtX(e.pos.x);
                pixels_per_step = rack::math::clamp(
                    pixels_per_step * (delta > 0.0f ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR),
                    MIN_PPS, MAX_PPS);
                scroll_steps = std::max(0.0f, step_at_cursor - (e.pos.x - KEYS_W) / pixels_per_step);
                follow = false;   // deliberate navigation stops the chase
                break;
            }
            case ZONE_KEYS:
            case ZONE_SCROLLBAR:
                top_pitch = clampTopPitch(top_pitch + (delta > 0.0f ? WHEEL_PITCH_STEP : -WHEEL_PITCH_STEP));
                break;
            case ZONE_RULER:
                scroll_steps = std::max(0.0f, scroll_steps + (delta > 0.0f ? -WHEEL_STEP_SCROLL : WHEEL_STEP_SCROLL));
                follow = false;
                break;
            default:
                break;
        }

        // Always consume, or the rack scrolls behind the editor.
        e.consume(this);
    }

    void openGridMenu()
    {
        // Roots and scales come from the shared table in PianoRollGeometry.hpp —
        // the same one the scale lock uses, so the two can never disagree.

        struct Amount { const char *name; int steps; };
        static const std::vector<Amount> AMOUNTS = {
            {"1/16", 1}, {"1/8", 2}, {"1/4", 4}, {"1/2", 8},
            {"1 Bar", 16}, {"2 Bars", 32}, {"4 Bars", 64},
        };

        ui::Menu *menu = createMenu();

        // Name the scope up front, so it is clear what an item will touch.
        size_t count = module->selection.size();
        menu->addChild(createMenuLabel(count
            ? (std::to_string(count) + (count > 1 ? " notes selected" : " note selected"))
            : ("Track " + std::to_string(module->active_track + 1))));

        PianoRollEditorWidget *self = this;

        menu->addChild(createSubmenuItem(
            count ? "Quantize selection" : "Quantize track", "",
            [self](ui::Menu *root_menu) {
                for (int root = 0; root < 12; root++)
                {
                    root_menu->addChild(createSubmenuItem(SCALE_ROOT_NAMES[root], "",
                        [self, root](ui::Menu *scale_menu) {
                            const std::vector<ScaleDefinition> &scales = scaleDefinitions();
                            for (size_t i = 0; i < scales.size(); i++)
                            {
                                std::vector<int> degrees = scales[i].degrees;
                                scale_menu->addChild(createMenuItem(scales[i].name, "",
                                    [self, root, degrees]() { self->quantizeToScale(root, degrees); }));
                            }
                        }));
                }
            }));

        menu->addChild(new MenuSeparator);

        menu->addChild(createSubmenuItem("Shift left", "", [self](ui::Menu *shift_menu) {
            for (size_t i = 0; i < AMOUNTS.size(); i++)
            {
                int steps = AMOUNTS[i].steps;
                shift_menu->addChild(createMenuItem(AMOUNTS[i].name, "",
                    [self, steps]() { self->shiftNotes(-steps); }));
            }
        }));

        menu->addChild(createSubmenuItem("Shift right", "", [self](ui::Menu *shift_menu) {
            for (size_t i = 0; i < AMOUNTS.size(); i++)
            {
                int steps = AMOUNTS[i].steps;
                shift_menu->addChild(createMenuItem(AMOUNTS[i].name, "",
                    [self, steps]() { self->shiftNotes(steps); }));
            }
        }));

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Select all on track", "",
            [self]() { self->selectAll(); }));
        menu->addChild(createMenuItem("Delete selection", "Delete",
            [self]() { self->deleteSelection(); }));
        menu->addChild(createMenuItem("Delete all on track", "",
            [self]() { self->deleteAllOnTrack(); }));

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Import MIDI...", "", [self]() { self->importMidiFile(); }));
        menu->addChild(createMenuItem("Export MIDI...", "",
            [self]() { self->exportMidiFile(); }, module->notes.empty()));
    }

    // ── MIDI files ───────────────────────────────────────────────────────────

    void exportMidiFile()
    {
        if (!module || module->notes.empty()) return;

        osdialog_filters *filters = osdialog_filters_parse("MIDI:mid,midi");
        char *path = osdialog_file(OSDIALOG_SAVE, NULL, "pianoroll.mid", filters);
        osdialog_filters_free(filters);
        if (!path) return;

        std::string filename = path;
        std::free(path);

        // Add the extension if the dialog did not.
        if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".mid")
        {
            if (filename.find('.') == std::string::npos) filename += ".mid";
        }

        // 120 BPM is written as the file's tempo. The module has no tempo of its
        // own — it is clocked externally — so this is a label for the DAW, not a
        // claim about how the pattern was played.
        if (!exportMidi(filename, module->notes, PianoRoll::TRACKS, 120.0f))
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, "Could not write the MIDI file.");
        }
    }

    //
    // Import REPLACES the whole roll, as one undo step. The loop grows to cover the
    // imported material, and the view frames it.
    //
    void importMidiFile()
    {
        if (!module || isLocked()) return;

        osdialog_filters *filters = osdialog_filters_parse("MIDI:mid,midi");
        char *path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, filters);
        osdialog_filters_free(filters);
        if (!path) return;

        std::string filename = path;
        std::free(path);

        std::vector<Note> imported;
        if (!importMidi(filename, PianoRoll::TRACKS, imported))
        {
            osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK,
                             "Could not read any notes from that MIDI file.");
            return;
        }

        beginEdit();

        module->notes.clear();
        module->selection.clear();

        int highest_pitch = MIN_PITCH;
        int last_end = 0;

        for (size_t i = 0; i < imported.size(); i++)
        {
            if ((int)module->notes.size() >= PianoRoll::MAX_NOTES) break;

            Note note = imported[i];
            note.id = module->id_minter.mint();
            module->notes.push_back(note);

            highest_pitch = std::max(highest_pitch, note.pitch);
            last_end = std::max(last_end, note.end());
        }

        // Grow the loop to a whole number of bars covering everything imported, so
        // the material actually plays rather than landing past the loop end.
        int bars = (last_end + STEPS_PER_BAR - 1) / STEPS_PER_BAR;
        module->loop_steps = std::max(STEPS_PER_BAR, bars * STEPS_PER_BAR);

        endEdit("import MIDI");

        // Frame the result, and stop Follow from immediately snapping away from it.
        scroll_steps = 0.0f;
        top_pitch = clampTopPitch(highest_pitch + 2);
        follow = false;
    }

    // ── The press-precedence chain ───────────────────────────────────────────
    //
    // Order matters and is asymmetric: shift outranks the selection box, the box
    // outranks note edges (which is HOW it captures), edges outrank note bodies,
    // and painting is the fallback when a press means nothing else.

    void beginDrag(Vec position, int mods)
    {
        drag_mode = DRAG_NONE;
        drag_start_position = position;
        drag_changed = false;
        drag_collapse_to = NOTE_ID_NONE;
        drag_originals.clear();

        if (!module) return;

        bool shift = (mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;

        switch (zoneAt(position))
        {
            case ZONE_RULER:
                if (loopHandleAt(position.x) && !isLocked()) { beginEdit(); drag_mode = DRAG_LOOP; }
                else { drag_mode = DRAG_PAN_X; drag_scroll_origin = scroll_steps; }
                return;

            case ZONE_SCROLLBAR:
            {
                rack::Rect thumb = scrollbarThumb();
                if (position.y >= thumb.pos.y && position.y < thumb.pos.y + thumb.size.y)
                {
                    drag_thumb_grab = position.y - thumb.pos.y;
                }
                else
                {
                    // Click the trough: centre the thumb on the click, then keep
                    // dragging from there.
                    drag_thumb_grab = thumb.size.y * 0.5f;
                    scrollbarSeek(position.y - scrollbarTrack().pos.y - drag_thumb_grab);
                }
                drag_mode = DRAG_SCROLLBAR;
                return;
            }

            case ZONE_GRID:
                // Selection still works while locked; only the edit branches are
                // suppressed inside beginGridDrag.
                beginGridDrag(position, shift);
                return;

            case ZONE_KEYS:
                startPreview(rack::math::clamp(pitchAtY(position.y), MIN_PITCH, MAX_PITCH));
                drag_mode = DRAG_PREVIEW;
                return;

            case ZONE_CORNER:
                setFollow(!follow);
                return;

            case ZONE_VEL_LANE:
                beginVelLaneDrag(position);
                return;

            default:
                return;
        }
    }

    void beginVelLaneDrag(Vec position)
    {
        if (!module) return;

        // Collapsed, the whole strip is one big "open me" button; open, the keys
        // column beneath the lane is the collapse control.
        if (!velLaneOpen() || position.x < KEYS_W)
        {
            module->velocity_lane_open = !module->velocity_lane_open;
            return;
        }

        if (isLocked()) return;

        int index = velBarIndexAt(position.x);
        if (index < 0) return;

        const Note &grabbed = module->notes[index];

        beginEdit();
        drag_vel_note = grabbed.id;
        drag_vel_grabbed_original = grabbed.velocity;
        drag_vel_originals.clear();

        // Dragging a bar that belongs to a multi-note selection scales the whole
        // selection proportionally; dragging any other bar edits that one note
        // and leaves the selection alone.
        bool group = module->selection.count(grabbed.id) && module->selection.size() > 1;

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (note.track != module->active_track) continue;
            if (group ? module->selection.count(note.id) == 0 : note.id != grabbed.id) continue;

            drag_vel_originals.push_back({ note.id, note.velocity });
        }

        drag_mode = DRAG_VELOCITY;
        applyVelocityDrag(position);   // the press itself already sets a value
    }

    void applyVelocityDrag(Vec position)
    {
        if (!module) return;

        int target = velocityAtY(position.y);

        for (size_t i = 0; i < drag_vel_originals.size(); i++)
        {
            const OriginalVelocity &original = drag_vel_originals[i];

            Note *note = module->findNote(original.id);
            if (!note) continue;

            int value;
            if (original.id == drag_vel_note)
            {
                value = target;
            }
            else if (drag_vel_grabbed_original > 0)
            {
                // Proportional: dragging the grabbed bar to half scales every
                // selected note to half, preserving the selection's dynamics.
                value = (int)std::lround(original.velocity
                            * (double)target / drag_vel_grabbed_original);
            }
            else
            {
                // A grabbed original of 0 has no ratio to scale by; fall back to
                // moving the group by the same absolute amount.
                value = original.velocity + target;
            }

            note->velocity = sanitizeVelocity(value);
        }

        // Republish the playback snapshot so the audio thread hears the new
        // velocities while the drag is still in progress.
        module->patternChanged();
    }

    void beginGridDrag(Vec position, bool shift)
    {
        float step = stepAtX(position.x);
        int pitch = rack::math::clamp(pitchAtY(position.y), MIN_PITCH, MAX_PITCH);
        int index = noteIndexAt(step, pitch);
        int snap = snapSteps();

        rack::Rect box_bounds;
        bool has_box = selectionBounds(box_bounds);
        bool in_box = has_box && box_bounds.contains(position);

        drag_press_step = step;
        drag_press_pitch = pitch;

        // A. Shift edits selection membership, anywhere — inside the box or out.
        if (shift)
        {
            if (index >= 0)
            {
                NoteId id = module->notes[index].id;
                if (module->selection.count(id)) module->selection.erase(id); else module->selection.insert(id);
                return;
            }
            marquee_base = module->selection;
            drag_mode = DRAG_MARQUEE;
            return;
        }

        // B. The selection box captures presses inside it — which is why note edges
        //    are not tested here, and a note in a group cannot be resized.
        if (in_box)
        {
            if (index >= 0 && !module->selection.count(module->notes[index].id))
            {
                // An unselected note that merely sits within the box bounds is a
                // fresh pick, not part of the group.
                selectOnly(module->notes[index].id);
                beginMove(module->notes[index].id);
                return;
            }
            beginMoveSelection(index >= 0 ? module->notes[index].id : NOTE_ID_NONE);
            return;
        }

        // C. Edge grab. Deliberately does NOT change the module->selection.
        EdgeGrab grab;
        if (edgeGrabAt(position, grab))
        {
            const Note &note = module->notes[grab.index];
            drag_mode = DRAG_RESIZE;
            drag_note = note.id;
            drag_left_edge = grab.left_edge;
            drag_origin_start = note.start;
            drag_origin_length = note.length;
            return;
        }

        // D. A note body: select only it, and arm a move.
        if (index >= 0)
        {
            selectOnly(module->notes[index].id);
            beginMove(module->notes[index].id);
            return;
        }

        // E. Empty space with a box up: the click is spent dismissing the module->selection.
        //    A second click paints.
        if (has_box)
        {
            module->selection.clear();
            return;
        }

        // F. Paint.
        if (isLocked()) return;
        beginEdit();
        module->selection.clear();
        int start = std::max(0, (int)std::floor(step / snap) * snap);
        int length = std::max(snap, last_note_length);

        NoteId id = module->addNote(restrictPitch(pitch), start, length, module->active_track);
        if (id == NOTE_ID_NONE) return;   // at the note cap: silently refuse

        module->selection.insert(id);
        drag_mode = DRAG_CREATE;
        drag_note = id;
    }

    void startPreview(int pitch)
    {
        if (!module) return;

        module->preview_track = module->active_track;
        module->preview_pitch = pitch;
        module->preview_active = true;
    }

    // Drop any held preview. Called on release, on leaving the widget, and on
    // teardown — a gate raised straight on the module would otherwise outlive the
    // gesture that raised it and drone with nothing to stop it.
    void releasePreview()
    {
        if (module) module->preview_active = false;
    }

    ~PianoRollEditorWidget() { releasePreview(); }

    //
    // Keep the playhead in view while the transport runs. Only DELIBERATE
    // horizontal navigation switches this off (see the pan and zoom paths) — note
    // editing never does, so painting while the view moves stays possible.
    //
    void setFollow(bool on)
    {
        follow = on;

        if (follow && module && module->playhead_position >= 0)
        {
            float visible = layout.visibleSteps(pixels_per_step);
            scroll_steps = std::max(0.0f, module->playhead_position - std::floor(visible * FOLLOW_ANCHOR_FRACTION));
        }
    }

    void updateFollow()
    {
        if (!follow || !module || drag_mode != DRAG_NONE) return;
        if (!module->isPlaying() || module->playhead_position < 0) return;

        float x = xOfStep((float)module->playhead_position);

        // Page when the playhead runs off the right edge, and snap back if it is
        // off the left — a loop wrap, or a pattern longer than the window.
        if (x > layout.grid.pos.x + layout.grid.size.x - 4.0f || x < layout.grid.pos.x)
        {
            scroll_steps = std::max(0.0f, (float)(module->playhead_position - FOLLOW_PAGE_LEAD_STEPS));
        }
    }

    void selectOnly(NoteId id) { module->selection.clear(); module->selection.insert(id); }

    void beginMove(NoteId id)
    {
        if (isLocked()) return;   // selected, but not draggable
        beginEdit();
        drag_mode = DRAG_MOVE;
        drag_collapse_to = id;
        snapshotSelection();
    }

    void beginMoveSelection(NoteId clicked)
    {
        if (isLocked()) return;
        beginEdit();
        drag_mode = DRAG_MOVE;
        drag_collapse_to = clicked;
        snapshotSelection();
    }

    void snapshotSelection()
    {
        drag_originals.clear();
        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (module->selection.count(note.id)) drag_originals.push_back({note.id, note.start, note.pitch});
        }
    }

    void scrollbarSeek(float y_in_track)
    {
        rack::Rect track = scrollbarTrack();
        rack::Rect thumb = scrollbarThumb();

        float travel = std::max(1.0f, track.size.y - thumb.size.y);
        float fraction = rack::math::clamp(y_in_track / travel, 0.0f, 1.0f);
        int lowest_top = clampTopPitch(layout.rows - 1);

        top_pitch = clampTopPitch((int)std::round(MAX_PITCH - fraction * (MAX_PITCH - lowest_top)));
    }

    void updateDrag(Vec position)
    {
        if (!module) return;
        int snap = snapSteps();

        switch (drag_mode)
        {
            case DRAG_VELOCITY:
                applyVelocityDrag(position);
                break;

            case DRAG_PAN_X:
                scroll_steps = std::max(0.0f, drag_scroll_origin - (position.x - drag_start_position.x) / pixels_per_step);
                follow = false;
                break;

            case DRAG_SCROLLBAR:
                scrollbarSeek(position.y - scrollbarTrack().pos.y - drag_thumb_grab);
                break;

            case DRAG_LOOP:
            {
                // The loop marker is bar-locked regardless of the snap setting.
                int bars = (int)std::round(stepAtX(position.x) / STEPS_PER_BAR);
                module->loop_steps = std::max(STEPS_PER_BAR, bars * STEPS_PER_BAR);
                module->recomputeTrackChannels();
                break;
            }

            case DRAG_CREATE:
            {
                Note *note = module->findNote(drag_note);
                if (!note) break;

                note->pitch = restrictPitch(rack::math::clamp(pitchAtY(position.y), MIN_PITCH, MAX_PITCH));

                // Length only tracks after real horizontal travel, or a pixel of
                // click jitter would collapse the note to one snap unit.
                if (std::fabs(position.x - drag_start_position.x) > CREATE_DRAG_THRESHOLD)
                {
                    float end = std::ceil(std::max(stepAtX(position.x), note->start + 0.01f) / snap) * snap;
                    note->length = rack::math::clamp((int)end - note->start, snap, PianoRoll::MAX_NOTE_LENGTH);
                }
                module->recomputeTrackChannels();
                break;
            }

            case DRAG_MOVE:
            {
                // The DISPLACEMENT is snapped, not the destination — so moving never
                // quantises a note, it preserves its offset from the grid.
                int delta_steps = (int)std::round((stepAtX(position.x) - drag_press_step) / snap) * snap;
                int delta_pitch = rack::math::clamp(pitchAtY(position.y), MIN_PITCH, MAX_PITCH) - drag_press_pitch;

                for (size_t i = 0; i < drag_originals.size(); i++)
                {
                    Note *note = module->findNote(drag_originals[i].id);
                    if (!note) continue;
                    note->start = std::max(0, drag_originals[i].start + delta_steps);
                    note->pitch = restrictPitch(
                        rack::math::clamp(drag_originals[i].pitch + delta_pitch, MIN_PITCH, MAX_PITCH));
                }

                // A jitter that rounds to zero is still a CLICK — that is what makes
                // collapse-on-click work and what keeps no-op moves out of undo.
                drag_changed = (delta_steps != 0 || delta_pitch != 0);
                module->recomputeTrackChannels();
                break;
            }

            case DRAG_RESIZE:
            {
                Note *note = module->findNote(drag_note);
                if (!note) break;

                if (!drag_left_edge)
                {
                    int end = (int)std::round(stepAtX(position.x) / snap) * snap;
                    note->length = rack::math::clamp(end - note->start, snap, PianoRoll::MAX_NOTE_LENGTH);
                }
                else
                {
                    // The far end is pinned to where it was when the drag started.
                    int end = drag_origin_start + drag_origin_length;
                    int start = rack::math::clamp((int)std::floor(stepAtX(position.x) / snap) * snap, 0, end - snap);
                    note->start = start;
                    note->length = end - start;
                }
                drag_changed = true;
                module->recomputeTrackChannels();
                break;
            }

            case DRAG_PREVIEW:
            {
                // Glissando: retarget the held note without dropping the gate, so a
                // slide reads as one gesture rather than a burst of retriggers.
                int pitch = rack::math::clamp(pitchAtY(position.y), MIN_PITCH, MAX_PITCH);
                if (pitch != module->preview_pitch) module->preview_pitch = pitch;
                break;
            }

            case DRAG_MARQUEE:
                updateMarquee(position);
                break;

            default:
                break;
        }
    }

    void updateMarquee(Vec position)
    {
        float step_lo = std::min(stepAtX(drag_start_position.x), stepAtX(position.x));
        float step_hi = std::max(stepAtX(drag_start_position.x), stepAtX(position.x));
        int pitch_hi = std::max(pitchAtY(drag_start_position.y), pitchAtY(position.y));
        int pitch_lo = std::min(pitchAtY(drag_start_position.y), pitchAtY(position.y));

        // Additive: the base set is restored each frame, so a marquee can never
        // REMOVE anything from the selection.
        module->selection = marquee_base;

        for (size_t i = 0; i < module->notes.size(); i++)
        {
            const Note &note = module->notes[i];
            if (note.track != module->active_track) continue;
            if (note.pitch < pitch_lo || note.pitch > pitch_hi) continue;

            // Touch, not containment — a note only partly crossed is caught.
            if (note.start < step_hi && note.end() > step_lo) module->selection.insert(note.id);
        }
    }

    void endDrag()
    {
        if (module)
        {
            if (drag_mode == DRAG_CREATE)
            {
                Note *note = module->findNote(drag_note);
                if (note) last_note_length = note->length;
                endEdit("paint note");
            }
            else if (drag_mode == DRAG_MOVE)
            {
                if (!drag_changed && drag_collapse_to != NOTE_ID_NONE)
                {
                    // A move that never travelled is a click: collapse a group
                    // selection down to the one note that was clicked. Nothing
                    // moved, so endEdit pushes nothing.
                    selectOnly(drag_collapse_to);
                }
                endEdit("move notes");
            }
            else if (drag_mode == DRAG_PREVIEW) releasePreview();
            else if (drag_mode == DRAG_RESIZE) endEdit("resize note");
            else if (drag_mode == DRAG_LOOP)   endEdit("set loop length");
            else if (drag_mode == DRAG_VELOCITY) endEdit("edit velocity");
            else abandonEdit();
        }

        drag_mode = DRAG_NONE;
        drag_note = NOTE_ID_NONE;
        drag_originals.clear();
        drag_vel_originals.clear();
        drag_vel_note = NOTE_ID_NONE;
    }
};
