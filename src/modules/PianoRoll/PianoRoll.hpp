//
// Piano Roll — a note-EVENT sequencer with a piano-roll editor.
//
// Port of the vxsynth Piano Roll module. See:
//   docs/modules/piano-roll/rack-port-design.md          <- Rack-specific design
//   docs/modules/piano-roll/vxsynth-feature-inventory.md
//   docs/modules/piano-roll/vxsynth-grid-navigation.md
//   docs/modules/piano-roll/vxsynth-keys-column-interactions.md
//   docs/modules/piano-roll/vxsynth-note-editing-interactions.md
//
// Notes are EVENTS { pitch, start step, length steps, track }, not a fixed step
// grid, so the pattern length is effectively unbounded. One step = a 16th note.
//
// EIGHT tracks share one grid and one transport. Each track drives a POLYPHONIC
// V/OCT + GATE pair, so overlapping notes on a track are chords rather than the
// mono last-note-wins voice the web original used.
//
// Clock is EXTERNAL ONLY, and one pulse is always one step (a 16th). There is
// deliberately no clock division or multiplication: any other ratio would have to
// PREDICT the intermediate note onsets from the previous interval, which for a note
// sequencer is audible as rushed or dragged 16ths. See rack-port-design.md 6.1.
//

using namespace piano_roll;

struct PianoRoll : Module
{
    static const int TRACKS = 8;

    // Raised from vxsynth's 512, which was chosen for 4 tracks and would be only 64
    // notes per track across 8. 2048 gives 256 per track, and costs ~64 KB per undo
    // snapshot at ~32 bytes a note.
    static const int MAX_NOTES = 2048;

    // The packed-double format the web original used is gone (Rack serializes to
    // JSON), but its field ranges remain useful as validation limits.
    static const int MAX_NOTE_LENGTH = 65535;

    enum ParamIds
    {
        NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK_INPUT,
        RESET_INPUT,
        REC_VOCT_INPUT,
        REC_GATE_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        ENUMS(VOCT_OUTPUTS, TRACKS),
        ENUMS(GATE_OUTPUTS, TRACKS),
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    // ── Pattern ──────────────────────────────────────────────────────────────
    std::vector<Note> notes;
    NoteIdMinter id_minter;

    int loop_steps = DEFAULT_LOOP_STEPS;
    int active_track = 0;

    // Editing grid, in steps. 1 = a 16th, 16 = a bar. "Off" resolves to 1 rather
    // than to free placement: note positions are integers throughout, so one step
    // is the finest resolution the model has.
    int snap_steps = 1;

    // Selected notes, by ID rather than index, so a selection survives deletion,
    // reordering and undo. UI state, but module-owned so an undo action can put it
    // back — undoing a Delete should return the selection, not just the notes.
    // Deliberately NOT persisted.
    std::set<NoteId> selection;

    // Polyphony channel count per track, derived from note content (see
    // channelsForTrack) and published by process(). Always in [1, 16].
    //
    // Written by the UI thread on edit, read by the audio thread every sample. A
    // uint8_t store is atomic on every platform Rack targets, so this particular
    // field is safe as-is; the NOTE LIST is not, and will need the double-buffer
    // treatment once process() starts reading it for playback.
    uint8_t track_channels[TRACKS];

    // ── Playback snapshot (UI thread -> audio thread) ────────────────────────
    //
    // `notes` belongs to the UI thread and is mutated freely while the user drags.
    // process() must never read it. Instead the UI publishes a SNAPSHOT into the
    // buffer that is not currently live, then flips an atomic index; process()
    // reads whichever buffer the index names.
    //
    // Two buffers are enough because there is exactly one writer, it writes only to
    // the buffer the reader is NOT using, and the reader finishes a read within one
    // process() call. A torn read would need the writer to flip twice inside that
    // window — the writer runs at UI rate, the reader takes well under a
    // microsecond. (A shared_ptr swap would be the obvious alternative and is
    // wrong: atomic shared_ptr is not lock-free before C++20, and whichever side
    // drops the last reference runs the deleter, which could be free() on the
    // audio thread.)
    struct PlaybackSnapshot
    {
        std::vector<Note> notes;
        int loop_steps = DEFAULT_LOOP_STEPS;
    };

    PlaybackSnapshot snapshots[2];
    std::atomic<int> live_snapshot;

    // ── Voices ───────────────────────────────────────────────────────────────
    struct Voice
    {
        bool active = false;
        int steps_left = 0;
        float voct = 0.0f;
        float retrigger_timer = 0.0f;   // seconds of forced gate-low remaining
    };

