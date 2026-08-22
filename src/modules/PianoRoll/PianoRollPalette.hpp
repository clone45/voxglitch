//
// PianoRollPalette — the eight track colours.
//
// Designed against the editor's DARK screen (lanes #182a2a / #122222), not the
// light "paper" grid of the web original, because Voxglitch displays are dark
// regardless of panel theme. See rack-port-design.md section 12.1.
//
// STRUCTURE: eight hues spread right around the wheel — red, amber, yellow,
// green, teal, azure, violet, magenta — with a slight lightness zig-zag so
// neighbouring tracks differ on two axes rather than one.
//
// MEASURED (CIEDE2000, Vienot-Brettel-Mollon 1999 dichromat simulation):
//
//                        normal   deutan   protan
//   active track          21.07     4.54     1.65
//   dimmed (a=0.50)       15.50     1.92     0.46
//
// Normal vision separates these far better than a two-hue design (21 against 15).
// The trade is that a full hue circle is NOT colour-vision-deficient safe: red and
// green collapse for dichromats, so violet/magenta and yellow/green become hard to
// tell apart. Track NUMBERS on the selector squares are the non-colour channel
// that keeps the module usable in that case, which is why they are drawn there.
//
// SELECTION uses an ADAPTIVE RIM rather than "make it brighter": at the top of the
// lightness range there is no headroom left to brighten into. Each track's
// sel_edge is chosen for contrast against its own fill — a light rim on the dark
// tracks, a dark rim on the light ones — giving every track 21-30 dE of rim
// contrast. The fill only shifts slightly, so track identity still reads through
// the selection state.
//
// Regenerate with scripts/piano_roll_palette.py.
//

namespace piano_roll
{
    struct TrackColors
    {
        NVGcolor fill;
        NVGcolor edge;
        NVGcolor selected_fill;
        NVGcolor selected_edge;
    };

    // Alpha applied to notes on tracks other than the active one. Higher than the
    // web original's 0.30 because a dark ground compresses a dimmed colour toward
    // the lane much faster than a light ground does.
    static constexpr float INACTIVE_TRACK_ALPHA = 0.50f;

    // Alpha for notes that lie past the loop end and can never play.
    static constexpr float OUT_OF_LOOP_ALPHA = 0.55f;

    static const TrackColors TRACK_COLORS[8] = {
        { nvgRGB(0xFE, 0x76, 0x72), nvgRGB(0xDF, 0x1B, 0x37),
          nvgRGB(0xFF, 0x9F, 0x98), nvgRGB(0xFF, 0xE9, 0xE7) },  // 1  red
        { nvgRGB(0xFF, 0xA8, 0x4B), nvgRGB(0xCB, 0x77, 0x02),
          nvgRGB(0xFF, 0xCA, 0x9B), nvgRGB(0x92, 0x54, 0x01) },  // 2  amber
        { nvgRGB(0xBB, 0xAE, 0x04), nvgRGB(0x88, 0x7E, 0x01),
          nvgRGB(0xD6, 0xC7, 0x01), nvgRGB(0x5D, 0x56, 0x00) },  // 3  yellow
        { nvgRGB(0x03, 0xCC, 0x37), nvgRGB(0x03, 0x96, 0x26),
          nvgRGB(0x02, 0xE9, 0x3F), nvgRGB(0x00, 0x69, 0x17) },  // 4  green
        { nvgRGB(0x0B, 0xBA, 0xAB), nvgRGB(0x05, 0x86, 0x7B),
          nvgRGB(0x0E, 0xD5, 0xC4), nvgRGB(0xC5, 0xFF, 0xF6) },  // 5  teal
        { nvgRGB(0x08, 0xB8, 0xF2), nvgRGB(0x02, 0x86, 0xB1),
          nvgRGB(0x6F, 0xCF, 0xFF), nvgRGB(0x05, 0x5C, 0x7A) },  // 6  azure
        { nvgRGB(0x7D, 0x94, 0xFE), nvgRGB(0x00, 0x66, 0xE7),
          nvgRGB(0xA3, 0xAE, 0xFF), nvgRGB(0xE6, 0xE6, 0xFE) },  // 7  violet
        { nvgRGB(0xFF, 0x77, 0xF3), nvgRGB(0xCB, 0x3F, 0xC2),
          nvgRGB(0xFF, 0xA6, 0xF4), nvgRGB(0x8C, 0x2A, 0x86) },  // 8  magenta
    };

    inline const TrackColors &trackColors(int track)
    {
        int index = ((track % 8) + 8) % 8;
        return TRACK_COLORS[index];
    }

    inline NVGcolor withAlpha(NVGcolor color, float alpha)
    {
        color.a = alpha;
        return color;
    }

} // namespace piano_roll
