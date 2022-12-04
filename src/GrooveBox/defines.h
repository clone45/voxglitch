namespace groove_box
{
    const int NUMBER_OF_STEPS = 16;
    const int NUMBER_OF_TRACKS = 8;
    const int NUMBER_OF_MEMORY_SLOTS = 16;
    const int NUMBER_OF_PARAMETER_LOCKS = 16;
    const int NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS = 8;
    const int NUMBER_OF_RATCHET_PATTERNS = 16;

    const float MODULE_WIDTH = 223.52000 * 2.952756;
    const float MODULE_HEIGHT = 128.50000 * 2.952756;

    const float maximum_release_time = 4.0;

    // WARNING!  Do not reorder the elements in the Parameters array, otherwise 
    // it will break people's patches.
    //
    // If you wish to reorder the parameters, change the "parameter_slots" array
    // below in this file.
    //
    // TODO: It might be worth thinking about making this more object-oriented

    enum Parameters
    {
        VOLUME,
        PAN,
        PITCH,
        RATCHET,
        SAMPLE_START,
        SAMPLE_END,
        PROBABILITY,
        LOOP,
        REVERSE,
        ATTACK,
        RELEASE,
        DELAY_MIX,
        DELAY_LENGTH,
        DELAY_FEEDBACK,
        FILTER_CUTOFF,
        FILTER_RESONANCE
    };

    // These must be in the same order as the Parameters enum above
    const std::array<float, NUMBER_OF_PARAMETER_LOCKS> default_parameter_values = {
        0.5, // volume
        0.5, // pan
        0.5, // pitch
        0.0, // ratchet
        0.0, // sample_start
        1.0, // sample_end
        1.0, // probability
        0.0, // loop
        0.0, // reverse
        0.0, // AR attack
        1.0, // AR release
        0.0, // delay mix
        0.5, // delay length
        0.5, // delay feedback
        1.0, // filter cutoff
        0.0  // filter resonance
    };    

    const std::string PARAMETER_LOCK_NAMES[NUMBER_OF_PARAMETER_LOCKS] = {
        "Volume",
        "Pan",
        "Pitch",
        "Ratchet",
        "Sample Start",
        "Sample End",
        "Probability",
        "Loop",
        "Reverse",
        "Attack",
        "Release",
        "Delay Mix",
        "Delay Length",
        "Delay Feedback",
        "Filter Cutoff",
        "Filter Resonance"
    };

    // This is an array that maps the slot to the slot's associated parameter
    // It's purpose is to allow for the parameters to be rearranged on the front
    // panel without changing the parameter ids, which would break people's patches
    //
    // These probably won't be in the same order as the Parameter enums, and
    // that's perfectly OK.

    int parameter_slots[NUMBER_OF_PARAMETER_LOCKS] = {
        VOLUME,
        PAN,
        PITCH,
        RATCHET,
        PROBABILITY,
        DELAY_MIX,
        DELAY_LENGTH,
        DELAY_FEEDBACK,
        ATTACK,
        RELEASE,
        LOOP,
        REVERSE,
        SAMPLE_START,
        SAMPLE_END,
        FILTER_CUTOFF,
        FILTER_RESONANCE
    };

    const std::string PLACEHOLDER_TRACK_NAMES[NUMBER_OF_TRACKS] = {
        "kick_drum.wav",
        "snare.wav",
        "hihat_open.wav",
        "hihat_closed.wav",
        "low_tom.wav",
        "jiggly_puff.wav",
        "",
        ""};

    const std::string sample_position_snap_names[NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS] = {
        "none",
        "4",
        "8",
        "16",
        "32",
        "64",
        "128",
        "256"};

    const unsigned int sample_position_snap_values[NUMBER_OF_SAMPLE_POSITION_SNAP_OPTIONS] = {
        0,
        4,
        8,
        16,
        32,
        64,
        128,
        256};

    const bool ratchet_patterns[NUMBER_OF_RATCHET_PATTERNS][7] = {
        {0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 1, 1, 1},
        {0, 0, 1, 0, 0, 1, 0},
        {0, 1, 1, 0, 1, 1, 0},
        {0, 1, 0, 1, 0, 1, 0},
        {0, 1, 0, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0, 1},
        {1, 1, 0, 0, 0, 1, 1},
        {1, 1, 0, 1, 1, 0, 1},
        {1, 1, 1, 0, 1, 0, 0},
        {1, 1, 1, 0, 1, 0, 1},
        {1, 1, 1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0},
        {1, 1, 1, 0, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1}};

    const float button_positions_y = mm2px(88.5);

    const float button_positions[16][2] = {
        {mm2px(9.941), button_positions_y},
        {mm2px(23.52), button_positions_y},
        {mm2px(37.10), button_positions_y},
        {mm2px(50.69), button_positions_y},
        {mm2px(64.27), button_positions_y},
        {mm2px(77.85), button_positions_y},
        {mm2px(91.43), button_positions_y},
        {mm2px(105.02), button_positions_y},
        {mm2px(118.60), button_positions_y},
        {mm2px(132.18), button_positions_y},
        {mm2px(145.76), button_positions_y},
        {mm2px(159.35), button_positions_y},
        {mm2px(172.93), button_positions_y},
        {mm2px(186.51), button_positions_y},
        {mm2px(200.09), button_positions_y},
        {mm2px(213.67), button_positions_y}};

}

/* previous version:
enum Parameters
{
VOLUME,
PAN,
PITCH,
RATCHET,
SAMPLE_START,
SAMPLE_END,
PROBABILITY,
LOOP,
REVERSE,
ATTACK,
RELEASE,
DELAY_MIX,
DELAY_LENGTH,
DELAY_FEEDBACK,
FILTER_CUTOFF,
FILTER_RESONANCE
};

const float default_volume = 0.5;
const float default_pan = 0.5;
const float default_pitch = 0.5;
const float default_ratchet = 0.0;
const float default_sample_start = 0.0;
const float default_sample_end = 1.0;
const float default_probability = 1.0;
const float default_loop = 0.0;
const bool default_reverse = false;
const float default_attack = 0.0;
const float default_release = 1.0;
const float default_delay_mix = 0.0;
const float default_delay_length = 0.5;
const float default_delay_feedback = 0.5;
const float default_filter_cutoff = 1.0;
const float default_filter_resonance = 0.0;
*/