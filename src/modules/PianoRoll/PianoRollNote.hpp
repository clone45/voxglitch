//
// PianoRollNote — the note event, and its identity.
//
// A note is an EVENT, not a cell in a grid: { pitch, start step, length steps,
// track }, where one step is a 16th note. That is what makes the pattern length
// effectively unbounded.
//
// IDENTITY. Every note carries an opaque id, minted once at creation and frozen
// for the note's whole life. Nothing is ever identified by array index: the web
// original indexed into the note array, which is why its selection had to be
// thrown away on every external change and why deletion had to compact and clear
// together. Ids remove that whole class of bug — a selection is a set of ids, and
// it survives reordering, deletion and undo.
//
// The id is a random 53-bit integer rather than a UUID string: one uint64_t
// compare in a 60 Hz redraw path instead of a string hash, and it matches Rack's
// own Module::id convention. 53 bits keeps it exactly representable as a double,
// so it round-trips through any JSON reader without loss.
//
// See docs/modules/piano-roll/rack-port-design.md section 9.
//

namespace piano_roll
{
    typedef uint64_t NoteId;

    // Reserved: means "no note". Never minted.
    static const NoteId NOTE_ID_NONE = 0;

    static const uint64_t NOTE_ID_MASK = 0x1FFFFFFFFFFFFFull;  // 2^53 - 1

    struct Note
    {
        NoteId id = NOTE_ID_NONE;
        int pitch = 60;    // MIDI note, 0..127 (C4 = 60 = 0 V)
        int start = 0;     // step offset from the beginning of the pattern
        int length = 1;    // steps, >= 1
        int track = 0;     // 0..TRACKS-1

        Note() {}

        Note(NoteId note_id, int note_pitch, int note_start, int note_length, int note_track)
            : id(note_id), pitch(note_pitch), start(note_start), length(note_length), track(note_track) {}

        // The half-open span [start, start + length). A note ending at step k and a
        // note starting at step k are NOT simultaneous — that is what makes
        // back-to-back notes retrigger instead of merging.
        int end() const { return start + length; }

        bool coversStep(int step) const { return step >= start && step < end(); }
    };

    //
    // Mints note ids.
    //
    // Seeded from three independent sources because no single one is trustworthy
    // here: std::random_device is a fixed-seed stub on some MinGW builds (which
    // would make every instance mint the identical sequence), the module id is
    // stable but small and predictable, and the clock is coarse. Mixing all three
    // means a bad std::random_device cannot collapse the sequence on its own.
    //
    // Random rather than a counter, specifically because Rack's module copy/paste
    // round-trips through toJson/fromJson: a counter would be duplicated verbatim
    // into the pasted instance, and every subsequent note would collide with one
    // in the original.
    //
    struct NoteIdMinter
    {
        rack::random::Xoroshiro128Plus rng;

        void seed(int64_t module_id)
        {
            std::random_device device;

            uint64_t entropy = ((uint64_t)device() << 32) ^ (uint64_t)device();
            uint64_t clock = (uint64_t)(rack::system::getUnixTime() * 1000.0);

            rng.seed(entropy ^ (uint64_t)module_id,
                     clock ^ (0x9E3779B97F4A7C15ull + (uint64_t)module_id));
        }

        NoteId mint()
        {
            if (!rng.isSeeded()) seed(0);

            NoteId id = NOTE_ID_NONE;
            while (id == NOTE_ID_NONE) id = (NoteId)(rng() & NOTE_ID_MASK);
            return id;
        }
    };

    inline bool operator==(const Note &a, const Note &b)
    {
        return a.id == b.id && a.pitch == b.pitch && a.start == b.start
            && a.length == b.length && a.track == b.track;
    }
    inline bool operator!=(const Note &a, const Note &b) { return !(a == b); }

    // Range checks used on load. A hand-edited or corrupt patch must not be able to
    // put the module into a state the rest of the code does not expect, and the
    // response is to SKIP the note rather than clamp it silently or throw — an
    // exception out of dataFromJson would take the host down.
    inline bool isValidNote(const Note &note, int track_count, int max_length)
    {
        if (note.pitch < MIN_PITCH || note.pitch > MAX_PITCH) return false;
        if (note.start < 0) return false;
        if (note.length < 1 || note.length > max_length) return false;
        if (note.track < 0 || note.track >= track_count) return false;
        return true;
    }

} // namespace piano_roll
