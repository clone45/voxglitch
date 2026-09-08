#pragma once
// VXDrumVoices.hpp — the VX Drums voice models, kits, and the stereo mix, as
// pure DSP (DESIGN.md §3.2 + DESIGN-KITS.md §1-3).
//
// Six COLUMNS (BD SD CP PERC CH OH), each holding one instance of every voice
// model and an `active` pointer. A KIT is a named row of six ModelIds; the
// module resolves kit + per-column overrides and calls setModel(). Every model
// exposes three sound knobs, TUNE / DECAY / SHAPE, stored per column as
// normalized 0..1 (v01) and mapped to the model's natural range through its
// ModelSpec; LEVEL is a plain gain at the mix.
//
// The six original voices (Kick 808, Snare 909, Clap, Rimshot, Closed Hat,
// Open Hat) are the vxdrumvoices.c port that shipped in the first cut of
// this file, arithmetic untouched, only re-housed as Model structs; their
// source lines are still cited as `vxdrumvoices.c:NNN`. The six new kernels
// (Kick Sine / Tom from kickdrum.c, Kick FM from kickfm.c, Kick Distort from
// kickdist.c, Snare 808 from snare808.c, Snare Layered from snaredrum.c,
// Snare Ring from snarering.c) are ported line for line below with
// `<file>.c:NNN` citations. Snare 606, Snare Sweep and Snare Gate, and the
// four claps Clap 909, Clap Trap, Clap Lo-Fi and Clap Gate, are ORIGINAL
// designs (2026-09-02, no vxsynth source): nothing to cite, so each carries a
// short description of its mechanism instead.
//
// Deliberate deviations from the C (DESIGN.md §3.2):
//   1. fastTanh is GAFastMath's Padé x(27+x²)/(27+9x²), clamped to ±1 at
//      |x| > 3, copied VERBATIM — std::tanh audibly softens the drives.
//   2. fastSin(turns) → std::sin(2π·turns) (the source table's worst-case
//      error is 3.5e-7 relative). fastExp2 → std::exp2 (only reachable through
//      the stripped 1V/oct paths, so it does not appear below).
//   3. js_exp → std::exp, fast_floor → std::floor, fast_abs → std::fabs,
//      dsp_clamp → clampd below (rack::math::clamp has no double overload,
//      and it would silently narrow to float).
//   4. rng_uniform → ONE xorshift32 in Voices (top 24 bits → [0,1)), handed to
//      each model by reference at strike/tick time, seeded by the module from
//      random::u32() | 1u so two kits decorrelate. The draw ORDER is column
//      order (0..5); the first cut ticked BD RS SD CP CH OH, so the noise
//      stream differs from it, statistically not audibly.
//   5. Per-sample constants that are only right at 48 kHz become time
//      constants, so the sound is the same at every rate:
//        clap burst decay 0.9931/sample     → exp(-1/(0.0030·sr))
//        hat choke fade   0.9977/sample     → exp(-1/(0.00905·sr))
//        snaredrum noise lowpass keep 0.7   → exp(-2π·2724.8/sr)   (snaredrum.c:160)
//      Each equals the source constant at 48 kHz. The hat HP/BP tilt
//      coefficients are NOT rate-corrected in the source and define the
//      timbre; they stay verbatim (see Hat::tick).
//   6. The Machine's setter clamps (vxdrums.c:568-578: hat tone and metal,
//      accent, drive → 0..1) are applied even though vxdrumvoices.c omits them.
//   7. The `< 1e-4 → 0` envelope floors and the NaN / 1e6 guards are kept.
//   8. Level / velocity / CV / attenuverter / 1V-oct plumbing of the standalone
//      modules is stripped: the kit strikes with `amp` (accent applied) where
//      the source struck with Level (+ velocity), and LEVEL is a mix gain. The
//      standalone ±5 V output scales are divided by 5 so a strike at amp 1
//      peaks near ±1 internal units (kickdrum/kickfm/kickdist/snaredrum ×5 →
//      ×1, snare808 ×3 → ×0.6, snarering ×4.5 → ×0.9).
//   9. snare808.c:108 arms its noise envelope at 1.0 regardless of Level;
//      here it is armed at `amp` so the accent rides the whole hit (identical
//      at amp 1, the standalone's default).
//
// No Rack types here: <cmath> <cstdint> <cstring> only. Double precision
// throughout, as the source.

#include <cmath>
#include <cstdint>
#include <cstring>

namespace vx_drums
{

// ── Identity ─────────────────────────────────────────────────────────────
// Column indices: input order = strip order = output order.   vxdrumvoices.c:43-51
const int COLUMNS = 6;
const int VOICES = COLUMNS;   // alias kept for older call sites
enum Lane { LANE_BD = 0, LANE_SD = 1, LANE_CP = 2, LANE_PERC = 3, LANE_CH = 4, LANE_OH = 5 };

// process() output slots: [mixL, mixR, c0..c5].   :97
const int OUTPUTS = 8;

enum ModelId { MODEL_KICK_808, MODEL_KICK_SINE, MODEL_KICK_FM, MODEL_KICK_DIST,
               MODEL_SNARE_909, MODEL_SNARE_808, MODEL_SNARE_LAYERED, MODEL_SNARE_RING,
               MODEL_SNARE_606, MODEL_SNARE_SWEEP, MODEL_SNARE_GATE,
               MODEL_CLAP, MODEL_CLAP_909, MODEL_CLAP_TRAP, MODEL_CLAP_LOFI, MODEL_CLAP_GATE,
               MODEL_RIMSHOT, MODEL_TOM, MODEL_HAT_CLOSED, MODEL_HAT_OPEN, NUM_MODELS };
enum KitId { KIT_HOUSE, KIT_TR808, KIT_TR909, KIT_LAYERED, KIT_INDUSTRIAL, KIT_HARDCORE, KIT_EIGHTIES, NUM_KITS };

// One sound knob: natural = min + v01 * (max - min). For "%" specs the natural
// range is 0..1 and display_multiplier_percent = 1 (shown ×100).
struct KnobSpec { const char* name; double min; double max; double def; const char* unit; int display_multiplier_percent; };
struct ModelSpec { ModelId id; const char* uuid; const char* name; KnobSpec tune; KnobSpec decay; KnobSpec shape; };
struct KitSpec   { KitId id; const char* uuid; const char* name; ModelId models[COLUMNS]; };

// UUIDs: frozen, persisted, never change. Persistence uses these strings,
// never the enum value or the display name.   DESIGN-KITS.md §1
const ModelSpec MODEL_SPECS[NUM_MODELS] = {
    { MODEL_KICK_808,      "68e73d73-263a-4fa0-a374-707a3ba61aaf", "Kick 808",
      { "Tune",  30.0,  100.0,  52.0,  " Hz", 0 }, { "Decay", 0.05, 1.5, 0.45, " s", 0 }, { "Punch",  0.0, 1.0, 0.5,  "%", 1 } },
    { MODEL_KICK_SINE,     "d5cda8ff-f404-4e3e-9724-056b2096fea1", "Kick Sine",
      { "Tune",  20.0,  200.0,  60.0,  " Hz", 0 }, { "Decay", 0.05, 1.0, 0.3,  " s", 0 }, { "Punch",  0.0, 1.0, 0.5,  "%", 1 } },
    { MODEL_KICK_FM,       "7fac18f9-fec2-40f8-874c-2d1dec83ed5c", "Kick FM",
      { "Tune",  30.0,  120.0,  50.0,  " Hz", 0 }, { "Decay", 0.1,  1.0, 0.4,  " s", 0 }, { "FM",     0.0, 1.0, 0.5,  "%", 1 } },
    { MODEL_KICK_DIST,     "bc3c4c17-82a8-4fae-b784-ed1731a6833c", "Kick Distort",
      { "Tune",  30.0,  120.0,  50.0,  " Hz", 0 }, { "Decay", 0.1,  1.0, 0.4,  " s", 0 }, { "Drive",  0.0, 1.0, 0.6,  "%", 1 } },
    { MODEL_SNARE_909,     "e66c0614-6a94-426a-8375-99b8683a5d18", "Snare 909",
      { "Tune", 120.0,  280.0, 180.0,  " Hz", 0 }, { "Decay", 0.05, 0.5, 0.2,  " s", 0 }, { "Snap",   0.0, 1.0, 0.7,  "%", 1 } },
    { MODEL_SNARE_808,     "6a179ab9-154b-4618-b404-f33136a4f98a", "Snare 808",
      { "Tune", 100.0,  300.0, 180.0,  " Hz", 0 }, { "Decay", 0.03, 0.5, 0.15, " s", 0 }, { "Snappy", 0.0, 1.0, 0.6,  "%", 1 } },
    { MODEL_SNARE_LAYERED, "415f5069-b56a-4237-9aa7-6ca744ea2071", "Snare Layered",
      { "Tune", 100.0,  400.0, 200.0,  " Hz", 0 }, { "Decay", 0.05, 0.5, 0.2,  " s", 0 }, { "Snap",   0.0, 1.0, 0.7,  "%", 1 } },
    { MODEL_SNARE_RING,    "9a837656-9063-4e1d-9ab7-f5be27652eb1", "Snare Ring",
      { "Tune", 100.0,  400.0, 200.0,  " Hz", 0 }, { "Decay", 0.05, 0.5, 0.2,  " s", 0 }, { "Ratio",  1.0, 4.0, 1.6,  "",  0 } },
    { MODEL_SNARE_606,     "a8cf99b0-de6b-49f6-b3a5-24630513d140", "Snare 606",
      { "Tune", 120.0,  400.0, 200.0,  " Hz", 0 }, { "Decay", 0.03, 0.4, 0.12, " s", 0 }, { "Snappy", 0.0, 1.0, 0.7,  "%", 1 } },
    { MODEL_SNARE_SWEEP,   "cacaa03c-ef71-4567-be59-6246bb551f8a", "Snare Sweep",
      { "Tune",  80.0,  300.0, 150.0,  " Hz", 0 }, { "Decay", 0.1,  1.0, 0.35, " s", 0 }, { "Sweep",  0.0, 1.0, 0.6,  "%", 1 } },
    { MODEL_SNARE_GATE,    "c79621f7-9fbc-4930-9652-46cdc6a83332", "Snare Gate",
      { "Tune", 120.0,  300.0, 190.0,  " Hz", 0 }, { "Decay", 0.1,  0.8, 0.3,  " s", 0 }, { "Hold",   0.0, 1.0, 0.7,  "%", 1 } },
    { MODEL_CLAP,          "67e3989c-8e84-44c9-a328-ab5473a181cd", "Clap 808",
      { "Tone", 600.0, 2500.0, 1550.0, " Hz", 0 }, { "Decay", 0.05, 0.5, 0.23, " s", 0 }, { "Spread", 6.0, 16.0, 10.0, " ms", 0 } },
    { MODEL_CLAP_909,      "30a5d65b-7718-4d67-9543-1573854355c2", "Clap 909",
      { "Tune", 600.0, 2500.0, 1100.0, " Hz", 0 }, { "Decay", 0.05, 0.5, 0.25, " s", 0 }, { "Density",  0.0, 1.0, 0.5, "%", 1 } },
    { MODEL_CLAP_TRAP,     "1568a42f-1b26-413a-81ef-74292fdc5d5f", "Clap Trap",
      { "Tune", 800.0, 3000.0, 1600.0, " Hz", 0 }, { "Decay", 0.05, 0.6, 0.2,  " s", 0 }, { "Humanize", 0.0, 1.0, 0.5, "%", 1 } },
    { MODEL_CLAP_LOFI,     "8daca98c-0906-4513-a5b7-29aed1dd2035", "Clap Lo-Fi",
      { "Tune", 700.0, 2500.0, 1400.0, " Hz", 0 }, { "Decay", 0.05, 0.4, 0.15, " s", 0 }, { "Crunch",   0.0, 1.0, 0.6, "%", 1 } },
    { MODEL_CLAP_GATE,     "343a6117-7932-4a6a-8cbf-4471dc9e08b8", "Clap Gate",
      { "Tune", 600.0, 2500.0, 1000.0, " Hz", 0 }, { "Decay", 0.1,  0.8, 0.35, " s", 0 }, { "Hold",     0.0, 1.0, 0.7, "%", 1 } },
    { MODEL_RIMSHOT,       "3cfbcdb2-2f23-4fa6-a6a0-db96a11d5618", "Rimshot",
      { "Tune", 300.0, 1200.0, 520.0,  " Hz", 0 }, { "Decay", 0.02, 0.15, 0.06, " s", 0 }, { "Drive", 0.0, 1.0, 0.3,  "%", 1 } },
    { MODEL_TOM,           "57a99e87-a71e-42a8-8567-fc8b7729e573", "Tom",
      { "Tune",  60.0,  300.0, 120.0,  " Hz", 0 }, { "Decay", 0.05, 0.8, 0.25, " s", 0 }, { "Punch",  0.0, 1.0, 0.3,  "%", 1 } },
    { MODEL_HAT_CLOSED,    "913f1973-cabc-43a0-8856-bdad6dbf9680", "Closed Hat",
      { "Tone",   0.0,    1.0,   0.6,  "%",   1 }, { "Decay", 0.02, 0.2, 0.06, " s", 0 }, { "Metal",  0.0, 1.0, 0.5,  "%", 1 } },
    { MODEL_HAT_OPEN,      "ee02c667-eb80-45ad-ad87-1409b2103482", "Open Hat",
      { "Tone",   0.0,    1.0,   0.6,  "%",   1 }, { "Decay", 0.05, 1.2, 0.35, " s", 0 }, { "Metal",  0.0, 1.0, 0.5,  "%", 1 } },
};

// Column order: BD SD CP PERC CH OH.   DESIGN-KITS.md §2
const KitSpec KIT_SPECS[NUM_KITS] = {
    { KIT_HOUSE,      "d38a9894-cf91-420a-8ae4-ac8809f76891", "House",
      { MODEL_KICK_808,  MODEL_SNARE_909,     MODEL_CLAP, MODEL_RIMSHOT,    MODEL_HAT_CLOSED, MODEL_HAT_OPEN } },
    { KIT_TR808,      "9d9c4c3d-0eaa-4a1d-aa3a-903b69fb3b8f", "TR-808",
      { MODEL_KICK_808,  MODEL_SNARE_808,     MODEL_CLAP, MODEL_RIMSHOT,    MODEL_HAT_CLOSED, MODEL_HAT_OPEN } },
    { KIT_TR909,      "83a87f66-01ee-4270-a0a7-cf0786bad1a6", "TR-909",
      { MODEL_KICK_SINE, MODEL_SNARE_909,     MODEL_CLAP_909, MODEL_RIMSHOT, MODEL_HAT_CLOSED, MODEL_HAT_OPEN } },
    { KIT_LAYERED,   "dabfa691-03d6-4db4-8a73-1125fd8092b1", "Layered",
      { MODEL_KICK_SINE, MODEL_SNARE_LAYERED, MODEL_CLAP, MODEL_TOM,        MODEL_HAT_CLOSED, MODEL_HAT_OPEN } },
    { KIT_INDUSTRIAL, "eb55f3a4-8b3b-4115-8f4e-e550273147ba", "Industrial",
      { MODEL_KICK_DIST, MODEL_SNARE_RING,    MODEL_CLAP, MODEL_TOM,        MODEL_HAT_CLOSED, MODEL_HAT_OPEN } },
    { KIT_HARDCORE,   "485157e7-916e-4328-b60d-c682b3af00f8", "Hardcore",
      { MODEL_KICK_FM,   MODEL_SNARE_909,     MODEL_CLAP, MODEL_SNARE_RING, MODEL_HAT_CLOSED, MODEL_HAT_OPEN } },
    { KIT_EIGHTIES,   "f1b90e3d-482f-4f29-a1ee-1cf12ee9c35d", "Eighties",
      { MODEL_KICK_SINE, MODEL_SNARE_GATE,    MODEL_CLAP_GATE, MODEL_SNARE_SWEEP, MODEL_HAT_CLOSED, MODEL_HAT_OPEN } },
};

inline const ModelSpec& modelSpec(ModelId id)
{
    int i = (int)id;
    if (i < 0 || i >= NUM_MODELS) i = 0;
    return MODEL_SPECS[i];
}
inline const KitSpec& kitSpec(KitId id)
{
    int i = (int)id;
    if (i < 0 || i >= NUM_KITS) i = 0;
    return KIT_SPECS[i];
}
inline ModelId modelFromUuid(const char* uuid, ModelId fallback)
{
    if (uuid == nullptr) return fallback;
    for (int i = 0; i < NUM_MODELS; i++)
        if (std::strcmp(MODEL_SPECS[i].uuid, uuid) == 0) return MODEL_SPECS[i].id;
    return fallback;
}
inline KitId kitFromUuid(const char* uuid, KitId fallback)
{
    if (uuid == nullptr) return fallback;
    for (int i = 0; i < NUM_KITS; i++)
        if (std::strcmp(KIT_SPECS[i].uuid, uuid) == 0) return KIT_SPECS[i].id;
    return fallback;
}

// ── Helpers ──────────────────────────────────────────────────────────────
const double PI = 3.14159265358979323846;   // core_head.c:440

// core_head.c:592-597 — dsp_clamp, mirrored verbatim as max(lo, min(hi, x)) on
// comparison-based helpers. A NaN passes through UNCHANGED (every comparison
// against NaN is false, so each helper returns its second argument); do not
// swap in std::fmax/std::fmin, which would silently turn a NaN into hi. The
// NaN guards live in the ticks (std::isnan), not here.
inline double fastMax(double a, double b) { return a > b ? a : b; }
inline double fastMin(double a, double b) { return a < b ? a : b; }
inline double clampd(double x, double lo, double hi) { return fastMax(lo, fastMin(hi, x)); }
inline double clamp01(double x) { return clampd(x, 0.0, 1.0); }

inline double knobNatural(const KnobSpec& k, double v01)        { return k.min + clamp01(v01) * (k.max - k.min); }
inline double knobNormalized(const KnobSpec& k, double natural)
{
    double span = k.max - k.min;
    if (!(span > 0.0)) return 0.0;
    return clamp01((natural - k.min) / span);
}
inline double knobDefault01(const KnobSpec& k) { return knobNormalized(k, k.def); }

// core_head.c:701-706 — GAFastMath's Padé tanh. NOT a close approximation of
// tanh (hotter by up to +0.0235, exactly 1.0 at x = 3, hence the continuous
// clamp); always used as fastTanh(x·dr) / fastTanh(dr). Deviation 1: verbatim.
inline double fastTanh(double x)
{
    if (x < -3.0) return -1.0;   // GAFastMath clamps to avoid Padé overflow
    if (x >  3.0) return  1.0;
    double x2 = x * x;
    return x * (27.0 + x2) / (27.0 + 9.0 * x2);
}

// Deviation 2: fastSin(turns) → std::sin(2π·turns). Phase in TURNS [0,1).
inline double sinTurns(double turns) { return std::sin(2.0 * PI * turns); }

// GA's GAFastMath::fastFMod: NaN/Inf → 0, then wrap to [0,1).   kickdrum.c:98-101, snaredrum.c:110-113
inline double fmod01(double phase)
{
    if (std::isnan(phase) || phase > 1e30 || phase < -1e30) phase = 0.0;
    return phase - std::floor(phase);
}

// Chamberlin SVF frequency coefficient: 2·sin(π·fc/sr) via fastSin(fc/(2sr)) turns,
// clamped as the source.   vxdrumvoices.c:126-128, snare808.c:58-59
inline double svfF(double fc, double sampleRate)
{
    return clampd(2.0 * sinTurns(fc / (2.0 * sampleRate)), 0.0001, 1.4);
}
// Chamberlin damping from T60.   vxdrumvoices.c:129-132
inline double svfQ(double fc, double decay)
{
    double t60 = (decay > 0.02) ? decay : 0.02;
    return clampd(6.9 / (PI * fc * t60), 0.006, 0.9);
}

// T60 (seconds) → per-sample envelope multiplier, exp(-ln(1000)·sampleTime/t60):
// the envelope is 60 dB down after t60 at every rate. The three house-designed
// snares (606 / Sweep / Gate) state every decay this way.
inline double t60Coeff(double t60, double sampleTime)
{
    if (t60 < 0.001) t60 = 0.001;
    return std::exp(-6.907755278982137 * sampleTime / t60);
}
// White noise with a constant per-sample variance has half the power per Hz
// at twice the sample rate, so a filtered noise band gets 3 dB quieter at
// 96 kHz. The three house-designed snares scale their FILTERED noise by
// sqrt(sr / 44100) so the band-limited noise power is the same at every rate
// (the ported kernels keep the source's constant-variance noise verbatim).
inline double noisePsdGain(double sampleRate) { return std::sqrt(sampleRate / 44100.0); }
// One-pole coefficient for a cutoff in Hz, clamped so it stays stable at any rate.
inline double onePoleCoeff(double fc, double sampleRate, double lo, double hi)
{
    return clampd(2.0 * PI * fc / sampleRate, lo, hi);
}

// ── Noise: core_head.c:717-725, one per Voices (deviation 4) ──
struct Rng
{
    uint32_t state = 1u;   // xorshift32 must never be all-zero

