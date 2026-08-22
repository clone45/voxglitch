//
// PianoRollGeometry — every measurement in the Piano Roll's editor, in one place.
//
// Nothing here is a magic number at a call site. The rule is:
//
//   * PRIMITIVES are the handful of numbers a designer actually chooses
//     (row height, keys column width, the zoom range).
//   * DERIVED values are computed from them and are never written out by hand.
//   * The editor's OUTER RECT is not here at all — it is read from the panel SVG
//     at construction (PanelHelper::findNamedRect("editor_area")), so moving the
//     rect in Inkscape moves the widget with it.
//
// See docs/modules/piano-roll/rack-port-design.md section 7.
//

namespace piano_roll
{
    // ── Editor chrome ────────────────────────────────────────────────────────
    static constexpr float KEYS_W = 36.0f;   // piano keys column, left
    static constexpr float VSB_W = 13.0f;    // vertical scrollbar column, right
    static constexpr float RULER_H = 16.0f;  // bar ruler, top
    static constexpr float ROW_H = 9.0f;     // one semitone lane

    // A note rectangle is inset within its lane so adjacent notes read as separate
    // blocks rather than one continuous bar.
    static constexpr float NOTE_INSET_Y = 1.5f;
    static constexpr float NOTE_MIN_W = 2.0f;      // never let a note vanish when zoomed out
    static constexpr float NOTE_CORNER_R = 2.0f;
    static constexpr float NOTE_GRIP_MIN_W = 14.0f; // show the resize grip past this width

    // ── Time axis ────────────────────────────────────────────────────────────
    static constexpr int STEPS_PER_BEAT = 4;
    static constexpr int STEPS_PER_BAR = 16;

    static constexpr float DEFAULT_PPS = 14.0f;  // pixels per step at default zoom
    static constexpr float MIN_PPS = 2.0f;
    static constexpr float MAX_PPS = 48.0f;
    static constexpr float ZOOM_IN_FACTOR = 1.25f;
    static constexpr float ZOOM_OUT_FACTOR = 0.8f;  // mutually inverse with the above

    // Below this zoom, individual step lines are suppressed and only beat and bar
    // lines are drawn.
    static constexpr float STEP_LINE_MIN_PPS = 7.0f;

    // ── Pitch axis ───────────────────────────────────────────────────────────
    static constexpr int MIN_PITCH = 0;
    static constexpr int MAX_PITCH = 127;
    static constexpr int PITCH_COUNT = MAX_PITCH - MIN_PITCH + 1;
    static constexpr int DEFAULT_TOP_PITCH = 79;   // G5 at the top row

    static constexpr int WHEEL_PITCH_STEP = 2;     // semitones per wheel notch
    static constexpr int WHEEL_STEP_SCROLL = 8;    // steps per wheel notch over the ruler

    // Below this rack zoom the editor lets the mouse wheel pass through to Rack
    // rather than claiming it, so scrolling past the module does not fight the
    // user. Matches the Tracks canvas.
    static constexpr float RACK_ZOOM_FOR_WHEEL = 0.95f;

    // ── Interaction ──────────────────────────────────────────────────────────
    static constexpr float EDGE_GRAB_W = 4.0f;     // resize hot zone, pixels either side
    static constexpr float EDGE_GRAB_MIN_NOTE_W = EDGE_GRAB_W * 2.5f;  // left edge needs this width
    static constexpr float CREATE_DRAG_THRESHOLD = 4.0f;  // px of travel before length tracks
    static constexpr float LOOP_HANDLE_GRAB_W = 5.0f;
    static constexpr float SELECTION_BOX_PAD = 2.0f;

    // ── Scrollbar ────────────────────────────────────────────────────────────
    static constexpr float VSB_PAD = 2.0f;         // trough inset within the column
    static constexpr float VSB_MIN_THUMB_H = 22.0f;

    // ── Follow (playhead chase) ──────────────────────────────────────────────
    static constexpr float FOLLOW_ANCHOR_FRACTION = 0.25f;  // where the playhead parks on enable
    static constexpr int FOLLOW_PAGE_LEAD_STEPS = 2;        // ...and after an auto page

    // ── Defaults ─────────────────────────────────────────────────────────────
    static constexpr int DEFAULT_LOOP_STEPS = 4 * STEPS_PER_BAR;  // 64
    static constexpr int DEFAULT_NOTE_LENGTH = STEPS_PER_BEAT;    // 4 steps = one beat

    //
    // Layout — everything derived from the editor rect the panel gives us.
    //
    // The grid height is deliberately SNAPPED DOWN to a whole number of rows. The
    // web original left a remainder band at the bottom that still hit-tested as
    // grid, so a click there returned a pitch below the last drawn row and painted
    // a note into dead space. Snapping makes that impossible for any panel size,
    // which is what lets the SVG be edited freely.
    //
    struct Layout
    {
        rack::Rect editor;   // the whole editor surface, panel coordinates
        rack::Rect grid;     // the note grid, editor-local coordinates
        int rows = 0;        // visible pitch lanes

        Layout() {}

        explicit Layout(rack::Rect editor_rect)
        {
            editor = editor_rect;

            float available_h = editor.size.y - RULER_H;
            rows = (int)std::floor(available_h / ROW_H);
            if (rows < 1) rows = 1;

            grid.pos = Vec(KEYS_W, RULER_H);
            grid.size = Vec(editor.size.x - KEYS_W - VSB_W, rows * ROW_H);
        }

        bool isValid() const { return grid.size.x > 0.0f && grid.size.y > 0.0f; }

        // Editor-local x of the scrollbar column.
        float scrollbarX() const { return editor.size.x - VSB_W; }

        // How many steps fit across the grid at a given zoom.
        float visibleSteps(float pixels_per_step) const
        {
            return grid.size.x / pixels_per_step;
        }
    };

} // namespace piano_roll
