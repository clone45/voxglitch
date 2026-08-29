#pragma once
// TimelineEngine.hpp — the vxsynth automation engine, ported and then
// restructured around per-lane transport (2026-08-28): every lane owns its
// OWN playhead, so the poly START/STOP/RESET inputs can drive each lane
// independently. No Rack dependencies: compiled standalone by
// tests/timeline/engine_test.cpp.
//
// Kept from the web engine: beats as the time base, linear interpolation
// with a tracked cursor, the hold-before-first/after-last rule, the
// empty-lane-is-0V rule, and the seek serial that lets a consumer tell a
// user seek from a loop wrap (now per lane).
//
// The ownership split, and why it is not negotiable:
//   - A Lane owns its PLAYBACK: playhead, playing, cursor, seek serial,
//     current value. Audio-thread state, never copied by an edit.
//   - The NODE DATA lives in LaneSet, which is double-buffered: the UI
//     thread edits a copy and publishes it atomically while the audio
//     thread reads. If playback state lived inside that buffer, every edit
//     would copy stale playheads over live ones.
// A Lane knows its own index and evaluates against whichever LaneSet the
// caller passes, so the buffering stays invisible to it.
//
// Global (TimelineEngine, applied to every lane): tempo, sample rate, the
// loop end. Global transport gestures are loops over the lanes.

#include <cstring>
#include <cmath>

namespace timeline_dsp
{

static const int    TL_LANES     = 16;
static const int    TL_MAX_NODES = 256;
static const double TL_MAX_BEAT  = 86400.0;   // 12 h at 120 BPM — runaway ceiling
static const double TL_BPM_MIN   = 20.0;
static const double TL_BPM_MAX   = 300.0;

// ── the node store ──────────────────────────────────────────────────────────
// Plain arrays, not vectors: read by the audio thread every sample, swapped
// wholesale by the UI thread. POD, so the swap copy is one memcpy and never
// allocates.
struct LaneSet
{
    int    count[TL_LANES];
    double t[TL_LANES][TL_MAX_NODES];      // beats, ascending
    double v[TL_LANES][TL_MAX_NODES];      // volts, -10..+10

    LaneSet() { clearAll(); }

    void clearAll()
    {
        std::memset(count, 0, sizeof(count));
        std::memset(t, 0, sizeof(t));
        std::memset(v, 0, sizeof(v));
    }

    void clearLane(int L)
    {
        if (L < 0 || L >= TL_LANES) return;
        count[L] = 0;
    }

    // Append in ascending time order (the caller keeps lanes sorted).
    bool add(int L, double beat, double volt)
    {
        if (L < 0 || L >= TL_LANES) return false;
        int n = count[L];
        if (n >= TL_MAX_NODES) return false;
        t[L][n] = beat;
        v[L][n] = volt;
        count[L] = n + 1;
        return true;
    }

    // Insert keeping ascending order; returns the index placed at, or -1.
    int insert(int L, double beat, double volt)
    {
        if (L < 0 || L >= TL_LANES) return -1;
        int n = count[L];
        if (n >= TL_MAX_NODES) return -1;
        int i = n;
        while (i > 0 && t[L][i - 1] > beat)
        {
            t[L][i] = t[L][i - 1];
            v[L][i] = v[L][i - 1];
            i--;
        }
        t[L][i] = beat;
        v[L][i] = volt;
        count[L] = n + 1;
        return i;
    }

    void erase(int L, int idx)
    {
        if (L < 0 || L >= TL_LANES) return;
        int n = count[L];
        if (idx < 0 || idx >= n) return;
        for (int i = idx; i + 1 < n; i++)
        {
            t[L][i] = t[L][i + 1];
            v[L][i] = v[L][i + 1];
        }
        count[L] = n - 1;
    }

    // Re-sort one lane after a drag moved a node in time (insertion sort: a
    // drag moves ONE node, so this is O(n) in practice).
    void resort(int L)
    {
        if (L < 0 || L >= TL_LANES) return;
        int n = count[L];
        for (int i = 1; i < n; i++)
        {
            double bt = t[L][i], bv = v[L][i];
            int j = i - 1;
            while (j >= 0 && t[L][j] > bt)
            {
                t[L][j + 1] = t[L][j];
                v[L][j + 1] = v[L][j];
                j--;
            }
            t[L][j + 1] = bt;
            v[L][j + 1] = bv;
        }
    }

    bool empty() const
    {
        for (int L = 0; L < TL_LANES; L++) if (count[L] > 0) return false;
        return true;
    }