    void seed(uint32_t s) { state = s | 1u; }   // the house seeding rule (vxdrumvoices.c:303)
    uint32_t next()
    {
        uint32_t x = state;
        if (x == 0u) x = 0x9e3779b9u;       // avoid the all-zero fixed point
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        state = x;
        return x;
    }
    double uniform() { return (double)(next() >> 8) * (1.0 / 16777216.0); }   // top 24 bits → [0,1)
    double white() { return uniform() * 2.0 - 1.0; }   // every noise source in the C: rng_uniform()*2-1
};

// ── The abstract model ───────────────────────────────────────────────────
// Knob setters take NATURAL units (Hz, s, 0..1, ms...). Coefficients are cached
// on knob change and on setSampleRate; strike()/tick() never call a
// transcendental except where the source did. reset() silences the voice and
// keeps knobs + coefficients. choke() is a no-op except on the hats.
struct Model
{
    double sampleRate = 44100.0;
    double sampleTime = 1.0 / 44100.0;

    virtual ~Model() {}
    virtual void setSampleRate(double sr) = 0;
    virtual void setTune(double natural) = 0;
    virtual void setDecay(double natural) = 0;
    virtual void setShape(double natural) = 0;
    virtual void strike(double amp, Rng& rng) = 0;
    virtual void choke() {}
    virtual double tick(Rng& rng) = 0;
    virtual void reset() = 0;

protected:
    // Shared by every setSampleRate: the Machine's guard (vxdrums.c:459).
    void cacheRate(double sr)
    {
        if (!(sr > 0.0)) sr = 44100.0;
        sampleRate = sr;
        sampleTime = 1.0 / sr;
    }
};

// ── Kick 808 / Rimshot: the struck-SVF kernel (kick808.c's mechanism) ──
// vxdrumvoices.c:55-61, :133-140, :169-175, :193-215, :384-385
// `rimshot` = false: SHAPE is Punch, drive FIXED at 0.25, the strike sweeps
// pitch. `rimshot` = true: no punch sweep, SHAPE is Drive, strike × 0.9.
struct Ping808 : Model
{
    bool rimshot = false;
    double tune = 52.0, decay = 0.45, shape = 0.5;   // natural: Hz, s, punch or drive 0..1

    double low = 0.0, band = 0.0;    // Chamberlin SVF state — band IS the drum
    double fBase = 0.0, q = 0.0;     // cached coefficients (tune/decay setters)
    double pitchEnv = 0.0;           // punch sweep 1→0 (BD only; RS leaves punch 0)
    double clickN = 0.0;             // beater-click sample countdown
    double clickSamp = 0.0;
    double pitchCoeff = 0.0;         // punch-envelope coefficient, τ 28 ms   :114, :304

    Ping808() { Ping808::setSampleRate(44100.0); }

    void updateCoeffs()   // :133-140
    {
        fBase = svfF(tune, sampleRate);
        q     = svfQ(tune, decay);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        pitchCoeff = std::exp(-sampleTime / 0.028);   // :304 — τ 28 ms punch sweep
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v;  updateCoeffs(); }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = v; }
    void reset() override
    {
        low = 0.0; band = 0.0; pitchEnv = 0.0; clickN = 0.0; clickSamp = 0.0;
    }
    void strike(double amp, Rng& rng) override   // :169-175, RS strike × 0.9 :305 (first cut strikeLane)
    {
        if (rimshot) amp *= 0.9;
        band = clampd(amp, 0.0, 2.5);   // the strike IS the state (kick808)
        low = 0.0;
        if (!rimshot) pitchEnv = 1.0;
        clickN = sampleRate * 0.004;    // 4 ms beater click
        clickSamp = rng.white();
    }
    double tick(Rng& rng) override   // :193-215
    {
        double punchAmt = rimshot ? 0.0 : shape;    // :384-385 — BD punch from the knob, RS none
        double driveAmt = rimshot ? shape : 0.25;   //           BD drive FIXED at 0.25, RS from the knob
        if (pitchEnv > 0.0001) pitchEnv *= pitchCoeff; else pitchEnv = 0.0;
        double f = fBase;
        if (pitchEnv > 0.0001 && punchAmt > 0.0) {
            f = clampd(fBase * (1.0 + pitchEnv * punchAmt * 2.5), 0.0001, 1.4);
        }
        low += f * band;
        double high = -low - q * band;
        band += f * high;
        double body = band;
        if (std::isnan(low)  || std::fabs(low)  > 1e6) low = 0.0;    // :203-204 guards
        if (std::isnan(band) || std::fabs(band) > 1e6) band = 0.0;

        double click = 0.0;
        if (clickN > 0.0) {
            click = clickSamp * (clickN / (sampleRate * 0.004)) * 0.3;
            clickN -= 1.0;
            clickSamp = rng.white();
        }
        double out = body + click;
        double dr = 0.6 + driveAmt * 7.0;
        return fastTanh(out * dr) / fastTanh(dr);   // level-matched soft clip
    }
};

// ── Snare 909: snare909.c's shells + two-band noise ──
// vxdrumvoices.c:64-72, :141-149, :176-180, :387-405
struct Snare909 : Model
{
    double tune = 180.0, decay = 0.2, shape = 0.7;   // natural: Hz, s, snap 0..1

    double phaseA = 0.0, phaseB = 0.0;
    double envA = 0.0, envB = 0.0;         // shell envelopes (fixed decays)
    double envLo = 0.0, envHi = 0.0;       // noise bands (decay knob)
    double coeffLo = 0.0, coeffHi = 0.0;   // cached from decay
    double shellCA = 0.0, shellCB = 0.0;   // FIXED shell coefficients (0.12 s / 0.085 s pair)
    double toneLp = 0.0, snapHp = 0.0;     // noise filters
    double freqA = 0.0, freqB = 0.0;       // cached from tune

    Snare909() { Snare909::setSampleRate(44100.0); }

