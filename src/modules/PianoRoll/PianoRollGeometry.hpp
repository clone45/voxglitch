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

    // ── Velocity ─────────────────────────────────────────────────────────────
    //
    // Stored in the MIDI domain, 0..127, rather than as a voltage or a 0..1
    // float. Import and export are first-class here, and the MIDI domain is the
    // only one that round-trips a file byte-for-byte.
    //
    // The default is 80 because that is the fixed value every note was exported
    // with before velocity existed. A patch saved by an older build loads with
    // every note at 80 and exports byte-identically to what it did before.
    static constexpr int MIN_VELOCITY = 0;
    static constexpr int MAX_VELOCITY = 127;
    static constexpr int DEFAULT_VELOCITY = 80;

    // Velocity leaves the module as 0..10 V, matching Rack's own MIDI-CV.
    static constexpr float VELOCITY_OUTPUT_VOLTS = 10.0f;

    // ── Velocity lane ────────────────────────────────────────────────────────
    //
    // An OVERLAY along the bottom of the editor, never a reflow: expanding it
    // covers the lowest pitch rows rather than resizing the grid, so the note
    // under the cursor never jumps when it opens.
    //
    // Collapsed it is a peek strip: always visible, drawing miniature bars, and
    // clicking anywhere on it expands the lane. That strip is the discoverability
    // story — a lane reachable only through a context menu never gets found.
    static constexpr float VEL_LANE_PEEK_H = 10.0f;
    static constexpr float VEL_LANE_OPEN_H = 64.0f;
    static constexpr float VEL_LANE_PAD_TOP = 3.0f;    // headroom above a full bar
    static constexpr float VEL_LANE_BAR_MAX_W = 7.0f;  // bars never wider than this
    static constexpr float VEL_LANE_BAR_MIN_W = 3.0f;  // ...or thinner than this
    static constexpr float VEL_LANE_HIT_SLOP = 3.0f;   // extra pixels either side of a bar

    // ── Scales ───────────────────────────────────────────────────────────────
    //
    // One table serving both the grid menu's one-shot "quantize to scale" and
    // the persistent scale lock, so the two features can never disagree about
    // what "Dorian" means. The abbreviation is what fits on the control-bar
    // button next to a root name.
    struct ScaleDefinition
    {
        const char *name;
        const char *abbrev;
        std::vector<int> degrees;
    };

    static const char *const SCALE_ROOT_NAMES[12] =
        {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    inline const std::vector<ScaleDefinition> &scaleDefinitions()
    {
        static const std::vector<ScaleDefinition> definitions = {
            {"Major",            "Maj",  {0,2,4,5,7,9,11}},
            {"Minor",            "Min",  {0,2,3,5,7,8,10}},
            {"Harmonic Minor",   "HMin", {0,2,3,5,7,8,11}},
            {"Melodic Minor",    "MMin", {0,2,3,5,7,9,11}},
            {"Dorian",           "Dor",  {0,2,3,5,7,9,10}},
            {"Phrygian",         "Phr",  {0,1,3,5,7,8,10}},
            {"Lydian",           "Lyd",  {0,2,4,6,7,9,11}},
            {"Mixolydian",       "Mix",  {0,2,4,5,7,9,10}},
            {"Locrian",          "Loc",  {0,1,3,5,6,8,10}},
            {"Major Pentatonic", "MajP", {0,2,4,7,9}},
            {"Minor Pentatonic", "MinP", {0,3,5,7,10}},
            {"Blues",            "Blu",  {0,3,5,6,7,10}},
        };
        return definitions;
    }

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
