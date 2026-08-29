//
// PianoRollMidi — Standard MIDI File import and export.
//
// This is the on-disk .mid container, not realtime MIDI: notes written here open
// in any DAW, and melodies from anywhere come back in.
//
// The roll's model is { pitch, start step, length steps, track } where one step is
// a 16th. On disk we use PPQ (ticks per quarter), so a 16th is PPQ/4 ticks: export
// rounds steps to ticks, import rounds ticks back to steps — which quantizes
// imported material to the 16th grid, the roll's native resolution.
//
// MULTI-TRACK: export writes a format-1 file, a conductor track carrying tempo
// plus one MTrk per non-empty roll track, each on its own MIDI channel. Import
// assigns a roll track by MIDI CHANNEL when the file uses more than one (which
// round-trips our own exports and channel-per-part files), otherwise by
// note-bearing-track order — the common one-instrument-per-track case.
//

namespace piano_roll
{
    static const int MIDI_EXPORT_PPQ = 96;      // 96/4 = 24 ticks per step

    // Export writes each note's own velocity. The fixed MIDI_EXPORT_VELOCITY that
    // used to live here is now DEFAULT_VELOCITY in PianoRollGeometry.hpp, and it
    // keeps the same value of 80 so a pattern that predates velocity still
    // exports byte-for-byte identically.

    // ── Writing ──────────────────────────────────────────────────────────────

    inline void midiPushVLQ(std::vector<uint8_t> &out, uint32_t value)
    {
        uint8_t buffer[5];
        int count = 0;

        buffer[count++] = value & 0x7F;
        value >>= 7;
        while (value > 0) { buffer[count++] = (value & 0x7F) | 0x80; value >>= 7; }

        for (int i = count - 1; i >= 0; i--) out.push_back(buffer[i]);
    }

    inline void midiPushU32(std::vector<uint8_t> &out, uint32_t value)
    {
        out.push_back((value >> 24) & 0xFF);
        out.push_back((value >> 16) & 0xFF);
        out.push_back((value >> 8) & 0xFF);
        out.push_back(value & 0xFF);
    }

    inline void midiPushChunk(std::vector<uint8_t> &out, const char *id, const std::vector<uint8_t> &body)
    {
        out.insert(out.end(), id, id + 4);
        midiPushU32(out, (uint32_t)body.size());
        out.insert(out.end(), body.begin(), body.end());
    }

    struct MidiEvent
    {
        uint32_t tick;
        int order;                  // note-offs (0) sort before note-ons (1)
        std::vector<uint8_t> bytes;
    };

    inline bool midiEventLess(const MidiEvent &a, const MidiEvent &b)
    {
        if (a.tick != b.tick) return a.tick < b.tick;
        return a.order < b.order;
    }