    void updateCoeffs()   // :141-149
    {
        freqA = tune;
        freqB = tune * 1.5;                                // snare909's shell ratio
        double d = (decay > 0.01) ? decay : 0.01;
        coeffLo = std::exp(-(5.0 / d)         * sampleTime);
        coeffHi = std::exp(-(5.0 / (d * 0.4)) * sampleTime);
        shellCA = std::exp(-(5.0 / 0.12)  * sampleTime);   // snare909's fixed pair
        shellCB = std::exp(-(5.0 / 0.085) * sampleTime);
    }
    void setSampleRate(double sr) override { cacheRate(sr); updateCoeffs(); }
    void setTune(double v) override  { tune = v;  updateCoeffs(); }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = v; }
    void reset() override
    {
        phaseA = 0.0; phaseB = 0.0; envA = 0.0; envB = 0.0; envLo = 0.0; envHi = 0.0;
        toneLp = 0.0; snapHp = 0.0;
    }
    void strike(double amp, Rng&) override   // :176-180
    {
        phaseA = 0.0; phaseB = 0.0;
        envA = amp; envB = amp;
        envLo = amp; envHi = amp;
    }
    double tick(Rng& rng) override   // :387-405 (snare909's tick, inlined)
    {
        phaseA += freqA * sampleTime; if (phaseA >= 1.0) phaseA -= 1.0;
        phaseB += freqB * sampleTime; if (phaseB >= 1.0) phaseB -= 1.0;
        if (envA > 0.0001) envA *= shellCA; else envA = 0.0;
        if (envB > 0.0001) envB *= shellCB; else envB = 0.0;
        double triA = 4.0 * std::fabs(phaseA - 0.5) - 1.0;
        double triB = 4.0 * std::fabs(phaseB - 0.5) - 1.0;
        double sdBody = 0.4 * 0.5 * (envA * triA + envB * triB);
        double w = rng.white();
        double toneCoeff = clampd(2.0 * PI * 5000.0 / sampleRate, 0.02, 1.0);   // :397 (rate-aware, kept)
        toneLp += toneCoeff * (w - toneLp);
        double hpCoeff = clampd(2.0 * PI * 3800.0 / sampleRate, 0.0, 0.98);     // :399 (rate-aware, kept)
        snapHp += hpCoeff * (toneLp - snapHp);
        double nHi = toneLp - snapHp;
        if (envLo > 0.0001) envLo *= coeffLo; else envLo = 0.0;
        if (envHi > 0.0001) envHi *= coeffHi; else envHi = 0.0;
        double sdV = sdBody + shape * (toneLp * envLo + 1.4 * nHi * envHi);
        return sdV * 0.9;
    }
};

// ── Clap 808: clap.c's burst scheduler + BP ──
// vxdrumvoices.c:75-84 (initial values :320-321), :150-158, :181-186, :217-239
// Natural knobs: TUNE = band centre in Hz (600..2500, was 600 + tone·1900),
// DECAY = tail seconds (was 0.05 + decay·0.45), SHAPE = burst spread in ms
// (was 6 + spread·10 ms). Same internals; the knobs just carry natural units.
struct Clap : Model
{
    double tune = 1550.0, decay = 0.23, shape = 10.0;   // natural: Hz, s, ms

    int seqCounter = -1;             // samples since the last burst (-1 idle)
    int burstsLeft = 0;
    int spacing = 1;                 // samples between bursts (SPREAD)
    double burstEnv = 0.0, tailEnv = 0.0;
    double tailCoeff = 0.0;          // cached from decay
    double low = 0.0, band = 0.0;    // Chamberlin BP state
    double f = 0.0;                  // cached from tone
    double amp = 0.0;                // strike amplitude for the scheduled bursts
    double burstCoeff = 0.0;         // deviation 5: burst decay, τ 3.0 ms (source 0.9931/sample, :226)

    Clap() { Clap::setSampleRate(44100.0); }

    void updateCoeffs()   // :150-158
    {
        f = svfF(tune, sampleRate);                   // clap.c's band, natural Hz
        tailCoeff = std::exp(-sampleTime / decay);
        double sp = shape * 0.001;                    // 6..16 ms → seconds
        spacing = (int)(sp * sampleRate);
        if (spacing < 1) spacing = 1;
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        burstCoeff = std::exp(-1.0 / (0.0030 * sr));   // deviation 5 (0.9931 at 48 kHz)
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v;  updateCoeffs(); }
    void setDecay(double v) override { decay = (v > 0.0001) ? v : 0.0001; updateCoeffs(); }
    void setShape(double v) override { shape = v; updateCoeffs(); }
    void reset() override
    {
        seqCounter = -1; burstsLeft = 0; burstEnv = 0.0; tailEnv = 0.0;
        low = 0.0; band = 0.0; amp = 0.0;
    }
    void strike(double a, Rng&) override   // :181-186
    {
        burstsLeft = 3;
        seqCounter = spacing;         // first burst fires immediately in tick
        tailEnv = a;
        amp = a;
    }
    double tick(Rng& rng) override   // :217-239
    {
        if (burstsLeft > 0) {
            seqCounter++;
            if (seqCounter >= spacing) {
                seqCounter = 0;
                burstsLeft--;
                burstEnv = amp;
            }
        }
        burstEnv *= burstCoeff;                       // :226 `0.9931` — deviation 5, τ 3 ms at every rate
        if (burstEnv < 0.0001) burstEnv = 0.0;
        if (tailEnv > 0.0001) tailEnv *= tailCoeff; else tailEnv = 0.0;

        double excite = (burstEnv + 0.7 * tailEnv);
        if (excite <= 0.0) { low = 0.0; band = 0.0; return 0.0; }
        double w = rng.white();
        double in = w * excite;
        low += f * band;
        double high = in - low - 0.6 * band;          // q = 0.6 fixed
        band += f * high;
        if (std::isnan(band) || std::fabs(band) > 1e6) { band = 0.0; low = 0.0; }   // :237 guard
        return band * 1.6;
    }
};

// ── Closed / Open Hat: hihat.c's metallic cluster ──
// vxdrumvoices.c:87-93, :159-166, :187-190, :241-272. One kernel serves both
// ModelIds; only the DECAY range differs (in the ModelSpec). choke() is the
// CH→OH fast fade, applied by Voices::strikeLane as a column rule.
const double HAT_RATIO[6] = { 1.0, 1.4471, 1.6170, 1.9265, 2.5028, 2.6637 };   // :123

struct Hat : Model
{
    double tune = 0.6, decay = 0.06, shape = 0.5;   // natural: tone 0..1, s, metal 0..1

    double phase[6] = {};
    double env = 0.0;
    double hpIn = 0.0, hpOut = 0.0, bp = 0.0;   // one-pole HP + BP-residue states
    double coeff = 0.0;              // cached from decay
    bool choking = false;            // fast-fade instead of a clicky hard cut
    double chokeCoeff = 0.0;         // deviation 5: choke fade, τ 9.05 ms (source 0.9977/sample, :243)

    Hat() { Hat::setSampleRate(44100.0); }

    void updateCoeffs()   // :159-165 — 9.21034 = ln(1e4): env hits 1e-4 at `seconds`
    {
        double seconds = decay;
        if (seconds < 0.005) seconds = 0.005;
        double c = std::exp(-sampleTime * 9.21034 / seconds);
        if (std::isnan(c)) c = 0.0;          // the source's !(c == c)
        coeff = clampd(c, 0.0, 0.99999);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        chokeCoeff = std::exp(-1.0 / (0.00905 * sr));   // deviation 5 (0.9977 at 48 kHz)
        updateCoeffs();
    }
    void setTune(double v) override  { tune = clampd(v, 0.0, 1.0); }    // vxdrums.c:568/572 (deviation 6)
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); }   // vxdrums.c:570/574
    void reset() override
    {
        for (int i = 0; i < 6; i++) phase[i] = 0.0;
        env = 0.0; hpIn = 0.0; hpOut = 0.0; bp = 0.0; choking = false;
    }
    void strike(double amp, Rng&) override   // :187-190
    {
        env = clampd(amp, 0.0, 2.0);
        choking = false;
    }
    // THE CHOKE: a closed hat cuts the open hat's ring (fast fade, not a hard
    // zero — a hard zero clicks).   :284-286
    void choke() override { if (env > 0.0001) choking = true; }
    double tick(Rng& rng) override   // :241-272
    {
        double tone = tune, metal = shape;
        if (choking) {
            env *= chokeCoeff;                        // :243 `0.9977` — deviation 5, τ 9.05 ms at every rate
            if (env < 0.0001) { env = 0.0; choking = false; }
        } else if (env > 0.0001) {
            env *= coeff;
        } else {
            env = 0.0;
        }
        if (env <= 0.0) { hpIn = 0.0; hpOut = 0.0; bp = 0.0; return 0.0; }   // phases freeze while silent

        double base = 320.0 * (1.0 + tone * 0.5);     // 320..480 Hz
        double sq = 0.0;
        for (int i = 0; i < 6; i++) {
            phase[i] += base * HAT_RATIO[i] * sampleTime;
            if (phase[i] >= 1.0) phase[i] -= std::floor(phase[i]);   // :256 (double)fast_floor
            sq += (phase[i] < 0.5) ? 1.0 : -1.0;
        }
        sq *= (1.0 / 6.0);
        double w = rng.white();
        double source = sq * (0.25 + 0.85 * metal) + w * (0.95 - 0.55 * metal);

        // NOT rate-corrected, on purpose: these per-sample tilt coefficients
        // are the hat's timbre in the source (:263, :266) and stay verbatim
        // (DESIGN.md §3.2 deviation 5, second half).
        double hpCoeff = 0.80 + tone * 0.185;
        double hp = hpCoeff * (hpOut + source - hpIn);
        hpIn = source; hpOut = hp;
        double lpCoeff = 0.30 + tone * 0.65;
        bp += lpCoeff * (hp - bp);
        double bandRes = hp - bp;
        double filtered = hp * (1.0 - tone) + bandRes * tone;
        double out = filtered * env;
        return clampd(out, -2.0, 2.0);
    }
};

// ── Kick Sine / Tom: kickdrum.c (GAKickDrum) — sine body + pitch sweep + click ──
// One kernel serves both ModelIds; `click` is the FIXED source knob (Kick Sine
// 0.5, Tom 0.1) and the ranges differ in the ModelSpec. Stripped: VEL
// (kickdrum.c:163) and 1V/oct PTCH (:134-138); the strike arms the amplitude
// envelope at `amp` where the source armed it at 1.0 (:110). Output ×5 (:166)
// → ×1 (deviation 8).
struct SineKick : Model
{
    double tune = 60.0, decay = 0.3, shape = 0.5;   // natural: Hz, s, punch 0..1
    double click = 0.5;                             // FIXED per ModelId (kickdrum.js:53 default)

    double phase = 0.0;                             // kickdrum.c:36-40
    double ampEnvelope = 0.0;
    double pitchEnvelope = 0.0;
    double clickSample = 0.0;
    int    clickCounter = 0;
    double ampDecayCoeff = 0.0;                     // exp(-(7/max(.01,decay))  * sampleTime)   :45
    double pitchDecayCoeff = 0.0;                   // exp(-(70/max(.01,decay)) * sampleTime)   :46
    double clickDurationSamples = 0.0;              // sampleRate * 0.003   :47

    SineKick() { SineKick::setSampleRate(44100.0); }

    void updateCoeffs()   // kickdrum.c:58-64
    {
        double d = (decay > 0.01) ? decay : 0.01;   // GA std::max(0.01, decay)
        double decayRate      = 7.0  / d;
        double pitchDecayRate = 70.0 / d;
        ampDecayCoeff   = std::exp(-decayRate      * sampleTime);
        pitchDecayCoeff = std::exp(-pitchDecayRate * sampleTime);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        clickDurationSamples = sampleRate * 0.003;   // :78 — KICK_CLICK_SECONDS, rate-aware in the source
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; }                   // :88
    void setDecay(double v) override { decay = v; updateCoeffs(); }  // :90-94
    void setShape(double v) override { shape = v; }                  // :95
    void reset() override   // :70-74
    {
        phase = 0.0; ampEnvelope = 0.0; pitchEnvelope = 0.0; clickSample = 0.0; clickCounter = 0;
    }
    void strike(double amp, Rng& rng) override   // :109-116
    {
        ampEnvelope = amp;                          // source: 1.0 (Level plumbing stripped, deviation 8)
        pitchEnvelope = 1.0;
        phase = 0.0;
        // Click noise burst (GA: rand()/RAND_MAX*2-1 -> [-1,1]; RNG substituted).
        clickSample = rng.white();
        clickCounter = (int)(sampleRate * 0.003);   // 3ms
    }
    double tick(Rng& rng) override   // :119-168
    {
        // Amplitude envelope (exponential decay, cached coeff).   :120-124
        if (ampEnvelope > 0.0001) ampEnvelope *= ampDecayCoeff; else ampEnvelope = 0.0;
        // Pitch envelope (faster decay, cached coeff).   :127-131
        if (pitchEnvelope > 0.0001) pitchEnvelope *= pitchDecayCoeff; else pitchEnvelope = 0.0;

        // Base pitch (1V/oct CV stripped, :134-138).
        double basePitch = tune;

        // Pitch envelope sweep (punch: up to 3x freq at onset).   :141-143
        double pitchMultiplier = 1.0 + pitchEnvelope * shape * 3.0;
        double currentFreq = basePitch * pitchMultiplier;
        currentFreq = clampd(currentFreq, 10.0, 500.0);

        // Advance phase, wrap to [0,1), sine body.   :146-148
        phase += currentFreq * sampleTime;
        phase = fmod01(phase);
        double oscOutput = sinTurns(phase);

        // Click burst (linear-fade noise; regenerates each sample for richness).   :151-157
        double clickOutput = 0.0;
        if (clickCounter > 0) {
            double clickEnv = (double)clickCounter / clickDurationSamples;
            clickOutput = clickSample * clickEnv * click;
            clickCounter--;
            clickSample = rng.white();
        }

        // Mix + amplitude envelope.   :160 (velocity :163 and ×5 :166 stripped)
        return (oscOutput + clickOutput) * ampEnvelope;
    }
};