    Voice voices[TRACKS][rack::engine::PORT_MAX_CHANNELS];

    // ── Transport ────────────────────────────────────────────────────────────
    dsp::SchmittTrigger clock_trigger;
    dsp::SchmittTrigger reset_trigger;
    dsp::TTimer<float> reset_ignore_timer;
    bool ignoring_clock = false;

    int position = -1;                  // -1 so the first clock plays step 0
    long clock_sample_counter = 0;      // samples since the last ACCEPTED edge
    float step_samples = 0.0f;          // measured step duration, 0 = unknown

    // Read by the editor for the playhead. Plain int: a single aligned store read
    // by the UI is fine, and a torn value would only misdraw one frame.
    int playhead_position = -1;

    // ── Recording ────────────────────────────────────────────────────────────
    //
    // Arm REC and any patched pitch+gate pair records into the roll, hard-quantized
    // to the step grid. Overdub by nature: playback keeps running underneath.
    //
    // The capture is produced on the AUDIO thread but `notes` belongs to the UI
    // thread, so completed notes travel up through a small ring with an atomic
    // write index, drained by the editor each frame. A ring rather than a single
    // slot is what makes fast playing safe: several note-offs can land inside one
    // UI frame and all of them survive.
    bool rec_armed = false;

    // Freezes note editing so a finished pattern cannot be disturbed by a stray
    // click. Auditioning and selection stay live, and the view can still be panned
    // and scrolled by dragging — but the MOUSE WHEEL is released entirely, so it
    // zooms the rack instead of the roll. Recording is an edit, so it is blocked
    // too. Matches the Tracks module's "Lock Editor".
    bool locked = false;

    struct RecordVoice
    {
        bool holding = false;
        int pitch = 60;
        int start_step = 0;
        float held_samples = 0.0f;
    };

    RecordVoice record_voices[rack::engine::PORT_MAX_CHANNELS];
    dsp::SchmittTrigger record_gate_triggers[rack::engine::PORT_MAX_CHANNELS];

    struct CapturedNote { int pitch; int start; int length; int track; };

    static const int CAPTURE_RING = 32;
    CapturedNote capture_ring[CAPTURE_RING];
    std::atomic<unsigned int> capture_write;
    unsigned int capture_read = 0;      // UI thread only

    float step_phase = 0.0f;            // samples since the last step boundary

    // ── Keys-column preview ──────────────────────────────────────────────────
    //
    // Clicking a key auditions that pitch on the ACTIVE track. The override is
    // applied to the OUTPUTS ONLY, after the voice loop — the sequencer keeps
    // running underneath, so releasing the key hands the voice straight back
    // mid-phrase instead of resetting it.
    //
    // It shares channel 0 rather than reserving a channel of its own, so the
    // derived channel count never changes on a mouse-down. The cost is that an
    // audition collides with whatever is sounding on channel 0.
    bool preview_active = false;
    int preview_pitch = 60;
    int preview_track = 0;

    // Seconds since the last step advance. Used by the editor to decide whether the
    // transport is running, without needing to know what is clocking it.
    float play_timer = 0.0f;
    bool isPlaying() const { return play_timer > 0.0f; }

    static constexpr float RETRIGGER_SECONDS = 0.001f;
    static constexpr float RESET_IGNORE_SECONDS = 0.001f;

    PianoRoll()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(CLOCK_INPUT, "Clock (one pulse = one 16th step)");
        configInput(RESET_INPUT, "Reset");
        configInput(REC_VOCT_INPUT, "Record V/OCT (polyphonic)");
        configInput(REC_GATE_INPUT, "Record gate (polyphonic)");

        for (int track = 0; track < TRACKS; track++)
        {
            std::string number = std::to_string(track + 1);
            configOutput(VOCT_OUTPUTS + track, "Track " + number + " V/OCT");
            configOutput(GATE_OUTPUTS + track, "Track " + number + " gate");

            track_channels[track] = 1;
        }

        live_snapshot.store(0);
        capture_write.store(0);
        notesChanged();

        id_minter.seed(id);
    }

    // ── Note list ────────────────────────────────────────────────────────────

    Note *findNote(NoteId note_id)
    {
        for (size_t i = 0; i < notes.size(); i++)
        {
            if (notes[i].id == note_id) return &notes[i];
        }
        return NULL;
    }