    //
    // Writes the whole roll. Returns false only if the file could not be opened.
    //
    inline bool exportMidi(const std::string &path, const std::vector<Note> &notes,
                           int track_count, float tempo_bpm)
    {
        const double ticks_per_step = MIDI_EXPORT_PPQ / 4.0;

        std::vector<std::vector<uint8_t> > chunks;

        // Conductor track: tempo only, the format-1 convention.
        {
            std::vector<uint8_t> body;
            uint32_t us_per_quarter = (uint32_t)(60000000.0 / std::max(1.0f, tempo_bpm));

            midiPushVLQ(body, 0);
            body.push_back(0xFF); body.push_back(0x51); body.push_back(0x03);
            body.push_back((us_per_quarter >> 16) & 0xFF);
            body.push_back((us_per_quarter >> 8) & 0xFF);
            body.push_back(us_per_quarter & 0xFF);

            midiPushVLQ(body, 0);
            body.push_back(0xFF); body.push_back(0x2F); body.push_back(0x00);
            chunks.push_back(body);
        }

        for (int track = 0; track < track_count; track++)
        {
            std::vector<MidiEvent> events;

            for (size_t i = 0; i < notes.size(); i++)
            {
                const Note &note = notes[i];
                if (note.track != track || note.length <= 0) continue;

                uint32_t on = (uint32_t)std::llround(note.start * ticks_per_step);
                uint32_t off = (uint32_t)std::llround(note.end() * ticks_per_step);
                if (off <= on) off = on + 1;

                uint8_t channel = (uint8_t)(track & 0x0F);
                uint8_t pitch = (uint8_t)(note.pitch & 0x7F);

                MidiEvent note_on;
                note_on.tick = on;
                note_on.order = 1;
                note_on.bytes.push_back(0x90 | channel);
                note_on.bytes.push_back(pitch);
                note_on.bytes.push_back((uint8_t)sanitizeVelocity(note.velocity));
                events.push_back(note_on);

                MidiEvent note_off;
                note_off.tick = off;
                note_off.order = 0;
                note_off.bytes.push_back(0x80 | channel);
                note_off.bytes.push_back(pitch);
                note_off.bytes.push_back(0);
                events.push_back(note_off);
            }

            if (events.empty()) continue;   // skip tracks with nothing on them

            std::sort(events.begin(), events.end(), midiEventLess);

            std::vector<uint8_t> body;

            // Track name, so a DAW labels the lane.
            std::string name = "Track " + std::to_string(track + 1);
            midiPushVLQ(body, 0);
            body.push_back(0xFF); body.push_back(0x03);
            body.push_back((uint8_t)name.size());
            body.insert(body.end(), name.begin(), name.end());

            uint32_t last_tick = 0;
            for (size_t i = 0; i < events.size(); i++)
            {
                midiPushVLQ(body, events[i].tick - last_tick);
                body.insert(body.end(), events[i].bytes.begin(), events[i].bytes.end());
                last_tick = events[i].tick;
            }

            midiPushVLQ(body, 0);
            body.push_back(0xFF); body.push_back(0x2F); body.push_back(0x00);
            chunks.push_back(body);
        }

        std::vector<uint8_t> file;
        {
            std::vector<uint8_t> header;
            header.push_back(0); header.push_back(1);                       // format 1
            header.push_back((chunks.size() >> 8) & 0xFF);
            header.push_back(chunks.size() & 0xFF);
            header.push_back((MIDI_EXPORT_PPQ >> 8) & 0xFF);
            header.push_back(MIDI_EXPORT_PPQ & 0xFF);
            midiPushChunk(file, "MThd", header);
        }
        for (size_t i = 0; i < chunks.size(); i++) midiPushChunk(file, "MTrk", chunks[i]);

        std::ofstream stream(path.c_str(), std::ios::binary);
        if (!stream.is_open()) return false;

        stream.write((const char *)file.data(), (std::streamsize)file.size());
        return stream.good();
    }

    // ── Reading ──────────────────────────────────────────────────────────────

    struct MidiReader
    {
        const uint8_t *data;
        size_t size;
        size_t position;

        bool ok() const { return position <= size; }
        size_t remaining() const { return position < size ? size - position : 0; }

        uint8_t byte() { return position < size ? data[position++] : (uint8_t)(position++, 0); }

        uint32_t u16() { uint32_t a = byte(); return (a << 8) | byte(); }
        uint32_t u32() { uint32_t v = 0; for (int i = 0; i < 4; i++) v = (v << 8) | byte(); return v; }

        uint32_t vlq()
        {
            uint32_t value = 0;
            for (int i = 0; i < 4; i++)
            {
                uint8_t b = byte();
                value = (value << 7) | (b & 0x7F);
                if (!(b & 0x80)) break;
            }
            return value;
        }
    };

    struct PendingNoteOn { int pitch; int channel; uint32_t tick; bool active; int velocity; };