// ── Kick FM: kickfm.c — 2-operator PM kick ──
// SHAPE = FM amount; FIXED ratio 1.0, punch 0.5, drive 0.3 (kickfm.js:56-62
// defaults). Stripped: Level/VEL strike (kickfm.c:110), 1V/oct PTCH (:122),
// FM CV (:134), Decay CV (:150-152). Output ×5 (:144) → ×1 (deviation 8).
struct KickFm : Model
{
    double tune = 50.0, decay = 0.4, shape = 0.5;   // natural: Hz, s, fm 0..1
    double ratio = 1.0, punch = 0.5, drive = 0.3;   // FIXED

    double phaseC = 0.0, phaseM = 0.0;              // kickfm.c:35-36, phase in TURNS [0,1)
    double ampEnv = 0.0, fmEnv = 0.0, pitchEnv = 0.0;   // :38-40, all 1→0, armed on strike
    double ampDecayCoeff = 0.0;                     // :42 from Decay
    double fmDecayCoeff = 0.0;                      // :43 fixed (KICKFM_FM_TAU 0.02 s)
    double pitchDecayCoeff = 0.0;                   // :44 fixed (KICKFM_PITCH_TAU 0.03 s)

    KickFm() { KickFm::setSampleRate(44100.0); }

    void updateCoeffs()   // :64-67 — T60 ≈ 6.9/k, so k = 7/decay makes the knob read ≈ seconds
    {
        double d = (decay > 0.02) ? decay : 0.02;
        ampDecayCoeff = std::exp(-(7.0 / d) * sampleTime);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        fmDecayCoeff    = std::exp(-(1.0 / 0.02) * sampleTime);   // :78 — τ 20 ms, rate-aware in the source
        pitchDecayCoeff = std::exp(-(1.0 / 0.03) * sampleTime);   // :79 — τ 30 ms
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; }                   // :91
    void setDecay(double v) override { decay = v; updateCoeffs(); }  // :95
    void setShape(double v) override { shape = v; }                  // :93 (fm)
    void reset() override   // :73-74
    {
        phaseC = 0.0; phaseM = 0.0; ampEnv = 0.0; fmEnv = 0.0; pitchEnv = 0.0;
    }
    void strike(double amp, Rng&) override   // :109-115
    {
        double strikeAmp = clampd(amp, 0.0, 2.5);   // :110 clamp kept, Level + VEL → amp
        phaseC = 0.0; phaseM = 0.0;
        ampEnv = strikeAmp;
        fmEnv = 1.0;
        pitchEnv = 1.0;
    }
    double tick(Rng&) override   // :118-165
    {
        // Carrier frequency: base Tune × onset PUNCH sweep (1V/oct stripped, :122).
        // Modulator tracks the carrier scaled by RATIO.   :121-124
        double carrierFreq = tune;
        carrierFreq *= 1.0 + pitchEnv * punch * 3.0;
        double modFreq = ratio * carrierFreq;

        // Advance both phases, wrap to [0,1).   :127-130
        phaseC += carrierFreq * sampleTime;
        phaseM += modFreq * sampleTime;
        phaseC -= std::floor(phaseC);
        phaseM -= std::floor(phaseM);

        // Phase-modulation FM: the modulator's output is a phase offset IN TURNS.   :134-138
        double fmEff = clampd(shape, 0.0, 1.0);                   // FM CV term stripped
        double mod = fmEnv * fmEff * 3.0 * sinTurns(phaseM);      // 0..3 turns (~19 rad = heavy)
        double ph = phaseC + mod;
        ph -= std::floor(ph);                                     // wrap the modulated phase
        double out = ampEnv * sinTurns(ph);

        // Drive: normalized soft clip — out = tanh(dr·x)/tanh(dr).   :142-144 (×5 stripped)
        double dr = 1.0 + drive * 6.0;
        out = fastTanh(out * dr) / fastTanh(dr);

        // Advance the envelopes (the Decay-CV re-derivation :150-152 is stripped:
        // the cached coefficient is the only path).   :148-161
        if (ampEnv > 0.00001) ampEnv *= ampDecayCoeff; else ampEnv = 0.0;
        if (fmEnv    > 0.00001) fmEnv    *= fmDecayCoeff;    else fmEnv    = 0.0;
        if (pitchEnv > 0.00001) pitchEnv *= pitchDecayCoeff; else pitchEnv = 0.0;

        // NaN / blowup guard.   :164
        if (std::isnan(out) || std::fabs(out) > 1e6) out = 0.0;
        return out;
    }
};

// ── Kick Distort: kickdist.c — sine body → tanh saturation (+ sine fold) → HP → tone LP ──
// SHAPE = Drive, and FOLD follows it: fold = max(0, (shape - 0.6) / 0.4)
// (DESIGN-KITS.md §1). FIXED punch 0.5, tone 0.6 (kickdist.js:57,60).
// Stripped: Level/VEL strike (kickdist.c:109), 1V/oct PTCH (:117), Drive CV
// (:125), Tone CV (:146-148). Output ×5 (:159) → ×1 (deviation 8).
struct KickDist : Model
{
    double tune = 50.0, decay = 0.4, shape = 0.6;   // natural: Hz, s, drive 0..1
    double punch = 0.5, tone = 0.6;                 // FIXED
    double fold = 0.0;                              // derived from shape

    double phase = 0.0;                             // kickdist.c:29-39
    double ampEnv = 0.0;
    double ampDecayCoeff = 0.0;                     // cached from Decay
    double pitchEnv = 0.0;                          // punch pitch envelope (fast decay 1→0)
    double pitchEnvCoeff = 0.0;
    double hpLp = 0.0;                              // one-pole state for the DC/sub-removal highpass
    double hpCoeff = 0.0;
    double toneLp = 0.0;                            // one-pole tone lowpass state
    double toneCoeff = 0.0;                         // cached from Tone

    KickDist() { KickDist::setSampleRate(44100.0); }

    // Tone knob (0..1) → one-pole lowpass coefficient (cutoff 1..8 kHz).   :54-57
    double toneToCoeff(double toneEff) const
    {
        double fc = 1000.0 + toneEff * 7000.0;
        return clampd(2.0 * PI * fc / sampleRate, 0.02, 1.0);
    }
    void updateCoeffs()   // :61-65
    {
        double d = (decay > 0.02) ? decay : 0.02;
        ampDecayCoeff = std::exp(-(5.0 / d) * sampleTime);   // ~-43 dB over `decay` s
        toneCoeff = toneToCoeff(tone);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        pitchEnvCoeff = std::exp(-(1.0 / 0.03) * sampleTime);          // :76 — KICKDIST_PITCH_TAU, rate-aware
        hpCoeff = clampd(2.0 * PI * 25.0 / sampleRate, 0.0, 0.98);     // :77 — KICKDIST_HP_HZ, rate-aware
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; }                   // :89
    void setDecay(double v) override { decay = v; updateCoeffs(); }  // :90
    void setShape(double v) override                                 // :92 drive, :94 fold (derived)
    {
        shape = v;
        fold = fastMax(0.0, (shape - 0.6) / 0.4);
    }
    void reset() override   // :71-72
    {
        phase = 0.0; ampEnv = 0.0; pitchEnv = 0.0; hpLp = 0.0; toneLp = 0.0;
    }
    void strike(double amp, Rng&) override   // :108-113
    {
        double strikeAmp = clampd(amp, 0.0, 2.5);   // :109 clamp kept, Level + VEL → amp
        phase = 0.0;
        ampEnv = strikeAmp;
        pitchEnv = 1.0;
    }
    double tick(Rng&) override   // :116-163
    {
        // ── Sine body: base tune × onset punch sweep (up to ~4×); 1V/oct stripped. ──   :117-121
        double freq = tune * (1.0 + pitchEnv * punch * 3.0);
        phase += freq * sampleTime;
        phase -= std::floor(phase);                   // wrap to [0,1) turns
        double body = sinTurns(phase) * ampEnv;

        // ── Distortion: heavy tanh saturation is the point (Drive CV stripped). ──   :125-127
        double driveEff = clampd(shape, 0.0, 1.0);
        double gain = 1.0 + driveEff * 12.0;
        double driven = fastTanh(body * gain);

        // ── Wavefold flavour: blend toward a sin() folder (FOLD 0 = pure saturation). ──   :131-137
        double shaped = driven;
        if (fold > 0.0001) {
            double foldArg = body * gain * (0.5 + fold * 2.0);
            foldArg -= std::floor(foldArg);          // wrap → the fold itself
            double folded = sinTurns(foldArg);
            shaped = driven * (1.0 - fold) + folded * fold;
        }

        // ── Post highpass: strip DC / sub the shaping introduces (~25 Hz). ──   :140-141
        hpLp += hpCoeff * (shaped - hpLp);
        double x = shaped - hpLp;

        // ── Tone lowpass: tame the fizz (Tone CV stripped: cached coefficient only). ──   :145-153
        toneLp += toneCoeff * (x - toneLp);
        double out = toneLp;

        // Recursive-filter blowup guards.   :156-157
        if (std::isnan(hpLp)   || std::fabs(hpLp)   > 1e6) hpLp   = 0.0;
        if (std::isnan(toneLp) || std::fabs(toneLp) > 1e6) toneLp = 0.0;

        // Advance the envelopes (after the output, as the source).   :162-163
        if (ampEnv   > 0.0001) ampEnv   *= ampDecayCoeff; else ampEnv   = 0.0;
        if (pitchEnv > 0.0001) pitchEnv *= pitchEnvCoeff; else pitchEnv = 0.0;
        return out;   // ×5 (:159) stripped
    }
};

// ── Snare 808: snare808.c — two detuned bridged-T-style resonators + highpassed noise ──
// SHAPE = Snappy (noise level); FIXED tone 0.5 (resonator crossfade), snap
// 0.2 s (noise decay) (snare808.js:51,56). Stripped: Level/VEL strike
// (snare808.c:105), 1V/oct PTCH (:114-117), Tone CV (:136), Snappy CV (:144).
// Output ×3 (:147) → ×0.6 (deviation 8). Noise envelope armed at amp (deviation 9).
struct Snare808 : Model
{
    double tune = 180.0, decay = 0.15, shape = 0.6;   // natural: Hz, s, snappy 0..1
    double tone = 0.5, snap = 0.2;                    // FIXED

    double svfLow1 = 0.0, svfBand1 = 0.0, svfF1 = 0.0, svfQ1 = 0.0;   // snare808.c:30 lower (~180 Hz)
    double svfLow2 = 0.0, svfBand2 = 0.0, svfF2 = 0.0, svfQ2 = 0.0;   // :31 upper (~330 Hz)
    double noiseEnv = 0.0;                            // :33-36 highpassed-noise snappy path
    double noiseCoeff = 0.0;
    double noiseLp = 0.0;                             // one-pole highpass state
    double hpCoeff = 0.0;

    Snare808() { Snare808::setSampleRate(44100.0); }

    void updateCoeffs()   // :55-65
    {
        double f1 = tune;
        double f2 = tune * 1.83;                                     // SNARE808_RATIO (≈330/180)
        svfF1 = svfF(f1, sampleRate);                                // :58-59
        svfF2 = svfF(f2, sampleRate);
        double d = (decay > 0.01) ? decay : 0.01;
        svfQ1 = clampd(6.9 / (PI * f1 * d),         0.01, 0.9);      // :61
        svfQ2 = clampd(6.9 / (PI * f2 * (d * 0.7)), 0.01, 0.9);      // :62 upper rings shorter
        double sn = (snap > 0.01) ? snap : 0.01;
        noiseCoeff = std::exp(-(5.0 / sn) * sampleTime);             // :64
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        hpCoeff = clampd(2.0 * PI * 1500.0 / sampleRate, 0.0, 0.95);   // :77 — SNARE808_HP_HZ, rate-aware
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v;  updateCoeffs(); }   // :88
    void setDecay(double v) override { decay = v; updateCoeffs(); }   // :91
    void setShape(double v) override { shape = v; }                   // :92 (snappy)
    void reset() override   // :71-73
    {
        svfLow1 = 0.0; svfBand1 = 0.0; svfLow2 = 0.0; svfBand2 = 0.0;
        noiseEnv = 0.0; noiseLp = 0.0;
    }
    void strike(double amp, Rng&) override   // :104-109
    {
        double strikeAmp = clampd(amp, 0.0, 2.5);   // :105 clamp kept, Level + VEL → amp
        svfBand1 = strikeAmp; svfLow1 = 0.0;
        svfBand2 = strikeAmp; svfLow2 = 0.0;
        noiseEnv = amp;                             // source 1.0 (:108) — deviation 9
    }
    double tick(Rng& rng) override   // :112-147
    {
        double f1 = svfF1, f2 = svfF2;              // :119 cached (1V/oct branch stripped)

        // Two band-pass resonators ringing freely from the struck state.   :123-128
        svfLow1 += f1 * svfBand1;
        double high1 = -svfLow1 - svfQ1 * svfBand1;
        svfBand1 += f1 * high1;
        svfLow2 += f2 * svfBand2;
        double high2 = -svfLow2 - svfQ2 * svfBand2;
        svfBand2 += f2 * high2;
        // Guards.   :130-133
        if (std::isnan(svfLow1)  || std::fabs(svfLow1)  > 1e6) svfLow1  = 0.0;
        if (std::isnan(svfBand1) || std::fabs(svfBand1) > 1e6) svfBand1 = 0.0;
        if (std::isnan(svfLow2)  || std::fabs(svfLow2)  > 1e6) svfLow2  = 0.0;
        if (std::isnan(svfBand2) || std::fabs(svfBand2) > 1e6) svfBand2 = 0.0;

        // TONE crossfades the two resonators (0 = lower only, 1 = upper only).   :136-137
        double toneEff = clampd(tone, 0.0, 1.0);
        double body = (1.0 - toneEff) * svfBand1 + toneEff * svfBand2;

        // Snappy: white → VCA(snap env) → one-pole highpass → SNAPPY level.   :140-145
        if (noiseEnv > 0.0001) noiseEnv *= noiseCoeff; else noiseEnv = 0.0;
        double vca = rng.white() * noiseEnv;
        noiseLp += hpCoeff * (vca - noiseLp);
        double hp = vca - noiseLp;
        double snappyEff = clampd(shape, 0.0, 1.0);
        double noise = hp * snappyEff;

        return (body + noise) * 0.6 * 1.3;   /* x1.3 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Snare 909) */   // :147 ×3 → ×0.6 (deviation 8)
    }
};