    // Adds a note, minting its identity. Returns NOTE_ID_NONE if the pattern is
    // full — callers must handle that rather than assume success.
    NoteId addNote(int pitch, int start, int length, int track)
    {
        if ((int)notes.size() >= MAX_NOTES) return NOTE_ID_NONE;

        Note note(id_minter.mint(), pitch, start, length, track);
        notes.push_back(note);

        notesChanged();
        return note.id;
    }

    //
    // THE chokepoint for wholesale note-list replacement. Undo, redo and any bulk
    // edit route through here, so the snapshot republish and the derived channel
    // counts can never be forgotten by one caller.
    //
    void setNotes(const std::vector<Note> &new_notes, const std::set<NoteId> &new_selection, int new_loop_steps)
    {
        notes = new_notes;
        selection = new_selection;
        loop_steps = std::max(1, new_loop_steps);
        notesChanged();
    }

    void clearNotes()
    {
        notes.clear();
        notesChanged();
    }

    // ── Polyphony ────────────────────────────────────────────────────────────

    //
    // How many output channels this track needs: the maximum number of notes
    // sounding simultaneously anywhere in the pattern.
    //
    // The sweep is CIRCULAR, not linear, because a note sustains straight through
    // the loop wrap — position wraps to 0 but a sounding note's countdown does not
    // reset, so a note at step 60 of length 8 in a 64-step loop is still sounding
    // at steps 0..3 of the next pass. A linear sweep under-counts, and
    // under-counting means notes are silently dropped at playback.
    //
    int channelsForTrack(int track) const
    {
        const int loop = std::max(1, loop_steps);

        std::vector<int> delta(loop + 1, 0);

        for (size_t i = 0; i < notes.size(); i++)
        {
            const Note &note = notes[i];

            if (note.track != track) continue;
            if (note.length <= 0) continue;

            // Notes at or past the loop end never play: the step counter only ever
            // takes values in [0, loop), so their start is never matched.
            if (note.start < 0 || note.start >= loop) continue;

            // A note longer than the loop re-triggers itself once per pass rather
            // than stacking, so it can never need more than one voice.
            const int length = std::min(note.length, loop);
            const int start = note.start;
            const int end = start + length;

            if (end <= loop)
            {
                delta[start]++;
                delta[end]--;
            }
            else
            {
                delta[start]++;              // head: start .. loop-1
                delta[0]++;                  // wrapped tail: 0 .. (end-loop)-1
                delta[end - loop]--;
            }
        }

        int running = 0;
        int peak = 0;

        for (int step = 0; step < loop; step++)
        {
            running += delta[step];
            if (running > peak) peak = running;
        }

        // MANDATORY, not defensive: Port::setChannels() takes a uint8_t and does not
        // clamp, and Port::setVoltage() has no bounds check, so an unclamped count
        // above 16 writes past the end of the port's voltage array.
        return rack::math::clamp(peak, 1, rack::engine::PORT_MAX_CHANNELS);
    }

    void recomputeTrackChannels()
    {
        for (int track = 0; track < TRACKS; track++)
        {
            track_channels[track] = (uint8_t)channelsForTrack(track);
        }
    }

    // Call after ANY edit to the note list or the loop length. Recomputes the
    // derived channel counts and republishes the snapshot the audio thread reads.
    void notesChanged()
    {
        recomputeTrackChannels();

        int next = 1 - live_snapshot.load();
        snapshots[next].notes = notes;
        snapshots[next].loop_steps = std::max(1, loop_steps);
        live_snapshot.store(next);
    }

    // ── Audio ────────────────────────────────────────────────────────────────

    // Silence every voice on every track. Used by reset and by anything that
    // invalidates the grid a sounding note was timed against.
    void releaseAllVoices()
    {
        for (int track = 0; track < TRACKS; track++)
        {
            for (int channel = 0; channel < rack::engine::PORT_MAX_CHANNELS; channel++)
            {
                voices[track][channel].active = false;
                voices[track][channel].steps_left = 0;
                voices[track][channel].retrigger_timer = 0.0f;
            }
        }
    }

