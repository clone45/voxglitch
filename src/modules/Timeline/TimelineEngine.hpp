#pragma once
// TimelineEngine.hpp — the vxsynth automation engine, ported and then
// restructured around per-lane transport (2026-08-28): every lane owns its
// OWN playhead, so the poly START/STOP/RESET inputs can drive each lane
// independently. No Rack dependencies: compiled standalone by the tests in
// tests/timeline/.
//
// Kept from the web engine: beats as the time base, linear interpolation
// with a tracked cursor, the hold-before-first/after-last rule, the
// empty-lane-is-0V rule, and the seek serial that lets a consumer tell a
// user seek from a loop wrap (now per lane).
//
// The ownership split, and why it is not negotiable:
//   - A Lane owns its PLAYBACK: playhead, playing, cursor, seek serial,
//     current value. Audio-thread state, never copied by an edit.
//   - The NODE DATA lives in LaneData, one immutable snapshot per lane,
//     published by pointer swap (LaneStore, 2026-09-03). The UI thread
//     copies the ONE lane it touches, mutates the copy, and publishes it;
//     the audio thread only ever loads the pointer and reads through it.
//     If playback state lived inside the snapshot, every edit would copy
//     stale playheads over live ones.
// A Lane knows its own index and evaluates against whichever LaneData the
// caller passes, so the store stays invisible to it.
//
// Global (TimelineEngine, applied to every lane): tempo, sample rate, the
// loop end. Global transport gestures are loops over the lanes.

#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <atomic>