    //
    // Parses an SMF into note events. Returns false if the file is not an SMF.
    //
    // `out` notes carry a track index already folded into [0, track_count).
    //
    inline bool importMidi(const std::string &path, int track_count, std::vector<Note> &out)
    {
        std::ifstream stream(path.c_str(), std::ios::binary);
        if (!stream.is_open()) return false;

        std::vector<uint8_t> data((std::istreambuf_iterator<char>(stream)),
                                   std::istreambuf_iterator<char>());
        if (data.size() < 14) return false;

        MidiReader reader = { data.data(), data.size(), 0 };

        if (!(data[0] == 'M' && data[1] == 'T' && data[2] == 'h' && data[3] == 'd')) return false;

        reader.position = 4;
        uint32_t header_length = reader.u32();
        reader.u16();                             // format, unused: we read all tracks
        uint32_t track_count_in_file = reader.u16();
        int division = (int)reader.u16();

        // SMPTE time division is negative in the high bit and is not supported; a
        // musical PPQ is what a piano roll needs.
        if (division <= 0) return false;

        reader.position = 8 + header_length;

        double ticks_per_step = division / 4.0;
        if (ticks_per_step < 1.0) ticks_per_step = 1.0;

        struct RawNote { int pitch; int channel; int chunk; int start; int length; int velocity; };
        std::vector<RawNote> raw;
        std::set<int> channels_seen;
        std::vector<int> chunks_with_notes;

        for (uint32_t chunk_index = 0; chunk_index < track_count_in_file; chunk_index++)
        {
            if (reader.remaining() < 8) break;

            char id[4] = { (char)reader.byte(), (char)reader.byte(), (char)reader.byte(), (char)reader.byte() };
            uint32_t length = reader.u32();
            size_t chunk_end = reader.position + length;

            if (!(id[0] == 'M' && id[1] == 'T' && id[2] == 'r' && id[3] == 'k'))
            {
                reader.position = chunk_end;      // unknown chunk: skip it, per spec
                continue;
            }

            PendingNoteOn pending[16][128];
            for (int c = 0; c < 16; c++) for (int p = 0; p < 128; p++) pending[c][p].active = false;

            uint32_t tick = 0;
            uint8_t running_status = 0;
            bool chunk_has_notes = false;

            while (reader.position < chunk_end && reader.position < reader.size)
            {
                tick += reader.vlq();

                uint8_t status = reader.byte();

                if (status < 0x80)
                {
                    // Running status: the byte just read is actually data.
                    reader.position--;
                    status = running_status;
                    if (status < 0x80) break;      // malformed
                }
                else if (status < 0xF0)
                {
                    running_status = status;
                }

                if (status == 0xFF)                // meta
                {
                    reader.byte();                 // type
                    uint32_t meta_length = reader.vlq();
                    reader.position += meta_length;
                    continue;
                }
                if (status == 0xF0 || status == 0xF7)   // sysex
                {
                    uint32_t sysex_length = reader.vlq();
                    reader.position += sysex_length;
                    continue;
                }

                uint8_t command = status & 0xF0;
                int channel = status & 0x0F;

                if (command == 0x90 || command == 0x80)
                {
                    int pitch = reader.byte() & 0x7F;
                    int velocity = reader.byte() & 0x7F;

                    // A note-on with velocity 0 is a note-off, and most files use it.
                    bool is_note_on = (command == 0x90 && velocity > 0);

                    if (is_note_on)
                    {
                        pending[channel][pitch].pitch = pitch;
                        pending[channel][pitch].channel = channel;
                        pending[channel][pitch].tick = tick;
                        pending[channel][pitch].active = true;

                        // The note-ON velocity is the one that is musically
                        // meaningful. Note-off release velocity is carried by the
                        // same byte position and is almost always 0 or 64 filler,
                        // so it is read and discarded below.
                        pending[channel][pitch].velocity = velocity;
                    }
                    else if (pending[channel][pitch].active)
                    {
                        PendingNoteOn &note_on = pending[channel][pitch];
                        note_on.active = false;

                        int start = (int)std::llround(note_on.tick / ticks_per_step);
                        int end = (int)std::llround(tick / ticks_per_step);
                        int length = std::max(1, end - start);

                        RawNote note = { pitch, channel, (int)chunk_index, std::max(0, start), length,
                                         note_on.velocity };
                        raw.push_back(note);

                        channels_seen.insert(channel);
                        chunk_has_notes = true;
                    }
                }
                else if (command == 0xC0 || command == 0xD0)
                {
                    reader.byte();                 // one data byte
                }
                else
                {
                    reader.byte(); reader.byte();  // two data bytes
                }
            }

            if (chunk_has_notes) chunks_with_notes.push_back((int)chunk_index);
            reader.position = chunk_end;
        }

        if (raw.empty()) return false;

        // Track assignment: by channel when the file uses more than one, else by
        // the order of the chunks that actually carry notes.
        bool by_channel = channels_seen.size() > 1;

        std::map<int, int> chunk_to_track;
        for (size_t i = 0; i < chunks_with_notes.size(); i++)
        {
            chunk_to_track[chunks_with_notes[i]] = (int)(i % track_count);
        }

        out.clear();
        for (size_t i = 0; i < raw.size(); i++)
        {
            const RawNote &note = raw[i];

            int track = by_channel ? (note.channel % track_count) : chunk_to_track[note.chunk];

            Note converted;
            converted.pitch = rack::math::clamp(note.pitch, MIN_PITCH, MAX_PITCH);
            converted.start = std::max(0, note.start);
            converted.length = std::max(1, note.length);
            converted.track = rack::math::clamp(track, 0, track_count - 1);
            converted.velocity = sanitizeVelocity(note.velocity);
            out.push_back(converted);
        }

        return true;
    }

} // namespace piano_roll