    //
    // Advance one step.
    //
    // ORDER MATTERS: every sounding voice is aged FIRST, then notes starting at the
    // new position are assigned voices. A note of length L spans steps
    // start .. start+L-1, so its gate falls exactly as the counter passes L
    // boundaries — and a note that ends where the next begins still retriggers,
    // instead of merging into one long gate for a downstream envelope.
    //
    void advanceStep(const PlaybackSnapshot &snapshot)
    {
        const int loop = std::max(1, snapshot.loop_steps);

        position++;
        if (position >= loop) position = 0;

        // Remember what was sounding BEFORE aging: a voice freed this step can be
        // reused this same step, and reuse is exactly what needs a retrigger gap.
        bool was_active[TRACKS][rack::engine::PORT_MAX_CHANNELS];

        for (int track = 0; track < TRACKS; track++)
        {
            for (int channel = 0; channel < rack::engine::PORT_MAX_CHANNELS; channel++)
            {
                Voice &voice = voices[track][channel];
                was_active[track][channel] = voice.active;

                if (voice.active && --voice.steps_left <= 0) voice.active = false;
            }
        }

        for (size_t i = 0; i < snapshot.notes.size(); i++)
        {
            const Note &note = snapshot.notes[i];

            if (note.start != position) continue;
            if (note.track < 0 || note.track >= TRACKS) continue;
            if (note.length <= 0) continue;

            int channels = track_channels[note.track];
            int channel = -1;

            // First free channel. The channel count is derived from the maximum
            // simultaneous overlap, so under normal editing a free one always
            // exists and no voice stealing is needed.
            for (int c = 0; c < channels; c++)
            {
                if (!voices[note.track][c].active) { channel = c; break; }
            }
            if (channel < 0) continue;   // only reachable if the count is stale

            Voice &voice = voices[note.track][channel];

            voice.voct = (note.pitch - 60.0f) / 12.0f;   // C4 = MIDI 60 = 0 V
            voice.steps_left = std::min(note.length, loop);
            voice.active = true;

            if (was_active[note.track][channel]) voice.retrigger_timer = RETRIGGER_SECONDS;
        }

        playhead_position = position;
        step_phase = 0.0f;
        play_timer = 0.5f;   // the record snap measures from here
    }

    // Quantize "now" to the NEAREST step boundary. Past the half-way point we round
    // FORWARD to the step that has not started yet — that is what makes a slightly
    // early hit land on the beat it was aiming at, so recording never feels late.
    int snapNow(int loop) const
    {
        int step = position < 0 ? 0 : position;

        if (step_samples > 1.0f && step_phase > step_samples * 0.5f)
        {
            step++;
            if (step >= loop) step = 0;   // a hit just before the wrap belongs to step 0
        }
        return step;
    }

    void emitCapturedNote(int pitch, int start, int length, int track)
    {
        unsigned int write = capture_write.load();
        capture_ring[write % CAPTURE_RING] = { pitch, start, length, track };
        capture_write.store(write + 1);
    }

    int recordedLength(float held_samples) const
    {
        if (step_samples <= 1.0f) return 1;
        int length = (int)(held_samples / step_samples + 0.5f);
        return rack::math::clamp(length, 1, MAX_NOTE_LENGTH);
    }

    void abortRecording()
    {
        for (int channel = 0; channel < rack::engine::PORT_MAX_CHANNELS; channel++)
        {
            record_voices[channel].holding = false;
            record_voices[channel].held_samples = 0.0f;
        }
    }

    //
    // One channel of capture. A pitch change while the gate is still held SPLITS the
    // note: plenty of sources never drop the gate between notes — an arpeggiator at
    // full gate length, a slid line — and recording those as one long smear would
    // capture something nobody played.
    //
    void processRecording(const ProcessArgs &args, int loop)
    {
        int channels = inputs[REC_GATE_INPUT].getChannels();

        // A grid is required to quantize against, and until a clock has been
        // measured there is none.
        bool can_record = rec_armed && !locked && step_samples > 1.0f && channels > 0;

        if (!can_record)
        {
            // Re-arm the Schmitts while disarmed, so arming mid-gate cannot capture
            // a note whose beginning was never seen.
            if (!rec_armed)
            {
                for (int channel = 0; channel < rack::engine::PORT_MAX_CHANNELS; channel++)
                {
                    record_gate_triggers[channel].reset();
                }
                abortRecording();
            }
            return;
        }

        for (int channel = 0; channel < channels; channel++)
        {
            RecordVoice &voice = record_voices[channel];

            float gate = inputs[REC_GATE_INPUT].getVoltage(channel);

            // getPolyVoltage: a MONO pitch source paired with a poly gate source
            // normals to every channel, which is what makes that pairing just work.
            float voct = inputs[REC_VOCT_INPUT].getPolyVoltage(channel);
            int pitch = rack::math::clamp((int)std::round(voct * 12.0f + 60.0f), MIN_PITCH, MAX_PITCH);

            bool high = record_gate_triggers[channel].process(gate,
                            constants::gate_low_trigger, constants::gate_high_trigger);

            if (high)
            {
                voice.holding = true;
                voice.pitch = pitch;
                voice.start_step = snapNow(loop);
                voice.held_samples = 0.0f;
            }
            else if (voice.holding && gate <= constants::gate_low_trigger)
            {
                emitCapturedNote(voice.pitch, voice.start_step,
                                 recordedLength(voice.held_samples), active_track);
                voice.holding = false;
                voice.held_samples = 0.0f;
            }

            if (voice.holding)
            {
                if (pitch != voice.pitch)
                {
                    // Close the note in flight and open the next at the boundary.
                    emitCapturedNote(voice.pitch, voice.start_step,
                                     recordedLength(voice.held_samples), active_track);
                    voice.pitch = pitch;
                    voice.start_step = snapNow(loop);
                    voice.held_samples = 0.0f;
                }
                voice.held_samples += 1.0f;
            }
        }
    }