// ── Snare Layered: snaredrum.c (GASnareDrum) — sine tone + lowpassed noise + snap burst ──
// SHAPE = Snap; FIXED noise 0.6, body 0.6 (snaredrum.js:61-64). Stripped: VEL
// (snaredrum.c:182), Tone CV (:141-144); the strike arms the three envelopes
// at `amp` where the source armed them at 1.0 (:122-124). Output ×5 (:185) → ×1
// (deviation 8). The noise lowpass keep 0.7 (:160) is a per-sample constant;
// it becomes exp(-2π·2724.8/sr), which is 0.7 at 48 kHz (deviation 5).
struct SnareLayered : Model
{
    double tune = 200.0, decay = 0.2, shape = 0.7;   // natural: Hz, s, snap 0..1
    double noise = 0.6, body = 0.6;                  // FIXED

    double phase = 0.0;                              // snaredrum.c:30-34
    double toneEnvelope = 0.0;
    double noiseEnvelope = 0.0;
    double snapEnvelope = 0.0;
    double noiseFilterState = 0.0;
    double toneCoeff = 0.0;                          // :43-45 cached from decay; snapCoeff fixed (rate 200)
    double noiseCoeff = 0.0;
    double snapCoeff = 0.0;
    double noiseLpKeep = 0.7;                        // deviation 5: :160 `filterCoeff = 0.7`

    SnareLayered() { SnareLayered::setSampleRate(44100.0); }

    void updateCoeffs()   // :59-67
    {
        double d = fastMax(0.01, decay);
        double toneDecayRate  = 15.0 / d;
        double noiseDecayRate = 8.0  / d;
        double snapDecayRate  = 200.0;   // GA: fixed fast decay for snap
        toneCoeff  = std::exp(-toneDecayRate  * sampleTime);
        noiseCoeff = std::exp(-noiseDecayRate * sampleTime);
        snapCoeff  = std::exp(-snapDecayRate  * sampleTime);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        noiseLpKeep = std::exp(-2.0 * PI * 2724.8 * sampleTime);   // = 0.7 at 48 kHz (-ln 0.7 · 48000 / 2π)
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; }                   // :98
    void setDecay(double v) override { decay = v; updateCoeffs(); }  // :102-106
    void setShape(double v) override { shape = v; }                  // :99 (snap)
    void reset() override   // :75-79
    {
        phase = 0.0; toneEnvelope = 0.0; noiseEnvelope = 0.0; snapEnvelope = 0.0; noiseFilterState = 0.0;
    }
    void strike(double amp, Rng&) override   // :121-126
    {
        toneEnvelope = amp;     // source: 1.0 (velocity plumbing stripped, deviation 8)
        noiseEnvelope = amp;
        snapEnvelope = amp;
        phase = 0.0;
    }
    double tick(Rng& rng) override   // :129-187
    {
        // Envelope decays (cached coeff, identical multiplier to GA's per-sample exp).   :130-137
        if (toneEnvelope > 0.0001) toneEnvelope *= toneCoeff; else toneEnvelope = 0.0;
        if (noiseEnvelope > 0.0001) noiseEnvelope *= noiseCoeff; else noiseEnvelope = 0.0;
        if (snapEnvelope > 0.0001) snapEnvelope *= snapCoeff; else snapEnvelope = 0.0;

        // Tone frequency (CV modulation :141-144 stripped).   :140, :145
        double currentTone = tune;
        currentTone = clampd(currentTone, 50.0, 500.0);

        // Pitch drop (characteristic snare): higher pitch at the attack.   :148-149
        double pitchDrop = 1.0 + toneEnvelope * 0.5;
        currentTone *= pitchDrop;

        // Advance phase, wrap to [0,1). GA: fastFMod (guards NaN/Inf -> 0).   :152-153
        phase += currentTone * sampleTime;
        phase = fmod01(phase);

        // Tone oscillator (sine) * its envelope.   :156
        double toneOutput = sinTurns(phase) * toneEnvelope;

        // White noise -> one-pole lowpass. GA: rand/RAND_MAX*2-1.   :159-162 (keep rate-corrected, deviation 5)
        double whiteNoise = rng.white();
        noiseFilterState = noiseLpKeep * noiseFilterState + (1.0 - noiseLpKeep) * whiteNoise;
        double noiseOutput = noiseFilterState * noiseEnvelope * noise;

        // Snap (short noise burst). NOTE: this is a SECOND rng draw this sample,
        // AFTER whiteNoise — the draw order matters.   :166-167
        double snapNoise = rng.white();
        double snapOutput = snapNoise * snapEnvelope * shape;

        // Mix.   :170 (velocity :182 and ×5 :185 stripped)
        return (toneOutput * body + noiseOutput * 0.8 + snapOutput * 0.5) * 1.2;   /* x1.2 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Snare 909) */
    }
};

// ── Snare Ring: snarering.c — ring-modulated sine pair + highpassed noise snap ──
// SHAPE = Ratio (osc B = pitch·ratio, 1..4); FIXED snappy 0.5, tone 0.5
// (snarering.js:57-58). Stripped: Level/VEL strike (snarering.c:99), 1V/oct
// PTCH (:109), Ratio CV (:110), Snappy CV (:130). Output ×4.5 (:132) → ×0.9
// (deviation 8).
struct SnareRing : Model
{
    double tune = 200.0, decay = 0.2, shape = 1.6;   // natural: Hz, s, ratio 1..4
    double snappy = 0.5, tone = 0.5;                 // FIXED

    double phaseA = 0.0, phaseB = 0.0;               // snarering.c:25-32
    double bodyEnv = 0.0;
    double bodyCoeff = 0.0;
    double noiseEnv = 0.0;
    double noiseCoeff = 0.0;
    double noiseHpLp = 0.0;                          // one-pole lowpass state; snap = white·env − lowpass
    double hpCoeff = 0.0;                            // cached from TONE (highpass cutoff)

    SnareRing() { SnareRing::setSampleRate(44100.0); }

    void updateCoeffs()   // :49-55
    {
        double d = (decay > 0.01) ? decay : 0.01;
        bodyCoeff  = std::exp(-(5.0 / d)         * sampleTime);
        noiseCoeff = std::exp(-(5.0 / (d * 0.5)) * sampleTime);   // snap is shorter than the body
        double cutoff = 500.0 + tone * 5500.0;                     // 500..6000 Hz
        hpCoeff = clampd(2.0 * PI * cutoff / sampleRate, 0.0, 0.98);
    }
    void setSampleRate(double sr) override { cacheRate(sr); updateCoeffs(); }
    void setTune(double v) override  { tune = v; }                   // :81
    void setDecay(double v) override { decay = v; updateCoeffs(); }  // :84
    void setShape(double v) override { shape = v; }                  // :82 (ratio)
    void reset() override   // :61-64
    {
        phaseA = 0.0; phaseB = 0.0; bodyEnv = 0.0; noiseEnv = 0.0; noiseHpLp = 0.0;
    }
    void strike(double amp, Rng&) override   // :98-103
    {
        double strikeAmp = clampd(amp, 0.0, 2.5);   // :99 clamp kept, Level + VEL → amp
        phaseA = 0.0; phaseB = 0.0;
        bodyEnv = strikeAmp;
        noiseEnv = strikeAmp;
    }
    double tick(Rng& rng) override   // :106-132
    {
        // ── Ring-mod body (1V/oct and Ratio CV stripped). ──   :109-119
        double ratioEff = clampd(shape, 1.0, 4.0);
        double fA = tune;
        double fB = tune * ratioEff;
        phaseA += fA * sampleTime;
        phaseB += fB * sampleTime;
        phaseA -= std::floor(phaseA);
        phaseB -= std::floor(phaseB);
        double rm = sinTurns(phaseA) * sinTurns(phaseB);   // partials at fA±fB
        if (bodyEnv > 0.0001) bodyEnv *= bodyCoeff; else bodyEnv = 0.0;
        double metallic = rm * bodyEnv;

        // ── Noise snap (highpassed white noise) ──   :122-127
        double white = rng.white();
        if (noiseEnv > 0.0001) noiseEnv *= noiseCoeff; else noiseEnv = 0.0;
        double ne = white * noiseEnv;
        noiseHpLp += hpCoeff * (ne - noiseHpLp);
        double nHi = ne - noiseHpLp;                       // highpassed (the crisp snap)
        if (std::isnan(noiseHpLp) || std::fabs(noiseHpLp) > 1e6) noiseHpLp = 0.0;

        double snappyEff = clampd(snappy, 0.0, 1.0);       // :130 (Snappy CV stripped)
        return (metallic + snappyEff * nHi) * 0.9 * 0.75;   /* x0.75 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Snare 909) */         // :132 ×4.5 → ×0.9 (deviation 8)
    }
};

// ── Snare 606: original design (no vxsynth source) — the thin, papery TR-606 snare ──
// Mechanism: two decaying sines at TUNE and TUNE×1.7 with FIXED short T60s
// (60 ms and 45 ms) stand in for the 606's two small bridged-T resonators;
// white noise through a 2-pole (two cascaded one-pole) highpass at 1.8 kHz
// and a one-pole lowpass at 9 kHz (papery, not hissy, and band-limited so
// the snap has the same power at 96 kHz as at 44.1 kHz) is the snap, under
// its own envelope whose T60 is DECAY. SHAPE (Snappy) is the noise level
// against the fixed body. Every decay is a T60 in seconds (t60Coeff), so the
// hit is the same at every sample rate. Levels: the body alone peaks ≈0.75
// and the noise at Snappy 0.7 brings the hit to ≈1.0.
struct Snare606 : Model
{
    double tune = 200.0, decay = 0.12, shape = 0.7;   // natural: Hz, s, snappy 0..1

    double phaseA = 0.0, phaseB = 0.0;               // body partials, phase in turns
    double envA = 0.0, envB = 0.0;                   // body envelopes (fixed T60 60 / 45 ms)
    double coeffA = 0.0, coeffB = 0.0;               // cached on setSampleRate
    double noiseEnv = 0.0;                           // snap envelope (T60 = DECAY)
    double noiseCoeff = 0.0;                         // cached from decay
    double hpLp1 = 0.0, hpLp2 = 0.0;                 // two one-pole highpass stages (1.8 kHz)
    double hpCoeff = 0.0;
    double topLp = 0.0;                              // one-pole lowpass (9 kHz) on the snap
    double topCoeff = 0.0;
    double noiseGain = 1.0;                          // noisePsdGain(sampleRate)

    Snare606() { Snare606::setSampleRate(44100.0); }

    void updateCoeffs() { noiseCoeff = t60Coeff(decay, sampleTime); }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        coeffA = t60Coeff(0.060, sampleTime);                        // FIXED body T60s
        coeffB = t60Coeff(0.045, sampleTime);
        hpCoeff = onePoleCoeff(1800.0, sampleRate, 0.0, 0.95);
        topCoeff = onePoleCoeff(9000.0, sampleRate, 0.02, 1.0);
        noiseGain = noisePsdGain(sampleRate);
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); }
    void reset() override
    {
        phaseA = 0.0; phaseB = 0.0; envA = 0.0; envB = 0.0; noiseEnv = 0.0;
        hpLp1 = 0.0; hpLp2 = 0.0; topLp = 0.0;
    }
    void strike(double amp, Rng&) override
    {
        double a = clampd(amp, 0.0, 2.5);
        phaseA = 0.0; phaseB = 0.0;
        envA = a; envB = a; noiseEnv = a;
    }
    double tick(Rng& rng) override
    {
        // Body: two short sine pings, the upper one weaker.
        phaseA += tune * sampleTime;       phaseA = fmod01(phaseA);
        phaseB += tune * 1.7 * sampleTime; phaseB = fmod01(phaseB);
        if (envA > 0.0001) envA *= coeffA; else envA = 0.0;
        if (envB > 0.0001) envB *= coeffB; else envB = 0.0;
        double body = envA * sinTurns(phaseA) + 0.7 * envB * sinTurns(phaseB);

        // Snap: white → VCA(noise env) → 2-pole highpass 1.8 kHz → SNAPPY.
        if (noiseEnv > 0.0001) noiseEnv *= noiseCoeff; else noiseEnv = 0.0;
        double w = rng.white() * noiseEnv * noiseGain;
        hpLp1 += hpCoeff * (w - hpLp1);
        double h1 = w - hpLp1;
        hpLp2 += hpCoeff * (h1 - hpLp2);
        double h2 = h1 - hpLp2;
        topLp += topCoeff * (h2 - topLp);
        if (std::isnan(hpLp1) || std::fabs(hpLp1) > 1e6) hpLp1 = 0.0;
        if (std::isnan(hpLp2) || std::fabs(hpLp2) > 1e6) hpLp2 = 0.0;
        if (std::isnan(topLp) || std::fabs(topLp) > 1e6) topLp = 0.0;

        return (body * 0.55 + topLp * shape * 0.5) * 1.35;   /* x1.35 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Snare 909) */
    }
};