namespace timeline_dsp
{

static const int    TL_LANES     = 16;
static const int    TL_MAX_NODES = 8192;      // SOFT cap per lane (2026-09-03):
                                              // the JSON bound and the
                                              // "lane full" state. The old
                                              // 256 was the web engine's
                                              // fixed-array budget.
static const double TL_MAX_BEAT  = 86400.0;   // 12 h at 120 BPM — runaway ceiling
static const double TL_BPM_MIN   = 20.0;
static const double TL_BPM_MAX   = 300.0;

inline double tlClampD(double x, double lo, double hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

// ── one lane's nodes: an immutable snapshot once published ──────────────────
// Three parallel float vectors. Mutated ONLY on a private copy (the UI
// thread's), then handed to LaneStore::publish, after which nobody writes
// to it again. The audio thread reads it through a const pointer and never
// allocates, frees or copies.
struct LaneData
{
    std::vector<float> t;      // beats, ascending
    std::vector<float> v;      // volts, -10..+10
    // Segment bend, stored on the segment's LEFT node: 0 = straight line,
    // else the exponential-approach (RC-charge) shape
    //   value = v0 + (v1-v0) * (1 - e^(-b*frac)) / (1 - e^(-b))
    // with b in [-10, 10]. Chosen over frac^k after Bret's feel test: the
    // power curve grows a CORNER at extremes (near-vertical then flat),
    // while this family saturates smoothly, and its exp and log sides are
    // exact mirror images (f_-b(x) = 1 - f_b(1-x)), which frac^k never was.
    // The last node's bend is meaningless (no segment to its right).
    std::vector<float> b;

    int  count() const { return (int)t.size(); }
    bool empty() const { return t.empty(); }
    bool full()  const { return (int)t.size() >= TL_MAX_NODES; }

    void clear() { t.clear(); v.clear(); b.clear(); }

    // Append in ascending time order (the caller keeps lanes sorted).
    bool add(double beat, double volt, double bend = 0.0)
    {
        if (full()) return false;
        t.push_back((float)beat);
        v.push_back((float)volt);
        b.push_back((float)bend);
        return true;
    }

    // Insert keeping ascending order; returns the index placed at, or -1
    // when the lane is full. The new node's own segment starts straight;
    // the left neighbour keeps its bend (its segment now ends at the new
    // node). Placed AFTER any node at the same beat, so a capture at an
    // existing beat lands to its right.
    int insert(double beat, double volt, double bend = 0.0)
    {
        if (full()) return -1;
        int n = count();
        int i = n;
        while (i > 0 && t[i - 1] > (float)beat) i--;
        t.insert(t.begin() + i, (float)beat);
        v.insert(v.begin() + i, (float)volt);
        b.insert(b.begin() + i, (float)bend);
        return i;
    }

    void erase(int idx)
    {
        if (idx < 0 || idx >= count()) return;
        t.erase(t.begin() + idx);
        v.erase(v.begin() + idx);
        b.erase(b.begin() + idx);
    }

    // Remove every node with lo < t <= hi (or lo <= t when incLo). One
    // compaction pass, so a recording sweep costs O(n) not O(n*k).
    int eraseRange(double lo, double hi, bool incLo = false)
    {
        int n = count();
        int w = 0;
        for (int i = 0; i < n; i++)
        {
            double tt = t[i];
            bool inside = (incLo ? tt >= lo : tt > lo) && tt <= hi;
            if (inside) continue;
            if (w != i) { t[w] = t[i]; v[w] = v[i]; b[w] = b[i]; }
            w++;
        }
        t.resize(w); v.resize(w); b.resize(w);
        return n - w;
    }

    // Re-sort after a drag moved a node in time (insertion sort: a drag
    // moves ONE node, so this is O(n) in practice). The bend travels with
    // its left node.
    void resort()
    {
        int n = count();
        for (int i = 1; i < n; i++)
        {
            float bt = t[i], bv = v[i], bb = b[i];
            int j = i - 1;
            while (j >= 0 && t[j] > bt)
            {
                t[j + 1] = t[j]; v[j + 1] = v[j]; b[j + 1] = b[j];
                j--;
            }
            t[j + 1] = bt; v[j + 1] = bv; b[j + 1] = bb;
        }
    }

    // The index of the node at exactly (beat, volt), or -1: how a drag
    // follows its node to the new index after a resort.
    int indexOf(double beat, double volt) const
    {
        int n = count();
        for (int i = 0; i < n; i++)
            if (t[i] == (float)beat && v[i] == (float)volt) return i;
        return -1;
    }

    // The last drawn beat (0 if nothing is drawn).
    double lastBeat() const { return t.empty() ? 0.0 : (double)t.back(); }
};

// The last drawn beat across a set of lanes — the loop-end default and the
// "content ends here" marker. Null entries count as empty.
inline double lastBeatOf(const LaneData* const* lanes)
{
    double m = 0.0;
    for (int L = 0; L < TL_LANES; L++)
        if (lanes[L] && lanes[L]->lastBeat() > m) m = lanes[L]->lastBeat();
    return m;
}

// ── the store: per-lane pointer swap with retire-by-generation ──────────────
// Single writer (the UI thread), single reader (the audio thread).
//
//   audio:  p = live[L].load()  ... read *p ...  ; generation++ per process()
//   UI:     copy = *live[L]; mutate; live[L].store(new LaneData(copy));
//           retired.push_back({old, generation_now});
//           housekeep(): delete retired lanes once
//                        generation > retire_gen + RETIRE_LAG
//
// Why not shared_ptr: the audio thread must never free (a free is a lock
// and an unbounded stall), and refcount traffic on every sample is the
// wrong price for a pointer that changes a few times a second at most.
// Retiring by generation instead lets the audio thread do ONE relaxed
// increment per block, and the UI frees only what the audio thread has
// provably moved past.
//
// The audio generation only advances while process() runs. Bypassed or
// with the engine paused, retired lanes wait; they are freed when it
// resumes, on module removal, and in the destructor. No path leaks.
struct LaneStore
{
    static const uint64_t RETIRE_LAG = 2;

    std::atomic<const LaneData*> live[TL_LANES];
    std::atomic<uint64_t> audioGeneration;

    struct Retired
    {
        const LaneData* lane = 0;
        uint64_t gen = 0;
    };
    std::vector<Retired> retired;      // UI thread only

    LaneStore()
    {
        audioGeneration.store(0);
        for (int L = 0; L < TL_LANES; L++) live[L].store(new LaneData());
    }

    ~LaneStore()
    {
        freeRetired(true);
        for (int L = 0; L < TL_LANES; L++)
        {
            const LaneData* p = live[L].load();
            live[L].store(0);
            delete p;
        }
    }

    // Every caller-supplied lane index is clamped before it reaches
    // `live[]`. Cheap, and it keeps the access provably in range once the
    // reads are inlined through the editor: GCC 15's range analysis
    // otherwise sees an unbounded index and reports a -Wstringop-overflow
    // on the atomic load (Bret's MinGW build, 2026-09-03).
    static int laneIndex(int L)
    {
        if (L < 0) return 0;
        if (L >= TL_LANES) return TL_LANES - 1;
        return L;
    }

    // ── audio thread ──
    const LaneData* load(int L) const
    {
        return live[laneIndex(L)].load(std::memory_order_acquire);
    }
    void loadAll(const LaneData* out[TL_LANES]) const
    {
        for (int L = 0; L < TL_LANES; L++) out[L] = live[L].load(std::memory_order_acquire);
    }
    // Once per process(), AFTER the last read of this block's pointers.
    void bumpGeneration() { audioGeneration.fetch_add(1, std::memory_order_release); }

    // ── UI thread ──
    const LaneData& lane(int L) const
    {
        return *live[laneIndex(L)].load(std::memory_order_acquire);
    }

    // Publish a new snapshot for lane L. Takes ownership of `fresh` (heap,
    // never touched again by the caller). The old snapshot goes to the
    // retire list, stamped with the generation at retirement.
    void publish(int L, const LaneData* fresh)
    {
        if (L < 0 || L >= TL_LANES || !fresh) { delete fresh; return; }
        const LaneData* old = live[L].exchange(fresh, std::memory_order_acq_rel);
        Retired r;
        r.lane = old;
        r.gen = audioGeneration.load(std::memory_order_acquire);
        retired.push_back(r);
    }

    // Convenience: publish a copy of `src`.
    void publishCopy(int L, const LaneData& src) { publish(L, new LaneData(src)); }

    // Free every retired snapshot the audio thread has provably moved past.
    // `all` skips the generation test: only legal once process() can no
    // longer run (module removed, or destruction).
    void freeRetired(bool all = false)
    {
        uint64_t now = audioGeneration.load(std::memory_order_acquire);
        size_t w = 0;
        for (size_t i = 0; i < retired.size(); i++)
        {
            const Retired& r = retired[i];
            if (all || now > r.gen + RETIRE_LAG) delete r.lane;
            else retired[w++] = r;
        }
        retired.resize(w);
    }

    size_t retiredCount() const { return retired.size(); }
};

// ── one lane's transport + evaluation ───────────────────────────────────────
struct Lane
{
    int    idx = 0;               // which slot of the store is mine

    double playhead = 0.0;        // beats
    int    playing = 0;
    int    seekSerial = 0;        // bumped by every seek; a loop wrap does NOT
                                  // bump it — that difference is what keeps
                                  // RWND and LOOP separate signals
    int    cursor = 0;            // active-segment index
    double value = 0.0;           // current output, volts
    int    wrapped = 0;           // set for one tick on a loop wrap
    double seekBeat = 0.0;        // where the last seek landed (the recorder
                                  // sees the lane one tick later)

    void setPlaying(int p) { playing = p ? 1 : 0; }

    void seek(double beat)
    {
        if (beat < 0.0) beat = 0.0;
        else if (beat > TL_MAX_BEAT) beat = TL_MAX_BEAT;
        playhead = beat;
        seekBeat = beat;
        seekSerial++;
        cursor = 0;
    }

    // Evaluate this lane at its own playhead. Linear, cursor-tracked: no
    // per-sample search, and the backward while-loop makes seek-back correct.
    // A null snapshot reads as an empty lane.
    void eval(const LaneData* ld)
    {
        int n = ld ? ld->count() : 0;
        if (n == 0) { value = 0.0; return; }             // empty lane = 0 V
        const float* T = &ld->t[0];
        const float* V = &ld->v[0];
        double t = playhead;
        int c = cursor;
        if (c >= n) c = n - 1;
        while (c + 1 < n && t >= (double)T[c + 1]) c++;
        while (c > 0 && t < (double)T[c]) c--;
        cursor = c;
        if (t <= (double)T[0]) { value = V[0]; return; }          // hold before first
        if (c >= n - 1)        { value = V[n - 1]; return; }      // hold after last
        double t0 = T[c], t1 = T[c + 1];
        double v0 = V[c], v1 = V[c + 1];
        double frac = (t1 > t0) ? (t - t0) / (t1 - t0) : 0.0;
        // Segment bend: the exponential-approach shape. A straight segment
        // skips the exps entirely, so lanes without curves cost exactly what
        // they always did. Endpoints are exact for ANY bend (f(0)=0, f(1)=1
        // by construction), so a bend can never step at a node.
        double bend = ld->b[c];
        if (bend > 1e-9 || bend < -1e-9)
            frac = (1.0 - std::exp(-bend * frac)) / (1.0 - std::exp(-bend));
        value = v0 + frac * (v1 - v0);
    }

    // Advance one sample, then re-evaluate. Tempo and the loop end are the
    // engine's globals, passed in; the accumulator itself is per lane.
    void tick(const LaneData* ld, double beatsPerSample, double loopEnd)
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
        eval(ld);
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

    // Advance every lane one sample against this block's snapshots
    // (one pointer per lane, loaded by the caller).
    void tick(const LaneData* const* ld)
    {
        double bps = beatsPerSampleFromBpm();
        for (int L = 0; L < TL_LANES; L++) lanes[L].tick(ld[L], bps, loopEnd);
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

// ── recording (2026-09-03): latch mode, the only mode ───────────────────────
// A TAKE runs while REC is armed AND a target lane is playing. The audio
// thread captures the input at every record-rate grid line the playhead
// crosses and pushes (lane, beat, volts) into a lock-free ring; the UI
// thread drains the ring, replacing the nodes the playhead has passed over
// and leaving everything it has not reached untouched. That replacement
// rule IS latch mode. At the take's end the recorded region is simplified
// and the whole take becomes one undo step.
//
// Audio side: Recorder + CaptureRing. UI side: TakeAssembler +
// simplifyRegion. Both Rack-free, tested by tests/timeline/record_test.cpp.

static const int    TL_CAPTURE_RING   = 8192;   // entries; overflow drops the oldest
static const double TL_REC_TOL        = 0.02;   // simplifier tolerance, volts
static const int    TL_REC_RATE_COUNT = 6;      // 1 bar .. 1/32
static const double TL_REC_V_MAX      = 10.0;
static const float  TL_REC_VALUE_EPS  = 0.001f;  // "unchanged" tolerance, volts

// The record-rate grid in beats: index into {1 bar, 1/2, 1/4, 1/8, 1/16, 1/32}.
inline double recRateBeats(int idx)
{
    static const double R[TL_REC_RATE_COUNT] = { 4.0, 2.0, 1.0, 0.5, 0.25, 0.125 };
    if (idx < 0) idx = 0;
    if (idx >= TL_REC_RATE_COUNT) idx = TL_REC_RATE_COUNT - 1;
    return R[idx];
}

enum CaptureKind { CAP_POINT = 0, CAP_START = 1, CAP_END = 2 };

struct Capture
{
    int   lane = 0;
    int   kind = CAP_POINT;
    int   wrap = 0;         // the sweep since the previous capture crossed the loop end
    int   seek = 0;         // the lane was seeked since the previous capture (a wrap
                            // does not count): nothing between was passed over
    float beat = 0.f;
    float volt = 0.f;
    float wrapEnd = 0.f;    // the loop end it crossed (valid when wrap)
};

// Single producer (audio), single consumer (UI). Fixed storage, no
// allocation on either side. On overflow the producer DROPS the new entry
// and counts it: overwriting the oldest instead would write into the very
// slot the consumer may be copying out of, and a torn capture is worse
// than a missing one.
struct CaptureRing
{
    Capture buf[TL_CAPTURE_RING];
    std::atomic<unsigned int> write;
    std::atomic<unsigned int> read;
    std::atomic<unsigned int> dropped;

    CaptureRing() { write.store(0); read.store(0); dropped.store(0); }

    void push(const Capture& c)                  // audio thread
    {
        unsigned int w = write.load(std::memory_order_relaxed);
        unsigned int r = read.load(std::memory_order_acquire);
        if (w - r >= (unsigned int)TL_CAPTURE_RING)
        {
            dropped.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        buf[w % TL_CAPTURE_RING] = c;
        write.store(w + 1, std::memory_order_release);
    }

    bool pop(Capture& out)                       // UI thread
    {
        unsigned int w = write.load(std::memory_order_acquire);
        unsigned int r = read.load(std::memory_order_relaxed);
        if (r == w) return false;
        out = buf[r % TL_CAPTURE_RING];
        read.store(r + 1, std::memory_order_release);
        return true;
    }

    unsigned int pending() const
    {
        return write.load(std::memory_order_acquire) - read.load(std::memory_order_acquire);
    }
};

// ── audio side ──
struct Recorder
{
    int      active = 0;
    unsigned mask = 0;                     // target lanes, FROZEN at the take start
    long long lastCell[TL_LANES] = {};     // grid cell each lane was last seen in
    int      wrapPending[TL_LANES] = {};
    float    wrapEnd[TL_LANES] = {};
    int      seekPending[TL_LANES] = {};
    int      lastSeekSerial[TL_LANES] = {};
    int      fullStop[TL_LANES] = {};      // hit the cap this take: no more captures

    // Mono (or unpatched) records into the selected lane; N poly channels
    // record channel c into lane c, clamped to 16.
    static unsigned targetMask(int channels, int selectedLane)
    {
        if (channels <= 1)
        {
            if (selectedLane < 0) selectedLane = 0;
            if (selectedLane >= TL_LANES) selectedLane = TL_LANES - 1;
            return 1u << selectedLane;
        }
        int n = channels > TL_LANES ? TL_LANES : channels;
        return (1u << n) - 1u;
    }

    bool targets(int L) const { return ((mask >> L) & 1u) != 0u; }

    static float clampV(float v)
    {
        return (float)tlClampD(v, -TL_REC_V_MAX, TL_REC_V_MAX);
    }

    bool anyTargetPlaying(unsigned m, const TimelineEngine& eng) const
    {
        for (int L = 0; L < TL_LANES; L++)
            if (((m >> L) & 1u) && eng.lanes[L].playing) return true;
        return false;
    }

    // Only the requested lanes that are PLAYING at the take start take
    // part: a parked lane has no sweep to record, and bypassing it for the
    // whole take would silence it for nothing.
    void begin(unsigned wantMask, const TimelineEngine& eng, double grid,
               const float* volts, CaptureRing& ring)
    {
        active = 1;
        mask = 0;
        for (int L = 0; L < TL_LANES; L++)
            if (((wantMask >> L) & 1u) && eng.lanes[L].playing) mask |= (1u << L);
        for (int L = 0; L < TL_LANES; L++)
        {
            if (!targets(L)) continue;
            const Lane& ln = eng.lanes[L];
            lastCell[L] = (long long)std::floor(ln.playhead / grid);
            wrapPending[L] = 0;
            wrapEnd[L] = 0.f;
            seekPending[L] = 0;
            lastSeekSerial[L] = ln.seekSerial;
            fullStop[L] = 0;
            Capture c;
            c.lane = L;
            c.kind = CAP_START;
            c.beat = (float)ln.playhead;
            c.volt = clampV(volts[L]);
            ring.push(c);
        }
    }

    void end(const TimelineEngine& eng, const float* volts, CaptureRing& ring)
    {
        for (int L = 0; L < TL_LANES; L++)
        {
            if (!targets(L)) continue;
            const Lane& ln = eng.lanes[L];
            Capture c;
            c.lane = L;
            c.kind = CAP_END;
            c.wrap = wrapPending[L];
            c.wrapEnd = wrapEnd[L];
            c.seek = seekPending[L] || (ln.seekSerial != lastSeekSerial[L]);
            c.beat = (float)ln.playhead;
            c.volt = clampV(volts[L]);
            wrapPending[L] = 0;
            seekPending[L] = 0;
            ring.push(c);
        }
        active = 0;
        mask = 0;
    }

    // Once per sample, AFTER the engine ticked (it reads each lane's
    // `wrapped` flag for this sample). `armed` is the REC switch; `wantMask`
    // the lanes a take started now would target; `volts[L]` the input for
    // lane L; `ld` this sample's snapshots (for the cap check).
    void process(bool armed, unsigned wantMask, const TimelineEngine& eng, double grid,
                 const float* volts, const LaneData* const* ld, CaptureRing& ring)
    {
        if (grid <= 0.0) grid = 1.0;
        if (!active)
        {
            if (!armed || !anyTargetPlaying(wantMask, eng)) return;
            begin(wantMask, eng, grid, volts, ring);
            return;
        }
        if (!armed || !anyTargetPlaying(mask, eng))
        {
            end(eng, volts, ring);
            return;
        }
        for (int L = 0; L < TL_LANES; L++)
        {
            if (!targets(L)) continue;
            const Lane& ln = eng.lanes[L];
            if (ln.wrapped) { wrapPending[L] = 1; wrapEnd[L] = (float)eng.loopEnd; }
            // A seek (ruler scrub, RESET, RWND) bumps the serial; a wrap
            // does not. The next capture is flagged so the drain knows
            // nothing between the old and new positions was passed over.
            if (ln.seekSerial != lastSeekSerial[L])
            {
                lastSeekSerial[L] = ln.seekSerial;
                seekPending[L] = 1;
            }
            if (!ln.playing) continue;
            long long cell = (long long)std::floor(ln.playhead / grid);
            if (cell == lastCell[L] && !wrapPending[L] && !seekPending[L]) continue;
            lastCell[L] = cell;
            if (fullStop[L]) continue;
            if (ld[L] && ld[L]->full()) { fullStop[L] = 1; continue; }
            Capture c;
            c.lane = L;
            c.kind = CAP_POINT;
            c.wrap = wrapPending[L];
            c.wrapEnd = wrapEnd[L];
            c.seek = seekPending[L];
            // A seek lands on the seek target itself (the lane has already
            // ticked once past it); a crossing lands on the grid line.
            c.beat = seekPending[L] ? (float)ln.seekBeat : (float)((double)cell * grid);
            c.volt = clampV(volts[L]);
            wrapPending[L] = 0;
            seekPending[L] = 0;
            ring.push(c);
        }
    }
};

// ── UI side ──
// Ramer-Douglas-Peucker over the recorded region [lo, hi] of a lane, with
// deviation measured vertically (volts against the chord), since the two
// axes have different units. Only nodes the take itself inserted
// (`removable`, their beats) may go: after a wrap or a seek the bounding
// interval contains stretches the playhead never passed, and the
// hand-placed nodes there are the latch promise. Nodes listed in
// `keepBeats` (the take's first and last captures) and any node carrying a
// bend are never removed; the region's own end nodes are the outermost
// chord ends and survive too.
inline void simplifyRegion(LaneData& d, double lo, double hi, double tol,
                           const float* keepBeats, int nKeep,
                           const std::vector<float>& removable)
{
    int n = d.count();
    std::vector<float> rem(removable);
    std::sort(rem.begin(), rem.end());
    int ia = -1, ib = -1;
    for (int i = 0; i < n; i++)
    {
        double tt = d.t[i];
        if (tt < lo || tt > hi) continue;
        if (ia < 0) ia = i;
        ib = i;
    }
    if (ia < 0 || ib - ia < 2) return;

    std::vector<char> keep(n, 1);
    for (int i = ia + 1; i < ib; i++)
    {
        if (!std::binary_search(rem.begin(), rem.end(), d.t[i])) continue;
        keep[i] = 0;
        if (d.b[i] != 0.f) keep[i] = 1;
        for (int k = 0; k < nKeep; k++) if (d.t[i] == keepBeats[k]) keep[i] = 1;
    }

    // Iterative RDP between consecutive kept nodes.
    std::vector<int> stack;
    int a = ia;
    for (int i = ia + 1; i <= ib; i++)
    {
        if (!keep[i]) continue;
        stack.push_back(a);
        stack.push_back(i);
        a = i;
    }
    while (!stack.empty())
    {
        int b1 = stack.back(); stack.pop_back();
        int a1 = stack.back(); stack.pop_back();
        if (b1 - a1 < 2) continue;
        double ta = d.t[a1], tb = d.t[b1];
        double va = d.v[a1], vb = d.v[b1];
        double span = tb - ta;
        int best = -1;
        double bestDev = -1.0;
        for (int i = a1 + 1; i < b1; i++)
        {
            double frac = (span > 0.0) ? ((double)d.t[i] - ta) / span : 0.0;
            double line = va + (vb - va) * frac;
            double dev = std::fabs((double)d.v[i] - line);
            if (dev > bestDev) { bestDev = dev; best = i; }
        }
        if (best >= 0 && bestDev > tol)
        {
            keep[best] = 1;
            stack.push_back(a1); stack.push_back(best);
            stack.push_back(best); stack.push_back(b1);
        }
    }

    int w = 0;
    for (int i = 0; i < n; i++)
    {
        if (!keep[i]) continue;
        if (w != i) { d.t[w] = d.t[i]; d.v[w] = d.v[i]; d.b[w] = d.b[i]; }
        w++;
    }
    d.t.resize(w); d.v.resize(w); d.b.resize(w);
}

struct TakeAssembler
{
    struct LaneTake
    {
        int   open = 0;        // START seen, END not yet
        int   ended = 0;
        int   wrapped = 0;
        int   full = 0;        // an insert failed at the cap this take
        float prev = 0.f;      // the previous capture's beat
        float first = 0.f;     // the take's first capture beat (protected)
        float last = 0.f;      // ... and its most recent (protected at the end)
        float lo = 0.f, hi = 0.f;   // the recorded region
        float lastValue = 0.f; // the value of the last node this take inserted
        int   holdPending = 0; // a grid capture was NOT inserted (value unchanged)...
        float holdBeat = 0.f;  // ...at this beat: the hold's end, written on a change
        LaneData before;       // the lane at the take start, for undo
        std::vector<float> captured;   // beats this take inserted: the only
                                       // nodes the simplifier may remove
    };

    void insertNode(LaneTake& lt, LaneData& d, float beat, float volt)
    {
        if (d.insert(beat, volt, 0.0) < 0) lt.full = 1;
        else lt.captured.push_back(beat);
    }
    LaneTake lanes[TL_LANES];

    // Apply one capture to `d`, the lane's private copy of its live
    // snapshot. Returns true when `d` changed.
    bool apply(const Capture& c, LaneData& d)
    {
        if (c.lane < 0 || c.lane >= TL_LANES) return false;
        LaneTake& lt = lanes[c.lane];
        bool fresh = (c.kind == CAP_START) || !lt.open || lt.ended;
        if (fresh)
        {
            // A new take on this lane (or a take whose START was lost to a
            // ring overflow): remember the lane for undo, and clear only
            // the node sitting exactly on the start beat.
            lt = LaneTake();
            lt.open = 1;
            lt.before = d;
            lt.first = c.beat;
            lt.lo = lt.hi = c.beat;
            d.eraseRange(c.beat, c.beat, true);
        }
        else if (c.wrap)
        {
            // The sweep ran off the loop end and back in from 0.
            d.eraseRange(lt.prev, c.wrapEnd);
            d.eraseRange(0.0, c.beat, true);
            lt.wrapped = 1;
            lt.lo = 0.f;
            if (c.wrapEnd > lt.hi) lt.hi = c.wrapEnd;
        }
        else if (c.seek || c.beat < lt.prev)
        {
            // A seek mid-take (either direction): nothing between the old
            // and new positions was passed over, so only the node on this
            // beat is replaced and the sweep restarts here.
            d.eraseRange(c.beat, c.beat, true);
        }
        else
        {
            d.eraseRange(lt.prev, c.beat);        // the nodes passed over
        }
        // A node is written only when the VALUE changes (Bret, 2026-09-03).
        // Every grid capture still sweeps the passed-over region above, so
        // the latch erases old nodes even under a static input; what it
        // writes back is: the take START (anchors the take), a node at
        // each change, the grid point BEFORE a change (so the hold stays
        // flat right up to it instead of ramping across the gap), and the
        // take END (pins where the latch stops).
        bool changed = fresh || std::fabs(c.volt - lt.lastValue) > TL_REC_VALUE_EPS;
        // A hold spanning the loop wrap is pinned at the last grid point
        // before the loop end; the wrap capture then starts a new hold.
        if (!fresh && c.wrap && lt.holdPending && lt.holdBeat != c.beat)
        {
            insertNode(lt, d, lt.holdBeat, lt.lastValue);
            lt.holdPending = 0;
        }
        // Did the previous capture put a node on THIS beat? (Read before
        // `prev` moves on.) An END there would only duplicate it.
        bool onPrevBeat = (c.beat == lt.prev);
        bool prevInserted = !lt.holdPending || lt.holdBeat != lt.prev;
        lt.prev = c.beat;
        lt.last = c.beat;
        if (c.beat < lt.lo) lt.lo = c.beat;
        if (c.beat > lt.hi) lt.hi = c.beat;
        if (fresh)
        {
            insertNode(lt, d, c.beat, c.volt);
            lt.lastValue = c.volt;
        }
        else if (c.kind == CAP_END)
        {
            // The END pins the latch's edge. It is skipped only when the
            // previous capture already put a node on this very beat.
            if (changed && lt.holdPending && lt.holdBeat != c.beat)
                insertNode(lt, d, lt.holdBeat, lt.lastValue);
            bool pin = !onPrevBeat || !prevInserted;
            if (pin) insertNode(lt, d, c.beat, c.volt);
            lt.holdPending = 0;
        }
        else if (!changed)
        {
            lt.holdPending = 1;
            lt.holdBeat = c.beat;
        }
        else
        {
            if (lt.holdPending && lt.holdBeat != c.beat)
                insertNode(lt, d, lt.holdBeat, lt.lastValue);
            insertNode(lt, d, c.beat, c.volt);
            lt.lastValue = c.volt;
            lt.holdPending = 0;
        }
        if (c.kind == CAP_END)
        {
            float keep[2] = { lt.first, lt.last };
            simplifyRegion(d, lt.lo, lt.hi, TL_REC_TOL, keep, 2, lt.captured);
            lt.ended = 1;
        }
        return true;
    }

    // True once every lane with an open take has received its END.
    bool complete() const
    {
        bool any = false;
        for (int L = 0; L < TL_LANES; L++)
        {
            if (!lanes[L].open) continue;
            if (!lanes[L].ended) return false;
            any = true;
        }
        return any;
    }

    bool inTake(int L) const { return lanes[L].open && !lanes[L].ended; }

    void reset() { for (int L = 0; L < TL_LANES; L++) lanes[L] = LaneTake(); }
};


} // namespace timeline_dsp