    void process(const ProcessArgs &args) override
    {
        const PlaybackSnapshot &snapshot = snapshots[live_snapshot.load()];

        // ── Reset ────────────────────────────────────────────────────────────
        if (reset_trigger.process(inputs[RESET_INPUT].getVoltage(),
                                  constants::gate_low_trigger,
                                  constants::gate_high_trigger))
        {
            position = -1;
            playhead_position = -1;
            releaseAllVoices();

            // A note being recorded was timed against the old position, so the grid
            // under it just moved: drop it rather than invent a length.
            abortRecording();

            // Ignore the clock briefly, and re-arm the clock Schmitt so a line that
            // is still high does not fire a phantom step when the window expires.
            ignoring_clock = true;
            reset_ignore_timer.reset();
            clock_trigger.reset();

            // Deliberately NOT clearing step_samples: the measured clock period is
            // still valid across a reset, and zeroing it would give the first note
            // recorded afterwards a garbage length.
        }

        if (ignoring_clock && reset_ignore_timer.process(args.sampleTime) >= RESET_IGNORE_SECONDS)
        {
            ignoring_clock = false;
        }

        // ── Clock ────────────────────────────────────────────────────────────
        clock_sample_counter++;

        if (clock_trigger.process(inputs[CLOCK_INPUT].getVoltage(),
                                  constants::gate_low_trigger,
                                  constants::gate_high_trigger))
        {
            if (ignoring_clock)
            {
                // Zero the counter even though the edge is REJECTED. The web
                // original does not, so its next accepted edge measures two clock
                // intervals and reports a step twice as long as it really is.
                clock_sample_counter = 0;
            }
            else
            {
                // Measure the period, with the shared sync debounce window: longer
                // than a sample, shorter than four seconds.
                if (clock_sample_counter > 1 && clock_sample_counter < (long)(args.sampleRate * 4.0f))
                {
                    step_samples = (float)clock_sample_counter;
                }
                clock_sample_counter = 0;

                advanceStep(snapshot);
            }
        }

        step_phase += 1.0f;
        processRecording(args, std::max(1, snapshot.loop_steps));

        // ── Outputs ──────────────────────────────────────────────────────────
        //
        // Channel counts are published EVERY sample, unconditionally:
        // Port::setChannels() silently does nothing while the port is
        // disconnected, so a one-shot push would be lost and the track would come
        // up mono the first time a cable was plugged in.
        for (int track = 0; track < TRACKS; track++)
        {
            uint8_t channels = track_channels[track];

            outputs[VOCT_OUTPUTS + track].setChannels(channels);
            outputs[GATE_OUTPUTS + track].setChannels(channels);

            for (uint8_t channel = 0; channel < channels; channel++)
            {
                Voice &voice = voices[track][channel];

                bool gate_high = voice.active && voice.retrigger_timer <= 0.0f;

                outputs[VOCT_OUTPUTS + track].setVoltage(voice.voct, channel);
                outputs[GATE_OUTPUTS + track].setVoltage(gate_high ? 10.0f : 0.0f, channel);

                if (voice.retrigger_timer > 0.0f) voice.retrigger_timer -= args.sampleTime;
            }
        }

        // The preview overrides the active track's channel 0 while held. Applied
        // AFTER the voice loop and to outputs only, deliberately.
        if (preview_active)
        {
            int track = rack::math::clamp(preview_track, 0, TRACKS - 1);

            outputs[VOCT_OUTPUTS + track].setVoltage((preview_pitch - 60.0f) / 12.0f, 0);
            outputs[GATE_OUTPUTS + track].setVoltage(10.0f, 0);
        }

        if (play_timer > 0.0f) play_timer -= args.sampleTime;
    }