// ── Snare Sweep: original design (no vxsynth source) — the Simmons-style 80s electronic snare ──
// Mechanism: a sine body whose pitch starts at TUNE×(1 + 3·sweep) and falls
// exponentially to TUNE, reaching 1 % of the drop after 60 + 120·sweep ms
// (ln(100)/T time constant); one amplitude envelope with T60 = DECAY drives
// the sine, the noise (white through a 2-pole lowpass at 6 kHz, at 0.6 the
// sine's level) and a 2 ms linear-fade noise click on the strike. SHAPE
// (Sweep) sets both the depth and the time of the pitch drop; at 0 the body
// is a plain decaying sine. Output ×0.7 so the hit peaks ≈1.0 at amp 1.
struct SnareSweep : Model
{
    double tune = 150.0, decay = 0.35, shape = 0.6;   // natural: Hz, s, sweep 0..1

    double phase = 0.0;                              // body phase in turns
    double ampEnv = 0.0;                             // amplitude envelope (T60 = DECAY)
    double pitchEnv = 0.0;                           // 1→0 pitch-drop envelope
    double ampCoeff = 0.0;                           // cached from decay
    double pitchCoeff = 0.0;                         // cached from shape (sweep time)
    double lp1 = 0.0, lp2 = 0.0;                     // two one-pole lowpass stages (6 kHz)
    double lpCoeff = 0.0;
    double clickSamp = 0.0;                          // 2 ms strike click
    int    clickN = 0;
    double clickLen = 1.0;                           // 2 ms in samples
    double noiseGain = 1.0;                          // noisePsdGain(sampleRate), the filtered noise only

    SnareSweep() { SnareSweep::setSampleRate(44100.0); }

    void updateCoeffs()
    {
        ampCoeff = t60Coeff(decay, sampleTime);
        double sweepTime = 0.060 + 0.120 * shape;                     // s: the drop is 99 % done by then
        pitchCoeff = std::exp(-4.605170185988092 * sampleTime / sweepTime);   // ln(100)
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        lpCoeff = onePoleCoeff(6000.0, sampleRate, 0.02, 1.0);
        noiseGain = noisePsdGain(sampleRate);
        clickLen = sampleRate * 0.002;
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); updateCoeffs(); }
    void reset() override
    {
        phase = 0.0; ampEnv = 0.0; pitchEnv = 0.0; lp1 = 0.0; lp2 = 0.0; clickSamp = 0.0; clickN = 0;
    }
    void strike(double amp, Rng& rng) override
    {
        phase = 0.0;
        ampEnv = clampd(amp, 0.0, 2.5);
        pitchEnv = 1.0;
        clickSamp = rng.white();
        clickN = (int)clickLen;
    }
    double tick(Rng& rng) override
    {
        if (ampEnv > 0.0001) ampEnv *= ampCoeff; else ampEnv = 0.0;
        if (pitchEnv > 0.0001) pitchEnv *= pitchCoeff; else pitchEnv = 0.0;

        // Body: the falling sine.
        double freq = tune * (1.0 + 3.0 * shape * pitchEnv);
        phase += freq * sampleTime;
        phase = fmod01(phase);
        double body = sinTurns(phase);

        // Noise: white → 2-pole lowpass 6 kHz, same envelope, 0.6 relative.
        double w = rng.white() * noiseGain;
        lp1 += lpCoeff * (w - lp1);
        lp2 += lpCoeff * (lp1 - lp2);
        double noise = lp2 * 0.6;

        // Click: 2 ms linear-fade noise burst on the strike.
        double click = 0.0;
        if (clickN > 0) {
            click = clickSamp * ((double)clickN / clickLen) * 0.5;
            clickN--;
            clickSamp = rng.white();
        }
        double out = (body + noise + click) * ampEnv * 0.7 * 0.55;   /* x0.55 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Snare 909) */
        if (std::isnan(lp2) || std::fabs(lp2) > 1e6) { lp1 = 0.0; lp2 = 0.0; out = 0.0; }
        return out;
    }
};

// ── Snare Gate: original design (no vxsynth source) — the 80s gated-reverb snare ──
// Mechanism: the BODY is two decaying sines at TUNE and TUNE×1.5 (FIXED T60
// 80 ms and 60 ms) plus a bright crack (white through a one-pole highpass at
// 3 kHz, T60 30 ms). The TAIL is the "room": white through a one-pole
// highpass at 1 kHz into a one-pole lowpass at 6 kHz, with a 5 ms linear
// attack, then an exponential decay whose T60 = gate × (0.5 + 3.5·hold), cut
// hard at the gate time (DECAY) by a 5 ms linear fade to zero. SHAPE (Hold)
// is how flat the tail stays before the cut. The tail sits at ≈0.7 of the
// body. A sample counter runs the gate so the cut lands at DECAY exactly.
struct SnareGate : Model
{
    double tune = 190.0, decay = 0.3, shape = 0.7;   // natural: Hz, s (gate length), hold 0..1

    double phaseA = 0.0, phaseB = 0.0;               // body partials, phase in turns
    double envA = 0.0, envB = 0.0, envCrack = 0.0;   // body envelopes (fixed T60 80 / 60 / 30 ms)
    double coeffA = 0.0, coeffB = 0.0, coeffCrack = 0.0;
    double crackLp = 0.0;                            // one-pole highpass state (3 kHz)
    double crackHpCoeff = 0.0;
    double tailEnv = 0.0;                            // tail decay envelope (T60 from gate × hold)
    double tailCoeff = 0.0;                          // cached from decay + shape
    double tailHpLp = 0.0, tailLp = 0.0;             // tail band: HP 1 kHz state, LP 6 kHz state
    double tailHpCoeff = 0.0, tailLpCoeff = 0.0;
    int    pos = -1;                                 // samples since the strike (-1: tail idle)
    int    attackN = 1, gateN = 1, fadeN = 1;        // 5 ms, DECAY, 5 ms in samples
    double noiseGain = 1.0;                          // noisePsdGain(sampleRate)

    SnareGate() { SnareGate::setSampleRate(44100.0); }

    void updateCoeffs()
    {
        double t60 = decay * (0.5 + 3.5 * shape);
        tailCoeff = t60Coeff(t60, sampleTime);
        gateN = (int)(decay * sampleRate);
        if (gateN < attackN) gateN = attackN;
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        coeffA     = t60Coeff(0.080, sampleTime);                    // FIXED body T60s
        coeffB     = t60Coeff(0.060, sampleTime);
        coeffCrack = t60Coeff(0.030, sampleTime);
        crackHpCoeff = onePoleCoeff(3000.0, sampleRate, 0.0, 0.95);
        tailHpCoeff  = onePoleCoeff(1000.0, sampleRate, 0.0, 0.95);
        tailLpCoeff  = onePoleCoeff(6000.0, sampleRate, 0.02, 1.0);
        noiseGain = noisePsdGain(sampleRate);
        attackN = (int)(sampleRate * 0.005); if (attackN < 1) attackN = 1;
        fadeN   = (int)(sampleRate * 0.005); if (fadeN   < 1) fadeN   = 1;
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); updateCoeffs(); }
    void reset() override
    {
        phaseA = 0.0; phaseB = 0.0; envA = 0.0; envB = 0.0; envCrack = 0.0; crackLp = 0.0;
        tailEnv = 0.0; tailHpLp = 0.0; tailLp = 0.0; pos = -1;
    }
    void strike(double amp, Rng&) override
    {
        double a = clampd(amp, 0.0, 2.5);
        phaseA = 0.0; phaseB = 0.0;
        envA = a; envB = a; envCrack = a;
        tailEnv = a;
        pos = 0;
    }
    double tick(Rng& rng) override
    {
        // Body: two sine pings + the highpassed crack.
        phaseA += tune * sampleTime;       phaseA = fmod01(phaseA);
        phaseB += tune * 1.5 * sampleTime; phaseB = fmod01(phaseB);
        if (envA > 0.0001) envA *= coeffA; else envA = 0.0;
        if (envB > 0.0001) envB *= coeffB; else envB = 0.0;
        if (envCrack > 0.0001) envCrack *= coeffCrack; else envCrack = 0.0;
        double body = envA * sinTurns(phaseA) + 0.5 * envB * sinTurns(phaseB);
        double crackIn = rng.white() * envCrack * noiseGain;
        crackLp += crackHpCoeff * (crackIn - crackLp);
        double crack = crackIn - crackLp;
        if (std::isnan(crackLp) || std::fabs(crackLp) > 1e6) crackLp = 0.0;

        // Tail: the gate. Attack ramp, decay, then the 5 ms cut at gateN.
        double tail = 0.0;
        if (pos >= 0) {
            double gain;
            if (pos < attackN)               gain = (double)pos / attackN;
            else if (pos < gateN)            gain = 1.0;
            else if (pos < gateN + fadeN)    gain = 1.0 - (double)(pos - gateN) / fadeN;
            else                             gain = 0.0;
            if (tailEnv > 0.0001) tailEnv *= tailCoeff; else tailEnv = 0.0;
            double x = rng.white() * tailEnv * gain * noiseGain;
            tailHpLp += tailHpCoeff * (x - tailHpLp);
            double h = x - tailHpLp;
            tailLp += tailLpCoeff * (h - tailLp);
            tail = tailLp;
            if (std::isnan(tailHpLp) || std::fabs(tailHpLp) > 1e6) tailHpLp = 0.0;
            if (std::isnan(tailLp)   || std::fabs(tailLp)   > 1e6) { tailLp = 0.0; tail = 0.0; }
            pos++;
            if (pos >= gateN + fadeN || tailEnv <= 0.0) {   // gate closed: the tail is over
                pos = -1; tailEnv = 0.0; tailHpLp = 0.0; tailLp = 0.0;
            }
        }
        return (body * 0.6 + crack * 0.4 + tail * 0.7) * 0.75;   /* x0.75 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Snare 909) */
    }
};

// ── Clap 909: original design (no vxsynth source) — the TR-909's two-path clap ──
// Mechanism: a 31-stage LFSR (taps 31 and 13, XOR feedback, the 909's digital
// noise) clocked at a FIXED 48 kHz through a fractional accumulator, so the
// noise is the same signal at every sample rate (held between clocks at 96
// kHz) and its band-limited power needs no rate correction; it is NEVER
// reset, so every hit lands on a different part of the sequence. The noise
// runs through a Chamberlin bandpass at TUNE (damping 0.6) and then into two
// VCAs: path A is a train of sawtooth bursts (instant rise, exponential fall
// with τ = 0.3 × the spacing) and path B a slower attack-release tail (3 ms
// linear attack, T60 = DECAY) at 0.5 of the burst level. SHAPE (Density)
// interpolates the train from three bursts 8 ms apart to five 14 ms apart.
struct Clap909 : Model
{
    double tune = 1100.0, decay = 0.25, shape = 0.5;   // natural: Hz, s, density 0..1

    uint32_t lfsr = 0x2545F491u;     // 31-bit shift register, non-zero, never reset
    double lfsrPhase = 0.0;          // fractional clock accumulator (48 kHz against sampleRate)
    double lfsrStep = 1.0;           // 48000 × sampleTime
    double noiseHeld = 0.0;          // the LFSR output bit as ±1, held between clocks
    double low = 0.0, band = 0.0;    // Chamberlin BP state
    double f = 0.0;                  // cached from tune
    int    burstsTotal = 4;          // from shape: round(3 + 2·density)
    int    spacingN = 1;             // from shape: (8 + 6·density) ms in samples
    double burstCoeff = 0.0;         // burst fall, τ = 0.3 × spacing
    int    burstsLeft = 0;
    int    seqCounter = 0;           // samples since the last burst
    double burstEnv = 0.0;
    double tailEnv = 0.0;            // attack-release tail envelope
    double tailTarget = 0.0;         // strike amplitude the attack ramps to
    double tailCoeff = 0.0;          // release, T60 = DECAY
    double attackStep = 1.0;         // per-sample linear attack increment (3 ms)
    bool   attacking = false;
    double amp = 0.0;

    Clap909() { Clap909::setSampleRate(44100.0); }