    // The last drawn beat across every lane (0 if nothing is drawn) — the
    // loop-end default and the "content ends here" marker.
    double lastBeat() const
    {
        double m = 0.0;
        for (int L = 0; L < TL_LANES; L++)
            if (count[L] > 0 && t[L][count[L] - 1] > m) m = t[L][count[L] - 1];
        return m;
    }
};

// ── one lane's transport + evaluation ───────────────────────────────────────
struct Lane
{
    int    idx = 0;               // which slot of the LaneSet is mine

    double playhead = 0.0;        // beats
    int    playing = 0;
    int    seekSerial = 0;        // bumped by every seek; a loop wrap does NOT
                                  // bump it — that difference is what keeps
                                  // RWND and LOOP separate signals
    int    cursor = 0;            // active-segment index
    double value = 0.0;           // current output, volts
    int    wrapped = 0;           // set for one tick on a loop wrap

    void setPlaying(int p) { playing = p ? 1 : 0; }

    void seek(double beat)
    {
        if (beat < 0.0) beat = 0.0;
        else if (beat > TL_MAX_BEAT) beat = TL_MAX_BEAT;
        playhead = beat;
        seekSerial++;
        cursor = 0;
    }

    // Evaluate this lane at its own playhead. Linear, cursor-tracked: no
    // per-sample search, and the backward while-loop makes seek-back correct.
    void eval(const LaneSet& ls)
    {
        int n = ls.count[idx];
        if (n == 0) { value = 0.0; return; }             // empty lane = 0 V
        double t = playhead;
        int c = cursor;
        if (c >= n) c = n - 1;
        while (c + 1 < n && t >= ls.t[idx][c + 1]) c++;
        while (c > 0 && t < ls.t[idx][c]) c--;
        cursor = c;
        if (t <= ls.t[idx][0]) { value = ls.v[idx][0]; return; }       // hold before first
        if (c >= n - 1)        { value = ls.v[idx][n - 1]; return; }   // hold after last
        double t0 = ls.t[idx][c], t1 = ls.t[idx][c + 1];
        double v0 = ls.v[idx][c], v1 = ls.v[idx][c + 1];
        double frac = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0;
        value = v0 + frac * (v1 - v0);
    }

    // Advance one sample, then re-evaluate. Tempo and the loop end are the
    // engine's globals, passed in; the accumulator itself is per lane.
    void tick(const LaneSet& ls, double beatsPerSample, double loopEnd)
    {
        wrapped = 0;
        if (playing)
        {
            playhead += beatsPerSample;
            if (loopEnd > 0.0 && playhead >= loopEnd)
            {
                playhead = 0.0;
                wrapped = 1;
                cursor = 0;
            }
            else if (playhead > TL_MAX_BEAT)
            {
                playhead = TL_MAX_BEAT;
                playing = 0;
            }
        }
        eval(ls);
    }
};

// ── the engine: 16 lanes + the globals ──────────────────────────────────────
struct TimelineEngine
{
    Lane   lanes[TL_LANES];
    double bpm = 120.0;
    double sampleRate = 48000.0;
    double loopEnd = 0.0;         // beats; 0 = no loop. GLOBAL: one loop
                                  // length, each lane wrapping at its own
                                  // arrival.

    TimelineEngine()
    {
        for (int L = 0; L < TL_LANES; L++) lanes[L].idx = L;
    }

    void setSampleRate(double sr) { if (sr > 0.0) sampleRate = sr; }

    void setBpm(double b)
    {
        if (b < TL_BPM_MIN) b = TL_BPM_MIN;
        else if (b > TL_BPM_MAX) b = TL_BPM_MAX;
        bpm = b;
    }

    void setLoopEnd(double beat) { loopEnd = (beat > 0.0) ? beat : 0.0; }

    double beatsPerSampleFromBpm() const { return bpm / (60.0 * sampleRate); }

    // Advance every lane one sample.
    void tick(const LaneSet& ls)
    {
        double bps = beatsPerSampleFromBpm();
        for (int L = 0; L < TL_LANES; L++) lanes[L].tick(ls, bps, loopEnd);
    }

    // ── global transport gestures: the panel's PLAY/RWND and the ruler ──
    void setPlayingAll(int p) { for (int L = 0; L < TL_LANES; L++) lanes[L].setPlaying(p); }
    void seekAll(double beat) { for (int L = 0; L < TL_LANES; L++) lanes[L].seek(beat); }

    int anyPlaying() const
    {
        for (int L = 0; L < TL_LANES; L++) if (lanes[L].playing) return 1;
        return 0;
    }
};

} // namespace timeline_dsp