    // ── Persistence ──────────────────────────────────────────────────────────
    //
    // Raw jansson, using json_object_set_new / json_array_append_new EXCLUSIVELY.
    // A bare set/append does not take ownership, so it needs a matching
    // json_decref — which is exactly the refcount leak recently fixed in GrooveBox
    // and AutobreakStudio. Using the _new forms throughout makes the mistake
    // unavailable.

    json_t *dataToJson() override
    {
        json_t *json_root = json_object();

        json_object_set_new(json_root, "version", json_string("1.0.0"));
        json_object_set_new(json_root, "loop_length", json_integer(loop_steps));
        json_object_set_new(json_root, "active_track", json_integer(active_track));
        json_object_set_new(json_root, "snap_steps", json_integer(snap_steps));
        json_object_set_new(json_root, "rec_armed", json_boolean(rec_armed));
        json_object_set_new(json_root, "locked", json_boolean(locked));

        json_t *notes_array = json_array();

        for (size_t i = 0; i < notes.size(); i++)
        {
            const Note &note = notes[i];

            json_t *note_object = json_object();
            json_object_set_new(note_object, "id", json_integer((json_int_t)note.id));
            json_object_set_new(note_object, "pitch", json_integer(note.pitch));
            json_object_set_new(note_object, "start", json_integer(note.start));
            json_object_set_new(note_object, "length", json_integer(note.length));
            json_object_set_new(note_object, "track", json_integer(note.track));

            json_array_append_new(notes_array, note_object);
        }

        json_object_set_new(json_root, "notes", notes_array);

        return json_root;
    }

    void dataFromJson(json_t *json_root) override
    {
        if (!json_root) return;

        // Clear FIRST. This does not only run on patch load — it also runs on module
        // paste and preset load, into an already-populated module.
        notes.clear();

        // Read scalars independently of the note array: a patch missing "notes" must
        // not also lose its loop length. And check key presence explicitly, because
        // a missing key must leave the constructor's default in place rather than
        // silently becoming zero.
        json_t *loop_json = json_object_get(json_root, "loop_length");
        if (loop_json) loop_steps = std::max(1, (int)json_integer_value(loop_json));

        json_t *active_track_json = json_object_get(json_root, "active_track");
        if (active_track_json)
        {
            active_track = rack::math::clamp((int)json_integer_value(active_track_json), 0, TRACKS - 1);
        }

        json_t *snap_json = json_object_get(json_root, "snap_steps");
        if (snap_json) snap_steps = std::max(1, (int)json_integer_value(snap_json));

        json_t *rec_json = json_object_get(json_root, "rec_armed");
        if (rec_json) rec_armed = json_boolean_value(rec_json);

        json_t *locked_json = json_object_get(json_root, "locked");
        if (locked_json) locked = json_boolean_value(locked_json);

        json_t *notes_array = json_object_get(json_root, "notes");

        if (notes_array && json_is_array(notes_array))
        {
            std::set<NoteId> seen_ids;
            size_t count = json_array_size(notes_array);

            for (size_t i = 0; i < count; i++)
            {
                // Bound the loop against MAX_NOTES: a hand-edited patch must not be
                // able to make this module allocate without limit.
                if ((int)notes.size() >= MAX_NOTES) break;

                json_t *note_object = json_array_get(notes_array, i);
                if (!note_object || !json_is_object(note_object)) continue;

                Note note;
                note.id = (NoteId)json_integer_value(json_object_get(note_object, "id"));
                note.pitch = (int)json_integer_value(json_object_get(note_object, "pitch"));
                note.start = (int)json_integer_value(json_object_get(note_object, "start"));
                note.length = (int)json_integer_value(json_object_get(note_object, "length"));
                note.track = (int)json_integer_value(json_object_get(note_object, "track"));

                // Skip garbage rather than clamping it into range: a note silently
                // moved to a different pitch is worse than a note that is missing.
                if (!isValidNote(note, TRACKS, MAX_NOTE_LENGTH)) continue;

                // Re-mint a missing or duplicated id and carry on. The invariant is
                // that every note has a unique identity; throwing to enforce it
                // would take the host down with us.
                if (note.id == NOTE_ID_NONE || seen_ids.count(note.id))
                {
                    note.id = id_minter.mint();
                }

                seen_ids.insert(note.id);
                notes.push_back(note);
            }
        }

        notesChanged();
    }
};