    static uint32_t lfsrNext(uint32_t s)   // taps 31 and 13 → maximal length 2^31 − 1
    {
        uint32_t bit = ((s >> 30) ^ (s >> 12)) & 1u;
        return ((s << 1) | bit) & 0x7FFFFFFFu;
    }
    void updateCoeffs()
    {
        f = svfF(tune, sampleRate);
        tailCoeff = t60Coeff(decay, sampleTime);
        double d = clampd(shape, 0.0, 1.0);
        burstsTotal = (int)std::floor(3.0 + 2.0 * d + 0.5);
        double spacing = 0.008 + 0.006 * d;                     // s
        spacingN = (int)(spacing * sampleRate); if (spacingN < 1) spacingN = 1;
        burstCoeff = std::exp(-sampleTime / (0.3 * spacing));
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        lfsrStep = 48000.0 * sampleTime;
        attackStep = 1.0 / (sampleRate * 0.003);
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; updateCoeffs(); }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); updateCoeffs(); }
    void reset() override   // silences; the LFSR keeps running by design
    {
        low = 0.0; band = 0.0; burstsLeft = 0; seqCounter = 0; burstEnv = 0.0;
        tailEnv = 0.0; tailTarget = 0.0; attacking = false; amp = 0.0;
    }
    void strike(double a, Rng&) override
    {
        amp = clampd(a, 0.0, 2.5);
        burstsLeft = burstsTotal;
        seqCounter = spacingN;        // first burst fires immediately in tick
        tailTarget = amp;
        attacking = true;
    }
    double tick(Rng&) override
    {
        // Digital noise at a fixed 48 kHz clock, whatever the host rate.
        lfsrPhase += lfsrStep;
        while (lfsrPhase >= 1.0) {
            lfsrPhase -= 1.0;
            lfsr = lfsrNext(lfsr);
            if (lfsr == 0u) lfsr = 0x2545F491u;
            noiseHeld = (lfsr & 1u) ? 1.0 : -1.0;
        }
        // Path A: the burst train.
        if (burstsLeft > 0) {
            seqCounter++;
            if (seqCounter >= spacingN) { seqCounter = 0; burstsLeft--; burstEnv = amp; }
        }
        burstEnv *= burstCoeff;
        if (burstEnv < 0.0001) burstEnv = 0.0;
        // Path B: attack-release tail.
        if (attacking) {
            tailEnv += tailTarget * attackStep;
            if (tailEnv >= tailTarget) { tailEnv = tailTarget; attacking = false; }
        } else if (tailEnv > 0.0001) tailEnv *= tailCoeff; else tailEnv = 0.0;

        double vca = burstEnv + 0.5 * tailEnv;
        if (vca <= 0.0) { low = 0.0; band = 0.0; return 0.0; }
        low += f * band;
        double high = noiseHeld - low - 0.6 * band;
        band += f * high;
        if (std::isnan(band) || std::fabs(band) > 1e6) { band = 0.0; low = 0.0; }
        return band * vca * 1.55;   /* x1.55 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Clap 808) */
    }
};

// ── Clap Trap: original design (no vxsynth source) — the Simmons Claptrap ──
// Mechanism: six short clicks (white noise under a τ 2 ms exponential
// envelope) excite a RESONANT Chamberlin bandpass at TUNE (damping 0.15,
// much sharper than the 808's 0.6), so each click rings the filter; the
// clicks' spacing (nominal 9 ms) and level are drawn fresh from the Rng on
// every strike, SHAPE (Humanize) scaling the spread from none to ±60 % on
// each gap and ±50 % on each level. A "reverb" path — white noise through a
// one-pole highpass at 800 Hz and a one-pole lowpass at 7 kHz under an
// envelope with T60 = DECAY — sits under the clicks. The noise is scaled by
// noisePsdGain so the filtered bands have the same power at every rate.
struct ClapTrap : Model
{
    double tune = 1600.0, decay = 0.2, shape = 0.5;   // natural: Hz, s, humanize 0..1

    enum { CLICKS = 6 };
    int    gapN[CLICKS] = {};        // samples between clicks, drawn per strike (gapN[0] unused)
    double lvl[CLICKS] = {};         // per-click level, drawn per strike
    int    clickIndex = CLICKS;      // next click to fire (CLICKS: train done)
    int    seqCounter = 0;           // samples since the last click
    double clickEnv = 0.0;
    double clickCoeff = 0.0;         // τ 2 ms
    double low = 0.0, band = 0.0;    // resonant Chamberlin BP state
    double f = 0.0;                  // cached from tune
    double verbEnv = 0.0;            // reverb envelope (T60 = DECAY)
    double verbCoeff = 0.0;
    double verbHpLp = 0.0;           // one-pole highpass state (800 Hz)
    double verbHpCoeff = 0.0;
    double verbLp = 0.0;             // one-pole lowpass state (7 kHz): the room is not hiss
    double verbLpCoeff = 0.0;
    double noiseGain = 1.0;          // noisePsdGain(sampleRate)
    double amp = 0.0;

    ClapTrap() { ClapTrap::setSampleRate(44100.0); }

    void updateCoeffs()
    {
        f = svfF(tune, sampleRate);
        verbCoeff = t60Coeff(decay, sampleTime);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        clickCoeff = std::exp(-sampleTime / 0.002);
        verbHpCoeff = onePoleCoeff(800.0, sampleRate, 0.0, 0.95);
        verbLpCoeff = onePoleCoeff(7000.0, sampleRate, 0.02, 1.0);
        noiseGain = noisePsdGain(sampleRate);
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; updateCoeffs(); }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); }
    void reset() override
    {
        clickIndex = CLICKS; seqCounter = 0; clickEnv = 0.0; low = 0.0; band = 0.0;
        verbEnv = 0.0; verbHpLp = 0.0; verbLp = 0.0; amp = 0.0;
    }
    void strike(double a, Rng& rng) override
    {
        amp = clampd(a, 0.0, 2.5);
        for (int i = 0; i < CLICKS; i++) {   // fresh timing and levels for this hit
            double gap = 0.009 * (1.0 + 0.6 * shape * rng.white());
            gapN[i] = (int)(gap * sampleRate); if (gapN[i] < 1) gapN[i] = 1;
            lvl[i] = clampd(1.0 + 0.5 * shape * rng.white(), 0.1, 1.5);
        }
        clickIndex = 0;
        seqCounter = gapN[0];         // the first click fires immediately in tick
        verbEnv = amp;
    }
    double tick(Rng& rng) override
    {
        if (clickIndex < CLICKS) {
            seqCounter++;
            if (seqCounter >= gapN[clickIndex]) {
                seqCounter = 0;
                clickEnv = amp * lvl[clickIndex];
                clickIndex++;
            }
        }
        clickEnv *= clickCoeff;
        if (clickEnv < 0.0001) clickEnv = 0.0;
        if (verbEnv > 0.0001) verbEnv *= verbCoeff; else verbEnv = 0.0;
        if (clickEnv <= 0.0 && verbEnv <= 0.0 && std::fabs(band) < 1e-4) { low = 0.0; band = 0.0; return 0.0; }

        // Clicks into the resonant band (VCA before the filter: the filter rings).
        double in = rng.white() * noiseGain * clickEnv;
        low += f * band;
        double high = in - low - 0.15 * band;
        band += f * high;
        if (std::isnan(band) || std::fabs(band) > 1e6) { band = 0.0; low = 0.0; }

        // Reverb: highpassed noise, T60 = DECAY.
        double v = rng.white() * noiseGain * verbEnv;
        verbHpLp += verbHpCoeff * (v - verbHpLp);
        verbLp += verbLpCoeff * ((v - verbHpLp) - verbLp);
        double verb = verbLp;
        if (std::isnan(verbHpLp) || std::fabs(verbHpLp) > 1e6) verbHpLp = 0.0;
        if (std::isnan(verbLp)   || std::fabs(verbLp)   > 1e6) { verbLp = 0.0; verb = 0.0; }

        return (band + verb * 0.87) * 1.5;   /* x1.5 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Clap 808) */
    }
};

// ── Clap Lo-Fi: original design (no vxsynth source) — the Linn / TR-707 sampled-clap character ──
// Mechanism: one dense burst (three sub-bursts 4 ms apart, τ 1.5 ms) plus a
// short tail (T60 = DECAY, 0.6 relative) excite white noise through a
// Chamberlin bandpass at TUNE (damping 0.4), exactly the 808 topology but
// tighter; then the OUTPUT goes through a deliberate sample-rate reduction
// and bit-depth quantisation: hold-and-quantise with no interpolation, so
// the aliasing is part of the sound. SHAPE (Crunch) runs the hold rate from
// 28 kHz to 11 kHz and the depth from 12 to 6 bits. The hold is a fractional
// phase accumulator against the real sample rate, so 44.1 / 48 / 96 kHz all
// give the same effective rate. Noise scaled by noisePsdGain (rate-flat band).
struct ClapLoFi : Model
{
    double tune = 1400.0, decay = 0.15, shape = 0.6;   // natural: Hz, s, crunch 0..1

    int    spacingN = 1;             // 4 ms in samples
    int    burstsLeft = 0;
    int    seqCounter = 0;
    double burstEnv = 0.0;
    double burstCoeff = 0.0;         // τ 1.5 ms
    double tailEnv = 0.0;
    double tailCoeff = 0.0;          // T60 = DECAY
    double low = 0.0, band = 0.0;    // Chamberlin BP state
    double f = 0.0;                  // cached from tune
    double holdRate = 17800.0;       // Hz, from shape: 28000 − 17000·crunch
    double holdStep = 0.0;           // holdRate × sampleTime
    double holdPhase = 1.0;          // fractional accumulator (starts ready to sample)
    double qStep = 0.0;              // quantiser step: 2 / 2^bits, bits = 12 − 6·crunch
    double held = 0.0;               // the held, quantised output
    double noiseGain = 1.0;          // noisePsdGain(sampleRate)
    double amp = 0.0;

    ClapLoFi() { ClapLoFi::setSampleRate(44100.0); }

    void updateCoeffs()
    {
        f = svfF(tune, sampleRate);
        tailCoeff = t60Coeff(decay, sampleTime);
        double c = clampd(shape, 0.0, 1.0);
        holdRate = 28000.0 - 17000.0 * c;
        holdStep = holdRate * sampleTime;
        double bits = 12.0 - 6.0 * c;
        qStep = 2.0 / std::exp2(bits);
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        spacingN = (int)(sampleRate * 0.004); if (spacingN < 1) spacingN = 1;
        burstCoeff = std::exp(-sampleTime / 0.0015);
        noiseGain = noisePsdGain(sampleRate);
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; updateCoeffs(); }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); updateCoeffs(); }
    void reset() override
    {
        burstsLeft = 0; seqCounter = 0; burstEnv = 0.0; tailEnv = 0.0;
        low = 0.0; band = 0.0; holdPhase = 1.0; held = 0.0; amp = 0.0;
    }
    void strike(double a, Rng&) override
    {
        amp = clampd(a, 0.0, 2.5);
        burstsLeft = 3;
        seqCounter = spacingN;        // first sub-burst fires immediately in tick
        tailEnv = amp;
    }
    double tick(Rng& rng) override
    {
        if (burstsLeft > 0) {
            seqCounter++;
            if (seqCounter >= spacingN) { seqCounter = 0; burstsLeft--; burstEnv = amp; }
        }
        burstEnv *= burstCoeff;
        if (burstEnv < 0.0001) burstEnv = 0.0;
        if (tailEnv > 0.0001) tailEnv *= tailCoeff; else tailEnv = 0.0;

        double excite = burstEnv + 0.6 * tailEnv;
        double clean = 0.0;
        if (excite <= 0.0) { low = 0.0; band = 0.0; }
        else {
            double in = rng.white() * noiseGain * excite;
            low += f * band;
            double high = in - low - 0.4 * band;
            band += f * high;
            if (std::isnan(band) || std::fabs(band) > 1e6) { band = 0.0; low = 0.0; }
            clean = band * 1.6;
        }

        // The lo-fi stage: hold at holdRate, quantise to qStep, no interpolation.
        holdPhase += holdStep;
        if (holdPhase >= 1.0) {
            holdPhase -= 1.0;
            if (holdPhase >= 1.0) holdPhase = 0.0;   // holdRate above sampleRate: never more than once
            double x = clampd(clean, -1.0, 1.0);
            held = std::floor(x / qStep + 0.5) * qStep;
        }
        return held * 1.65;   /* x1.65 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Clap 808) */
    }
};

// ── Clap Gate: original design (no vxsynth source) — the big 80s gated clap ──
// Mechanism: the 808 burst train (three bursts 10 ms apart, τ 3 ms) excites
// white noise through a Chamberlin bandpass at TUNE (damping 0.6); under it
// the "room": white noise through a one-pole highpass at 1 kHz into a
// one-pole lowpass at 6 kHz, with a 5 ms linear attack, an exponential decay
// whose T60 = gate × (0.5 + 3.5·hold), cut hard at the gate time (DECAY) by
// a 5 ms linear fade — exactly Snare Gate's tail discipline. SHAPE (Hold) is
// how flat the tail stays before the cut. A sample counter runs the gate so
// the cut lands at DECAY exactly. Noise scaled by noisePsdGain (rate-flat).
struct ClapGate : Model
{
    double tune = 1000.0, decay = 0.35, shape = 0.7;   // natural: Hz, s (gate length), hold 0..1

    int    spacingN = 1;             // 10 ms in samples
    int    burstsLeft = 0;
    int    seqCounter = 0;
    double burstEnv = 0.0;
    double burstCoeff = 0.0;         // τ 3 ms
    double low = 0.0, band = 0.0;    // Chamberlin BP state
    double f = 0.0;                  // cached from tune
    double tailEnv = 0.0;            // tail decay envelope (T60 from gate × hold)
    double tailCoeff = 0.0;          // cached from decay + shape
    double tailHpLp = 0.0, tailLp = 0.0;   // tail band: HP 1 kHz state, LP 6 kHz state
    double tailHpCoeff = 0.0, tailLpCoeff = 0.0;
    int    pos = -1;                 // samples since the strike (-1: tail idle)
    int    attackN = 1, gateN = 1, fadeN = 1;   // 5 ms, DECAY, 5 ms in samples
    double noiseGain = 1.0;          // noisePsdGain(sampleRate)
    double amp = 0.0;

