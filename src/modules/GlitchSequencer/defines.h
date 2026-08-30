#define MAX_SEQUENCE_LENGTH 64
#define SEQUENCER_ROWS 16
#define SEQUENCER_COLUMNS 21
#define NUMBER_OF_TRIGGER_GROUPS 8
#define NUMBER_OF_MEMORY_SLOTS 16

// Which layer the screen is editing.  -1 = watch (nothing selected),
// 0 = the seed, 1..8 = trigger group (value - 1).
#define EDIT_LAYER_WATCH -1
#define EDIT_LAYER_SEED 0

// Screen-internal layout, in px.  The screen rectangle itself comes from the
// panel SVG (findNamedRect("screen")); these constants carve it up.  The
// layout mirrors the vxsynth web port: a tab column on the left (SEED + 8
// trigger groups), the cell grid, a memory column on the right (16 slots),
// and a position bar along the bottom.
#define SCREEN_PADDING 4.0
#define TAB_COLUMN_WIDTH 46.0
#define SCREEN_GAP 8.0
#define MEMORY_COLUMN_WIDTH 44.0
#define MEMORY_LABEL_HEIGHT 11.0
#define MEMORY_BUTTON_GAP 3.0
#define POSITION_BAR_HEIGHT 6.0

// One hue per trigger group, shared by the screen tabs and the gate-output
// labels so a jack is visually tied to its layer.
static const unsigned int GS_GROUP_COLORS[NUMBER_OF_TRIGGER_GROUPS] = {
  0xff5a5a, 0xff9f40, 0xffe04a, 0x7bd957, 0x4ad6c4, 0x5aa9ff, 0xb07bff, 0xff6fce
};

static inline NVGcolor gs_rgb(unsigned int hex)
{
  return nvgRGB((hex >> 16) & 0xff, (hex >> 8) & 0xff, hex & 0xff);
}