    ClapGate() { ClapGate::setSampleRate(44100.0); }

    void updateCoeffs()
    {
        f = svfF(tune, sampleRate);
        double t60 = decay * (0.5 + 3.5 * shape);
        tailCoeff = t60Coeff(t60, sampleTime);
        gateN = (int)(decay * sampleRate);
        if (gateN < attackN) gateN = attackN;
    }
    void setSampleRate(double sr) override
    {
        cacheRate(sr);
        spacingN = (int)(sampleRate * 0.010); if (spacingN < 1) spacingN = 1;
        burstCoeff = std::exp(-sampleTime / 0.003);
        tailHpCoeff = onePoleCoeff(1000.0, sampleRate, 0.0, 0.95);
        tailLpCoeff = onePoleCoeff(6000.0, sampleRate, 0.02, 1.0);
        noiseGain = noisePsdGain(sampleRate);
        attackN = (int)(sampleRate * 0.005); if (attackN < 1) attackN = 1;
        fadeN   = (int)(sampleRate * 0.005); if (fadeN   < 1) fadeN   = 1;
        updateCoeffs();
    }
    void setTune(double v) override  { tune = v; updateCoeffs(); }
    void setDecay(double v) override { decay = v; updateCoeffs(); }
    void setShape(double v) override { shape = clampd(v, 0.0, 1.0); updateCoeffs(); }
    void reset() override
    {
        burstsLeft = 0; seqCounter = 0; burstEnv = 0.0; low = 0.0; band = 0.0;
        tailEnv = 0.0; tailHpLp = 0.0; tailLp = 0.0; pos = -1; amp = 0.0;
    }
    void strike(double a, Rng&) override
    {
        amp = clampd(a, 0.0, 2.5);
        burstsLeft = 3;
        seqCounter = spacingN;        // first burst fires immediately in tick
        tailEnv = amp;
        pos = 0;
    }
    double tick(Rng& rng) override
    {
        // The hands: the 808 burst train into the band.
        if (burstsLeft > 0) {
            seqCounter++;
            if (seqCounter >= spacingN) { seqCounter = 0; burstsLeft--; burstEnv = amp; }
        }
        burstEnv *= burstCoeff;
        if (burstEnv < 0.0001) burstEnv = 0.0;
        double hands = 0.0;
        if (burstEnv <= 0.0) { low = 0.0; band = 0.0; }
        else {
            double in = rng.white() * noiseGain * burstEnv;
            low += f * band;
            double high = in - low - 0.6 * band;
            band += f * high;
            if (std::isnan(band) || std::fabs(band) > 1e6) { band = 0.0; low = 0.0; }
            hands = band * 1.6;
        }

        // The room: the gate. Attack ramp, decay, then the 5 ms cut at gateN.
        double tail = 0.0;
        if (pos >= 0) {
            double gain;
            if (pos < attackN)               gain = (double)pos / attackN;
            else if (pos < gateN)            gain = 1.0;
            else if (pos < gateN + fadeN)    gain = 1.0 - (double)(pos - gateN) / fadeN;
            else                             gain = 0.0;
            if (tailEnv > 0.0001) tailEnv *= tailCoeff; else tailEnv = 0.0;
            double x = rng.white() * tailEnv * gain * noiseGain;
            tailHpLp += tailHpCoeff * (x - tailHpLp);
            double h = x - tailHpLp;
            tailLp += tailLpCoeff * (h - tailLp);
            tail = tailLp;
            if (std::isnan(tailHpLp) || std::fabs(tailHpLp) > 1e6) tailHpLp = 0.0;
            if (std::isnan(tailLp)   || std::fabs(tailLp)   > 1e6) { tailLp = 0.0; tail = 0.0; }
            pos++;
            if (pos >= gateN + fadeN || tailEnv <= 0.0) {   // gate closed: the tail is over
                pos = -1; tailEnv = 0.0; tailHpLp = 0.0; tailLp = 0.0;
            }
        }
        return (hands + tail * 0.8) * 1.4;   /* x1.4 loudness trim (survey 2026-09-02: 80 ms ear-weighted RMS matched to Clap 808) */
    }
};

// ── One column: every concrete model as a member, plus the active pointer ──
// Shared kernels are configured, not duplicated: Ping808 flips `rimshot`,
// SineKick takes its fixed `click`, Hat is the same object for both hats.
struct ModelBank
{
    Ping808      ping;
    SineKick     sine;
    KickFm       fm;
    KickDist     dist;
    Snare909     s909;
    Snare808     s808;
    SnareLayered layered;
    SnareRing    ring;
    Snare606     s606;
    SnareSweep   sweep;
    SnareGate    gate;
    Clap         clap;
    Clap909      clap909;
    ClapTrap     clapTrap;
    ClapLoFi     clapLoFi;
    ClapGate     clapGate;
    Hat          hat;

    ModelId id = MODEL_KICK_808;
    Model* active = &ping;

    // Which member serves a ModelId, and its fixed-value configuration.
    Model* select(ModelId which)
    {
        switch (which) {
            case MODEL_KICK_808:      ping.rimshot = false; return &ping;
            case MODEL_RIMSHOT:       ping.rimshot = true;  return &ping;
            case MODEL_KICK_SINE:     sine.click = 0.5;     return &sine;   // kickdrum.js:53
            case MODEL_TOM:           sine.click = 0.1;     return &sine;   // DESIGN-KITS.md §1
            case MODEL_KICK_FM:       return &fm;
            case MODEL_KICK_DIST:     return &dist;
            case MODEL_SNARE_909:     return &s909;
            case MODEL_SNARE_808:     return &s808;
            case MODEL_SNARE_LAYERED: return &layered;
            case MODEL_SNARE_RING:    return &ring;
            case MODEL_SNARE_606:     return &s606;
            case MODEL_SNARE_SWEEP:   return &sweep;
            case MODEL_SNARE_GATE:    return &gate;
            case MODEL_CLAP:          return &clap;
            case MODEL_CLAP_909:      return &clap909;
            case MODEL_CLAP_TRAP:     return &clapTrap;
            case MODEL_CLAP_LOFI:     return &clapLoFi;
            case MODEL_CLAP_GATE:     return &clapGate;
            case MODEL_HAT_CLOSED:    return &hat;
            case MODEL_HAT_OPEN:      return &hat;
            default:                  ping.rimshot = false; return &ping;
        }
    }

    void setSampleRate(double sr)
    {
        ping.setSampleRate(sr);  sine.setSampleRate(sr);    fm.setSampleRate(sr);
        dist.setSampleRate(sr);  s909.setSampleRate(sr);    s808.setSampleRate(sr);
        layered.setSampleRate(sr); ring.setSampleRate(sr);  clap.setSampleRate(sr);
        hat.setSampleRate(sr);   s606.setSampleRate(sr);    sweep.setSampleRate(sr);
        gate.setSampleRate(sr);  clap909.setSampleRate(sr); clapTrap.setSampleRate(sr);
        clapLoFi.setSampleRate(sr); clapGate.setSampleRate(sr);
    }
};

// ── The kit facade (DESIGN-KITS.md §3, binding) ──────────────────────────
struct Voices
{
    double sampleRate = 44100.0;
    double sampleTime = 1.0 / 44100.0;
    Rng rng;                          // deviation 4: one xorshift32 for every column

    ModelBank bank[COLUMNS];
    double v01[COLUMNS][3] = {};      // stored knob positions: [c][0 tune, 1 decay, 2 shape]
    double level[COLUMNS] = { 0.9, 0.8, 0.75, 0.7, 0.7, 0.65 };   // vxdrumvoices.c:104-112 defaults
    double m_accent = 0.5, m_drive = 0.15, m_volume = 0.8;         // :308-314
    bool m_panning = true;   // the Machine's fixed per-voice pans on the mix; off = every voice dead centre

    // Every coefficient is valid from construction (the create path at
    // :326-327 does the same): the House kit at each model's default knobs.
    Voices()
    {
        setSampleRate(44100.0);
        const KitSpec& house = kitSpec(KIT_HOUSE);
        for (int c = 0; c < COLUMNS; c++) {
            const ModelSpec& ms = modelSpec(house.models[c]);
            v01[c][0] = knobDefault01(ms.tune);
            v01[c][1] = knobDefault01(ms.decay);
            v01[c][2] = knobDefault01(ms.shape);
            setModel(c, house.models[c]);
        }
    }

    // ── sample rate: every model, every column; resets nothing else (voice
    // state rings on across a rate change).
    void setSampleRate(double sr)
    {
        if (!(sr > 0.0)) sr = 44100.0;   // the Machine's guard (vxdrums.c:459)
        sampleRate = sr;
        sampleTime = 1.0 / sr;
        for (int c = 0; c < COLUMNS; c++) bank[c].setSampleRate(sr);
    }

    void setSeed(uint32_t seed) { rng.seed(seed); }

    // Audio-thread safe (no allocation): swaps the column's active model,
    // silences it, and re-pushes the column's three stored knobs.
    void setModel(int column, ModelId id)
    {
        if (column < 0 || column >= COLUMNS) return;
        ModelBank& b = bank[column];
        b.id = id;
        b.active = b.select(id);
        b.active->reset();
        pushKnobs(column);
    }
    ModelId model(int column) const
    {
        if (column < 0 || column >= COLUMNS) return MODEL_KICK_808;
        return bank[column].id;
    }

    // Normalized knobs; the active model maps each to its natural range and
    // recomputes coefficients.
    void setTune(int column, double v)
    {
        if (column < 0 || column >= COLUMNS) return;
        v01[column][0] = v;
        bank[column].active->setTune(knobNatural(modelSpec(bank[column].id).tune, v));
    }
    void setDecay(int column, double v)
    {
        if (column < 0 || column >= COLUMNS) return;
        v01[column][1] = v;
        bank[column].active->setDecay(knobNatural(modelSpec(bank[column].id).decay, v));
    }
    void setShape(int column, double v)
    {
        if (column < 0 || column >= COLUMNS) return;
        v01[column][2] = v;
        bank[column].active->setShape(knobNatural(modelSpec(bank[column].id).shape, v));
    }
    void setLevel(int column, double v)   // 0..1.2 gain at the mix (unchanged semantics)
    {
        if (column < 0 || column >= COLUMNS) return;
        level[column] = v;
    }

    void setAccent(double v) { m_accent = clampd(v, 0.0, 1.0); }   // vxdrums.c:576 (deviation 6)
    void setDrive(double v)  { m_drive = clampd(v, 0.0, 1.0); }    // vxdrums.c:578
    void setVolume(double v) { m_volume = v; }
    void setPanning(bool on) { m_panning = on; }

    // Strike ONE column at amplitude `amp` (accent already applied) — the
    // single path every trigger takes. Column rule: a strike on column 4
    // chokes column 5's active model (only the hats do anything).   :276-291
    void strikeLane(int lane, double amp)
    {
        if (lane < 0 || lane >= COLUMNS) return;
        bank[lane].active->strike(amp, rng);
        if (lane == LANE_CH) bank[LANE_OH].active->choke();   // :284-286
    }

    // ── one sample: tick every column, then the outs and the mix   :383-431 ──
    // Strikes for THIS sample must already have been delivered through
    // strikeLane() (the source fires before it ticks, :368-381, so a strike in
    // sample n is heard in sample n). `out` is in VOLTS (the source's ×5):
    // [mixL, mixR, c0..c5].
    void process(double out[OUTPUTS])
    {
        double v[COLUMNS] = {};
        for (int c = 0; c < COLUMNS; c++) {
            // Individual outs: dry, post level + accent (the accent rode the
            // strike), pre drive/volume.   :411-420
            v[c] = bank[c].active->tick(rng) * level[c];
            out[2 + c] = v[c] * 5.0;
        }

        // The mix: the Machine's fixed pans → one-knob normalized tanh drive → volume.   :422-430
        // With panning off every voice takes the centre gain (0.50 each side), so a
        // centred voice is identical in both modes and the total energy per voice
        // is unchanged (each voice's pair sums to 1.0 either way).
        double mixL, mixR;
        if (m_panning) {
            mixL = v[0] * 0.50 + v[1] * 0.50 + v[2] * 0.62 + v[3] * 0.38 + v[4] * 0.42 + v[5] * 0.58;
            mixR = v[0] * 0.50 + v[1] * 0.50 + v[2] * 0.38 + v[3] * 0.62 + v[4] * 0.58 + v[5] * 0.42;
        } else {
            mixL = mixR = (v[0] + v[1] + v[2] + v[3] + v[4] + v[5]) * 0.50;
        }
        double dr = 1.0 + m_drive * 5.0;
        double tdr = fastTanh(dr);
        mixL = fastTanh(mixL * dr) / tdr;
        mixR = fastTanh(mixR * dr) / tdr;
        out[0] = mixL * m_volume * 5.0;
        out[1] = mixR * m_volume * 5.0;
    }

private:
    void pushKnobs(int column)
    {
        const ModelSpec& ms = modelSpec(bank[column].id);
        Model* m = bank[column].active;
        m->setTune(knobNatural(ms.tune,   v01[column][0]));
        m->setDecay(knobNatural(ms.decay, v01[column][1]));
        m->setShape(knobNatural(ms.shape, v01[column][2]));
    }
};

} // namespace vx_drums
