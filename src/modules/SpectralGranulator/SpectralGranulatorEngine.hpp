#pragma once
//
// Spectral Granulator — the engine (2026-11-21). Rack-free: the module
// (SpectralGranulator.hpp) and the listening lab (tests/spectral_granulator/lab.cpp) drive
// the same code, so what the lab renders is what the module plays.
//
// The idea: an FFT listens to the input and reduces it to four signals, and
// those signals steer a granular smear of the same input.
//
//   ANALYSIS (mono, every 256 samples: one 1024-point FFT, and one
//   2048-point FFT for TONAL)
//     ONSET   a trigger when the spectrum changes suddenly (spectral flux
//             over an adaptive threshold: a multiple of the flux's running
//             upper quartile; SENS moves the multiple)
//     LEVEL   energy, in dB, mapped 0..1
//     BRIGHT  where the energy sits, low to high (log-frequency centroid)
//     TONAL   pitched vs noisy (flatness of the spectrum after its broad
//             shape is divided out, so a tilted noise is still noise). It
//             has its own 2048-point FFT: at 1024 the harmonics of a voice
//             near 100 Hz are 2 bins apart, unresolved, and read as noise.
//     FOCUS / WIDTH choose the part of the spectrum all four listen to.
//     SMOOTH slews the three continuous signals.
//
//   GRAINS — the Sample Tracker's Granular Smear (voxglitch_devices,
//   src/SampleTracker/fx/GranularSmear.hpp, 2026-09-10), with its bundled
//   models split into separate controls:
//     STRETCH a read head falls behind real time by (1 - 1/stretch) samples
//             per sample; grains spawned around it re-read overlapping
//             source. That is the time stretch.
//     GRAIN   grain length. DENSITY is how many overlap.
//     PITCH   a set of intervals the grains cycle through IN ORDER (it reads
//             as a machine, not a cloud). TUNE transposes the whole set.
//     JITTER  loosens the machine into a cloud: each grain reads from a
//             random distance further back, and the spawn times wander.
//     SPREAD  how far left and right the grains are panned.
//     FEEDBACK the grain output is written back into the grain buffer
//             (through a DC blocker and a soft limiter), so smears
//             regenerate. Past 100 % it builds until the limiter holds it.
//     STEP    the head the grains use is a copy of the read head taken every
//             STEP seconds; between copies every grain re-reads the same
//             region. 0 = continuous.
//     RESYNC  the read head jumps back to the present. It always does so
//             when the lag reaches MAX_LAG_SEC, landing on the next trigger;
//             RESYNC is the chance that ANY trigger does it.
//
//   STEERING — each of STRETCH, GRAIN and BLEND has an amount (-1..1) and a
//   source. The source is an analysis signal unless the caller says a cable
//   is patched (Ext), which is how the module normals its CV jacks:
//     STRETCH <- LEVEL      GRAIN <- BRIGHT      BLEND <- TONAL
//     RESYNC trigger <- ONSET
//
//   BLEND is a SEND, applied where the input splits: (1 - blend) goes
//   straight to the output, blend goes into the grain buffer. So when TONAL
//   steers it, noisy material never enters the grains at all, and grains
//   already sounding ring out instead of being cut.
//
//   IMPRINT — the grain output is re-shaped, FFT frame by frame, toward the
//   live input's broad spectral shape: divide out the grains' own shape,
//   multiply in the live one. At 1 the smear "says" what the input is doing
//   now. Its three extra FFTs per hop only run while IMPRINT is above zero.
//
// LATENCY: the direct path is delayed by FFT_SIZE samples (21 ms at 48 kHz)
// so the analysis looks slightly ahead of the audio it steers and the
// IMPRINT path lines up with the direct path.
//
// RANGES (widened 2026-11-21, owner: the first ranges were the practical
// middle, and the edges are where the unusual sounds are). GRAIN runs 1 ms to
// 1 s: below about 10 ms the spawn rate is an audible pitch and the smear
// turns into a buzz or a comb. STEP runs 1 ms to 2 s, STRETCH to 128x (all
// but frozen), DENSITY 0.25 (grains with gaps between them) to 16 deep.
//
// Signals are in engine units: audio +/-1 (the module scales Rack's +/-5 V),
// controls 0..1 (TUNE in semitones), analysis signals 0..1.
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace spectral_granulator {

static constexpr int   FFT_SIZE  = 1024;
static constexpr int   HOP_SIZE  = 256;            // 4x overlap
static constexpr int   HALF_SIZE = FFT_SIZE / 2;
static constexpr int   BINS      = HALF_SIZE + 1;
static constexpr int   TONAL_FFT  = 2048;          // TONAL's own, longer frame
static constexpr int   TONAL_BINS = TONAL_FFT / 2 + 1;
static constexpr int   FLUX_HISTORY = 512;         // hops of flux kept for the running median
static constexpr float WOLA_NORM = 1.5f;           // sum of Hann^2 at 75 % overlap
static constexpr float PI_F      = 3.14159265358979f;

static constexpr float MAX_LAG_SEC   = 8.0f;       // then the read head jumps back, on the next trigger
static constexpr float LATE_JUMP_SEC = 2.0f;       // ... or after this long with no trigger at all
static constexpr int   MAX_GRAINS    = 48;         // DENSITY 16 with JITTER bunching the spawns
static constexpr float MAX_GRAIN_SEC = 1.0f;
static constexpr float MAX_STEP_SEC  = 2.0f;
static constexpr float MAX_JITTER_SEC = 2.0f;
static constexpr float MAX_TUNE_SEMIS = 48.f;      // knob plus CV, plus the pitch set on top
static constexpr int   CONTROL_DIV   = 16;         // control curves are evaluated every 16 samples
static constexpr int   DISPLAY_COLS  = 64;

// ── Pitch sets ──────────────────────────────────────────────────────────────
static constexpr int PITCH_SET_COUNT = 8;
struct PitchSet { const char* name; int len; float semi[5]; };
inline const PitchSet* pitchSets()
{
    static const PitchSet S[PITCH_SET_COUNT] = {
        { "Unison",   1, {  0,   0,  0,  0,  0 } },
        { "Oct down", 2, {  0, -12,  0,  0,  0 } },
        { "Oct up",   2, {  0,  12,  0,  0,  0 } },
        { "Octaves",  2, { 12, -12,  0,  0,  0 } },
        { "Fifths",   4, {  0,   7,  0, -5,  0 } },
        { "Five",     5, {  0, -12, 12, -7,  7 } },
        { "Minor",    3, {  0,   3,  7,  0,  0 } },
        { "Major",    3, {  0,   4,  7,  0,  0 } },
    };
    return S;
}

// ── Control curves: 0..1 in, physical units out ─────────────────────────────
inline float clamp01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }
inline float focusHzFor(float x)    { return 20.f * std::pow(1000.f, clamp01(x)); }         // 20 Hz .. 20 kHz
inline float widthOctFor(float x)   { return 0.1f * std::pow(100.f, clamp01(x)); }          // 0.1 .. 10 octaves
inline bool  widthIsFull(float x)   { return x >= 0.98f; }                                  // top of travel: everything
inline float smoothSecFor(float x)  { return 0.002f * std::pow(5000.f, clamp01(x)); }       // 2 ms .. 10 s
inline float stretchFor(float x)    { return std::pow(128.f, clamp01(x)); }                 // 1x .. 128x
inline float grainSecFor(float x)   { return 0.001f * std::pow(1000.f, clamp01(x)); }       // 1 ms .. 1 s
inline float densityFor(float x)    { return 0.25f * std::pow(64.f, clamp01(x)); }          // 0.25 .. 16 grains deep
inline float stepSecFor(float x)                                                            // off, then 1 ms .. 2 s
{
    x = clamp01(x);
    if (x < 0.02f) return 0.f;
    return 0.001f * std::pow(2000.f, (x - 0.02f) / 0.98f);
}
inline float jitterSecFor(float x)  { x = clamp01(x); return MAX_JITTER_SEC * x * x * x; }  // 0 .. 2 s further back

// The curves backwards: the knob position for a value. For defaults and the lab.
inline float normFor(float value, float lo, float ratio) { return clamp01(std::log(value / lo) / std::log(ratio)); }
inline float focusNorm(float hz)    { return normFor(hz, 20.f, 1000.f); }
inline float widthNorm(float oct)   { return normFor(oct, 0.1f, 100.f); }
inline float smoothNorm(float sec)  { return normFor(sec, 0.002f, 5000.f); }
inline float stretchNorm(float x)   { return normFor(x, 1.f, 128.f); }
inline float grainNorm(float sec)   { return normFor(sec, 0.001f, 1000.f); }
inline float densityNorm(float n)   { return normFor(n, 0.25f, 64.f); }
inline float stepNorm(float sec)    { return sec <= 0.f ? 0.f : 0.02f + 0.98f * normFor(sec, 0.001f, 2000.f); }
inline float jitterNorm(float sec)  { return clamp01(std::pow(sec / MAX_JITTER_SEC, 1.f / 3.f)); }

// What the knobs say, all 0..1 except the steering amounts (-1..1), the set
// and TUNE (semitones).
struct Controls
{
    // analysis
    float focus  = 0.5f;
    float width  = 1.0f;     // full spectrum
    float sens   = 0.5f;
    float smooth = smoothNorm(0.050f);
    // grains
    int   pitchSet = 0;
    float step     = 0.f;    // off
    float stretch  = stretchNorm(4.f);
    float grain    = grainNorm(0.095f);
    float density  = densityNorm(6.f);
    float tune     = 0.f;    // semitones, on top of the pitch set
    float jitter   = 0.f;    // off: the ordered machine
    float spread   = 0.5f;   // half width
    float feedback = 0.f;
    float blend    = 1.f;    // all grains
    float imprint  = 0.f;
    float resync   = 0.f;    // only when the lag runs out
    // steering amounts
    float stretchAmt = 0.f;
    float grainAmt   = 0.f;
    float blendAmt   = 0.f;
};

// Patched cables. A patched CV replaces the analysis signal as the steering
// source (cv is 0..1 per 10 V, may be negative); a patched RESYNC jack
// replaces the onset as the trigger.
struct Ext
{
    bool  stretchPatched = false; float stretchCv = 0.f;
    bool  grainPatched   = false; float grainCv   = 0.f;
    bool  blendPatched   = false; float blendCv   = 0.f;
    bool  resyncPatched  = false; bool  resyncEdge = false;
};

struct Out
{
    float left = 0.f, right = 0.f;
    bool  onset = false;                 // true on the one sample an onset is detected
    float level = 0.f, bright = 0.f, tonal = 0.f;   // smoothed, 0..1
};

// What the panel display reads. Written on the audio thread once per hop,
// read by the widget; a torn read costs one frame of a stale column.
struct Display
{
    float col[DISPLAY_COLS]    = {};     // spectrum, 0..1 (-72..0 dB), log-frequency columns
    float weight[DISPLAY_COLS] = {};     // the FOCUS/WIDTH band, 0..1
    float flux = 0.f;                    // onset function, same scale as threshold
    float threshold = 0.f;
    uint32_t onsetSerial = 0;            // increments on every onset
};

// ── FFT: in-place radix-2, shared twiddle table (the SpectralEffect one) ────
struct Fft
{
    static constexpr int MAX_N = TONAL_FFT;
    struct Table
    {
        float re[MAX_N / 2];
        float im[MAX_N / 2];
        float window[FFT_SIZE];          // periodic Hann, FFT_SIZE long
        float windowLong[TONAL_FFT];     // ... and TONAL_FFT long
        Table()
        {
            for (int k = 0; k < MAX_N / 2; k++)
            {
                float a = 2.f * PI_F * (float)k / (float)MAX_N;
                re[k] = std::cos(a);
                im[k] = std::sin(a);
            }
            for (int i = 0; i < FFT_SIZE; i++)
                window[i] = 0.5f * (1.f - std::cos(2.f * PI_F * (float)i / (float)FFT_SIZE));
            for (int i = 0; i < TONAL_FFT; i++)
                windowLong[i] = 0.5f * (1.f - std::cos(2.f * PI_F * (float)i / (float)TONAL_FFT));
        }
    };
    static const Table& table() { static const Table t; return t; }

    // n is a power of two, at most MAX_N.
    static void transform(float* real, float* imag, int n, bool inverse)
    {
        for (int i = 1, j = 0; i < n; i++)
        {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) { std::swap(real[i], real[j]); std::swap(imag[i], imag[j]); }
        }
        const Table& tw = table();
        const float sign = inverse ? -1.f : 1.f;
        for (int len = 2; len <= n; len <<= 1)
        {
            const int half = len >> 1;
            const int stride = MAX_N / len;
            for (int i = 0; i < n; i += len)
            {
                for (int j = 0; j < half; j++)
                {
                    const float wR = tw.re[j * stride];
                    const float wI = tw.im[j * stride] * sign;
                    const float tR = wR * real[i + j + half] - wI * imag[i + j + half];
                    const float tI = wR * imag[i + j + half] + wI * real[i + j + half];
                    real[i + j + half] = real[i + j] - tR;
                    imag[i + j + half] = imag[i + j] - tI;
                    real[i + j] += tR;
                    imag[i + j] += tI;
                }
            }
        }
        if (inverse)
        {
            const float inv = 1.f / (float)n;
            for (int i = 0; i < n; i++) { real[i] *= inv; imag[i] *= inv; }
        }
    }
};

// ── Calibration of the analysis signals ─────────────────────────────────────
// Set against the lab's clips (tests/spectral_granulator/README.md has the numbers).
struct Calibration
{
    // LEVEL: dB of the band's RMS, mapped linearly.
    static constexpr float LEVEL_DB_LO = -50.f, LEVEL_DB_HI = -6.f;
    // BRIGHT: magnitude-weighted centroid in log2(Hz).
    static constexpr float BRIGHT_LOG2_LO = 7.0f;     // 128 Hz
    static constexpr float BRIGHT_LOG2_HI = 12.0f;    // 4.1 kHz
    // TONAL: flatness of the whitened power spectrum. Noise sits near HI, a
    // few clean partials near LO.
    static constexpr float FLAT_NOISE = 0.50f;
    static constexpr float FLAT_TONE  = 0.12f;
    // Below this band level the three continuous signals hold (BRIGHT, TONAL)
    // rather than chase the noise floor.
    static constexpr float GATE_DB = -66.f;
    // ONSET: flux is compared with (running upper quartile * mult + floor).
    static constexpr float FLUX_GAIN = 40.f;          // log(1 + FLUX_GAIN * magnitude)
    static constexpr float MEDIAN_SEC = 0.5f;
    static constexpr float REFRACTORY_SEC = 0.05f;
    // SENS 0 / 0.5 / 1: mult 5 / 2.2 / 1, floor 0.03 / 0.004 / 0.0005. On the
    // lab's clips a sustained voice or flute peaks near 4x its median at a
    // flux of 0.005 .. 0.02; drums peak at 10x and more, near 0.05 .. 0.2.
    static float threshMult(float sens)  { return 5.f * std::pow(0.2f, sens); }
    static float threshFloor(float sens) { return 0.03f * std::pow(1.f / 60.f, sens); }
};

class Engine
{
public:
    Display display;

    Engine() { setSampleRate(48000.f); }

    void seed(uint32_t s) { rng = s ? s : 0x9E3779B9u; }

    // Allocates. Call off the audio path (constructor, onSampleRateChange).
    void setSampleRate(float sr)
    {
        sampleRate = sr > 1000.f ? sr : 44100.f;
        // Room for the furthest read: the lag, a STEP hold on top of it, JITTER
        // on top of that, and a slowed grain drifting back by its own length.
        size_t want = (size_t)((MAX_LAG_SEC + MAX_STEP_SEC + MAX_JITTER_SEC + MAX_GRAIN_SEC + 0.1f) * sampleRate);
        size_t size = 1;
        while (size < want) size <<= 1;
        if (size > ((size_t)1 << 22)) size = (size_t)1 << 22;
        ringL.assign(size, 0.f);
        ringR.assign(size, 0.f);
        ringMask = size - 1;
        const float binHz = sampleRate / (float)FFT_SIZE;
        for (int k = 0; k < BINS; k++)
        {
            binLog2[k] = std::log2(std::max((float)k, 0.5f) * binHz);
            // The envelope smoother's half-width: 4 bins, widening with
            // frequency (roughly a constant fraction of an octave up top).
            envHalfWidth[k] = 4 + k / 16;
        }
        const float longBinHz = sampleRate / (float)TONAL_FFT;
        for (int k = 0; k < TONAL_BINS; k++)
        {
            longBinLog2[k] = std::log2(std::max((float)k, 0.5f) * longBinHz);
            longEnvHalfWidth[k] = 8 + k / 16;
        }
        medianHops = std::max(8, std::min(FLUX_HISTORY, (int)(Calibration::MEDIAN_SEC * sampleRate / (float)HOP_SIZE)));
        // Display columns: log-spaced 30 Hz .. 0.45 * sr, as bin ranges.
        const float lo = std::log2(30.f), hi = std::log2(0.45f * sampleRate);
        for (int c = 0; c <= DISPLAY_COLS; c++)
        {
            float f = std::pow(2.f, lo + (hi - lo) * (float)c / (float)DISPLAY_COLS);
            int k = (int)(f / binHz + 0.5f);
            colEdge[c] = std::max(1, std::min(BINS - 1, k));
        }
        reset();
    }

    // Clears everything, the grain buffer included (megabytes: call it from
    // the UI thread or a sample-rate change, never per sample).
    void reset()
    {
        std::fill(ringL.begin(), ringL.end(), 0.f);
        std::fill(ringR.begin(), ringR.end(), 0.f);
        ringWrite = 0;
        for (int i = 0; i < MAX_GRAINS; i++) grains[i].active = false;
        spawnAcc = 1.e9f;                 // spawn on the first sample
        lag = heldLag = 32.0;
        holdCounter = 0;
        pitchStep = 0;
        panStep = 0;
        catchUpPending = false;
        catchUpWait = 0;
        spawnJitter = 1.f;
        avEnv = avEnvSq = avCross = avCrossMax = 0.f;
        outGain = 1.f;
        fbL = fbR = 0.f;
        dcL = dcR = 0.f;

        std::memset(anaIn, 0, sizeof(anaIn));
        std::memset(anaLong, 0, sizeof(anaLong));
        std::memset(fluxHistory, 0, sizeof(fluxHistory));
        fluxHistoryPos = 0;
        longPos = 0;
        std::memset(delayL, 0, sizeof(delayL));
        std::memset(delayR, 0, sizeof(delayR));
        std::memset(grainDelayL, 0, sizeof(grainDelayL));
        std::memset(grainDelayR, 0, sizeof(grainDelayR));
        std::memset(stftIn, 0, sizeof(stftIn));
        std::memset(stftOut, 0, sizeof(stftOut));
        std::memset(prevLogMag, 0, sizeof(prevLogMag));
        std::memset(liveEnv, 0, sizeof(liveEnv));
        std::memset(liveEnvHeld, 0, sizeof(liveEnvHeld));
        ringPos = 0;
        hopCounter = 0;
        outReadPos = 0;

        fluxMedian = 0.f;
        onsetArmed = true;
        refractory = 0;
        levelTarget = brightTarget = tonalTarget = 0.f;
        level = bright = tonal = 0.f;
        imprintMix = 0.f;
        stftRunning = false;
        controlCounter = 0;
        sendGrain = 1.f; sendDirect = 0.f;
        sendGrainTarget = 1.f; sendDirectTarget = 0.f;
        snapSend = true;                  // the first control update sets the send without a slew
        hopOnset = false;
    }

    float latencySamples() const { return (float)FFT_SIZE; }

    Out process(float inL, float inR, const Controls& c, const Ext& ext)
    {
        Out out;

        // ── 1. Analysis input and the FFT_SIZE delay the audio rides on ──────
        anaIn[ringPos] = 0.5f * (inL + inR);
        anaLong[longPos] = anaIn[ringPos];
        longPos = (longPos + 1) & (TONAL_FFT - 1);
        const float dL = delayL[ringPos];      // the sample written FFT_SIZE ago
        const float dR = delayR[ringPos];
        delayL[ringPos] = inL;
        delayR[ringPos] = inR;

        // ── 2. Control rate: curves and steering ─────────────────────────────
        if (controlCounter <= 0)
        {
            controlCounter = CONTROL_DIV;
            updateControls(c, ext);
        }
        controlCounter--;

        // ── 3. Triggers: an onset from the last hop, or the patched jack ─────
        bool trigger = ext.resyncPatched ? ext.resyncEdge : hopOnset;
        out.onset = hopOnset;
        hopOnset = false;
        if (trigger)
        {
            if (catchUpPending) catchUp();
            else if (c.resync > 0.f && nextRandom() < c.resync) catchUp();
        }

        // ── 4. The send split, then the grains ───────────────────────────────
        sendGrain  += sendSlew * (sendGrainTarget  - sendGrain);
        sendDirect += sendSlew * (sendDirectTarget - sendDirect);

        float gL = dL * sendGrain;
        float gR = dR * sendGrain;
        processGrains(gL, gR);

        // ── 5. IMPRINT: the grains through the STFT, or through a matching delay ──
        const float bypassL = grainDelayL[ringPos];
        const float bypassR = grainDelayR[ringPos];
        grainDelayL[ringPos] = gL;
        grainDelayR[ringPos] = gR;
        stftIn[0][ringPos] = gL;
        stftIn[1][ringPos] = gR;

        float wetL = bypassL, wetR = bypassR;
        if (stftRunning)
        {
            const float sL = stftOut[0][outReadPos];
            const float sR = stftOut[1][outReadPos];
            wetL = bypassL + imprintMix * (sL - bypassL);
            wetR = bypassR + imprintMix * (sR - bypassR);
        }
        outReadPos++;

        // The crossfade between the delayed grains and the STFT. At IMPRINT 0
        // the two are the same signal, so the fade only hides the STFT's
        // start-up (its overlap-add needs FFT_SIZE samples to fill).
        const float mixTarget = (stftRunning && c.imprint > 0.f && stftWarm >= FFT_SIZE) ? 1.f : 0.f;
        imprintMix += mixSlew * (mixTarget - imprintMix);

        ringPos = (ringPos + 1) & (FFT_SIZE - 1);

        // ── 6. The hop: analysis, then the IMPRINT frames ────────────────────
        hopCounter++;
        if (hopCounter >= HOP_SIZE)
        {
            hopCounter = 0;
            outReadPos = 0;
            analyse(c);

            const bool want = c.imprint > 0.f;
            if (want && !stftRunning)
            {
                std::memset(stftOut, 0, sizeof(stftOut));
                stftRunning = true;
                stftWarm = 0;
            }
            if (stftRunning)
            {
                shiftOla(stftOut[0]);
                shiftOla(stftOut[1]);
                imprintFrame(0, c.imprint);
                imprintFrame(1, c.imprint);
                if (stftWarm < FFT_SIZE) stftWarm += HOP_SIZE;
                if (!want && imprintMix < 1.e-3f) stftRunning = false;
            }
        }

        // ── 7. Feature slew and the mix ──────────────────────────────────────
        level  += featureSlew * (levelTarget  - level);
        bright += featureSlew * (brightTarget - bright);
        tonal  += featureSlew * (tonalTarget  - tonal);
        out.level = level;
        out.bright = bright;
        out.tonal = tonal;

        out.left  = dL * sendDirect + wetL;
        out.right = dR * sendDirect + wetR;
        return out;
    }

    // For the lab and the tests.
    float debugFlux() const { return lastFlux; }
    float debugFluxMedian() const { return fluxMedian; }
    float debugFlatness() const { return lastFlatness; }
    float debugCentroidLog2() const { return lastCentroid; }
    float debugLevelDb() const { return lastLevelDb; }
    double debugLagSamples() const { return lag; }
    float sampleRateHz() const { return sampleRate; }

private:
    struct Grain
    {
        bool  active = false;
        int   age = 0;
        int   duration = 0;
        float durationRecip = 0.f;
        float readOffset = 0.f;
        float pitchRate = 1.f;
        float gainL = 1.f, gainR = 1.f;
    };

    float sampleRate = 48000.f;
    uint32_t rng = 0x9E3779B9u;

    // grain buffer
    std::vector<float> ringL, ringR;
    size_t ringMask = 0;
    size_t ringWrite = 0;
    Grain grains[MAX_GRAINS];
    float spawnAcc = 0.f;
    double lag = 32.0;                   // the read head, samples behind the write head
    double heldLag = 32.0;               // the head the grains use (== lag with STEP off)
    int   holdCounter = 0;
    int   pitchStep = 0;
    int   panStep = 0;
    bool  catchUpPending = false;
    int   catchUpWait = 0;
    float spawnJitter = 1.f;             // this grain's share of the spawn interval, 0.1 .. 1.9
    float fbL = 0.f, fbR = 0.f;          // last grain output, for FEEDBACK
    float dcL = 0.f, dcR = 0.f;          // the feedback path's DC blocker

    // derived at control rate
    int   pitchSet = 0;
    double rate = 1.0;                   // 1 / stretch
    int   grainSamples = 4800;
    float spawnInterval = 800.f;
    int   stepSamples = 0;
    float tuneSemis = 0.f;
    float jitterAmt = 0.f;               // 0..1
    float jitterSamples = 0.f;
    float spreadAmt = 0.5f;
    float feedbackGain = 0.f;            // 0 .. 1.1
    float dcCoef = 0.003f;
    float outGain = 1.f;
    float gainSlew = 0.0002f;
    float avEnv = 0.f, avEnvSq = 0.f;    // running sum(env) and sum(env^2) of the live grains
    float avCross = 0.f, avCrossMax = 0.f;
    float sendGrain = 1.f, sendDirect = 0.f;
    float sendGrainTarget = 1.f, sendDirectTarget = 0.f;
    float sendSlew = 0.01f;
    float featureSlew = 0.01f;
    float mixSlew = 0.001f;
    int   controlCounter = 0;
    bool  snapSend = true;

    // analysis / STFT, all indexed by ringPos (mod FFT_SIZE)
    float anaIn[FFT_SIZE] = {};
    float delayL[FFT_SIZE] = {}, delayR[FFT_SIZE] = {};
    float grainDelayL[FFT_SIZE] = {}, grainDelayR[FFT_SIZE] = {};
    float stftIn[2][FFT_SIZE] = {};
    float stftOut[2][FFT_SIZE] = {};
    int   ringPos = 0;
    int   hopCounter = 0;
    int   outReadPos = 0;
    bool  stftRunning = false;
    int   stftWarm = 0;
    float imprintMix = 0.f;

    float anaLong[TONAL_FFT] = {};            // TONAL's input ring, indexed by longPos
    int   longPos = 0;
    float fftRe[TONAL_FFT] = {}, fftIm[TONAL_FFT] = {};
    float mag[TONAL_BINS] = {};
    float env[TONAL_BINS] = {};
    float cum[TONAL_BINS + 1] = {};
    float weight[BINS] = {};
    float binLog2[BINS] = {};
    int   envHalfWidth[BINS] = {};
    float longBinLog2[TONAL_BINS] = {};
    int   longEnvHalfWidth[TONAL_BINS] = {};
    float fluxHistory[FLUX_HISTORY] = {};
    float fluxSorted[FLUX_HISTORY] = {};
    int   fluxHistoryPos = 0;
    int   medianHops = 94;
    int   colEdge[DISPLAY_COLS + 1] = {};
    float prevLogMag[BINS] = {};
    float liveEnv[BINS] = {};                 // this hop's live envelope
    float liveEnvHeld[BINS] = {};             // instant attack, SMOOTH-length release: what IMPRINT applies

    float fluxMedian = 0.f;
    bool  onsetArmed = true;
    int   refractory = 0;                // hops
    bool  hopOnset = false;
    float levelTarget = 0.f, brightTarget = 0.f, tonalTarget = 0.f;
    float level = 0.f, bright = 0.f, tonal = 0.f;
    float lastFlux = 0.f, lastFlatness = 0.f, lastCentroid = 0.f, lastLevelDb = -120.f;

    float nextRandom()
    {
        rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
        return (float)(rng >> 8) * (1.f / 16777216.f);
    }

    // ── Control rate ────────────────────────────────────────────────────────
    void updateControls(const Controls& c, const Ext& ext)
    {
        const float stretchSrc = ext.stretchPatched ? ext.stretchCv : level;
        const float grainSrc   = ext.grainPatched   ? ext.grainCv   : bright;
        const float blendSrc   = ext.blendPatched   ? ext.blendCv   : tonal;

        const float stretchN = clamp01(c.stretch + c.stretchAmt * stretchSrc);
        const float grainN   = clamp01(c.grain   + c.grainAmt   * grainSrc);
        const float blendN   = clamp01(c.blend   + c.blendAmt   * blendSrc);

        pitchSet = std::max(0, std::min(PITCH_SET_COUNT - 1, c.pitchSet));
        const float stretch = stretchFor(stretchN);
        rate = 1.0 / (double)stretch;

        grainSamples = std::max(16, (int)(grainSecFor(grainN) * sampleRate));
        const float density = densityFor(c.density);
        spawnInterval = std::max(4.f, (float)grainSamples / density);
        stepSamples = (int)(stepSecFor(c.step) * sampleRate);

        tuneSemis = std::max(-MAX_TUNE_SEMIS, std::min(MAX_TUNE_SEMIS, c.tune));
        jitterAmt = clamp01(c.jitter);
        jitterSamples = jitterSecFor(c.jitter) * sampleRate;
        spreadAmt = clamp01(c.spread);
        feedbackGain = 1.1f * clamp01(c.feedback);
        dcCoef = 1.f - std::exp(-2.f * PI_F * 20.f / sampleRate);

        // Output gain, MEASURED (2026-11-21). Grains that read different
        // audio add incoherently: power adds, so the right divisor is
        // sqrt(sum of env^2) (GranularSmear's note of 2026-09-14). Grains
        // that read the same audio add coherently and the divisor is
        // sum(env). Which case holds depends on the material as much as on
        // the settings: 5 ms grains 16 deep on a djembe (long periods) came
        // out 5.5 dB hot under the first version's rule, which guessed from
        // STRETCH alone, while 2 ms grains on a voice were right. So the
        // grains' own cross terms are measured in processGrains() and the
        // gain slides between the two divisors by how coherent they are.
        // Both divisors are floored at 1 so a lone grain is never boosted.
        {
            const float gIncoherent = 1.f / std::sqrt(std::max(1.f, avEnvSq));
            const float gCoherent = 1.f / std::max(1.f, avEnv);
            const float coherence = avCrossMax > 1.e-12f ? clamp01(avCross / avCrossMax) : 0.f;
            outGain = gIncoherent + coherence * (gCoherent - gIncoherent);
        }
        gainSlew = 1.f - std::exp(-1.f / (0.100f * sampleRate));

        // Equal-power send split.
        sendGrainTarget  = std::sqrt(blendN);
        sendDirectTarget = std::sqrt(1.f - blendN);
        if (snapSend) { sendGrain = sendGrainTarget; sendDirect = sendDirectTarget; snapSend = false; }

        sendSlew    = 1.f - std::exp(-1.f / (0.005f * sampleRate));
        featureSlew = 1.f - std::exp(-1.f / (smoothSecFor(c.smooth) * sampleRate));
        mixSlew     = 1.f - std::exp(-1.f / (0.020f * sampleRate));
    }

    // ── Grains ──────────────────────────────────────────────────────────────
    void catchUp()
    {
        lag = 32.0;
        heldLag = lag;
        holdCounter = 0;
        catchUpPending = false;
        catchUpWait = 0;
    }

    static float hermite(float y0, float y1, float y2, float y3, float f)
    {
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + y1;
    }

    // delay 0 = the sample just written. Needs delay >= 2.
    void readRing(float delay, float& l, float& r) const
    {
        const size_t d = (size_t)delay;
        const float f = delay - (float)d;
        const size_t base = ringWrite - 1 - d;
        const size_t i0 = (base - 1) & ringMask;      // one older
        const size_t i1 = base & ringMask;
        const size_t i2 = (base + 1) & ringMask;
        const size_t i3 = (base + 2) & ringMask;
        l = hermite(ringL[i0], ringL[i1], ringL[i2], ringL[i3], f);
        r = hermite(ringR[i0], ringR[i1], ringR[i2], ringR[i3], f);
    }

    void spawnGrain()
    {
        for (int i = 0; i < MAX_GRAINS; i++)
        {
            Grain& g = grains[i];
            if (g.active) continue;
            g.active = true;
            g.age = 0;
            g.duration = grainSamples;

            // The set's next interval, in order, plus TUNE.
            const PitchSet& set = pitchSets()[pitchSet];
            float semis = set.semi[pitchStep % set.len] + tuneSemis;
            pitchStep++;
            semis = std::max(-MAX_TUNE_SEMIS, std::min(MAX_TUNE_SEMIS, semis));
            g.pitchRate = std::pow(2.f, semis / 12.f);

            // Where it reads: the held head, and with JITTER a random
            // distance further back.
            float offset = (float)heldLag;
            if (jitterSamples > 0.f) offset += nextRandom() * jitterSamples;

            // A slowed grain drifts back by up to its own length, so the
            // furthest a grain may start is that much short of the buffer.
            const float room = (float)ringMask - 16.f - (float)g.duration;

            // A grain faster than unity eats history faster than the write
            // head makes it: keep its whole life behind the head. If the
            // buffer cannot hold that much history (a long grain several
            // octaves up), the grain is shortened to what it can hold.
            if (g.pitchRate > 1.f)
            {
                float needed = 16.f + (g.pitchRate - 1.f) * (float)g.duration;
                if (needed > room)
                {
                    g.duration = std::max(16, (int)((room - 16.f) / (g.pitchRate - 1.f)));
                    needed = 16.f + (g.pitchRate - 1.f) * (float)g.duration;
                }
                if (offset < needed) offset = needed;
            }
            if (offset > room) offset = room;
            g.readOffset = offset;
            g.durationRecip = 1.f / (float)g.duration;

            // Left, centre, right in turn, SPREAD wide: a stereo smear from a
            // mono source. The cycle is 3 long so it does not lock to the
            // 2- and 4-long pitch sets.
            static const float PAN[3] = { -1.f, 0.f, 1.f };
            const float a = (PAN[panStep % 3] * spreadAmt + 1.f) * 0.25f * PI_F;
            panStep++;
            g.gainL = std::cos(a) * 1.41421356f;
            g.gainR = std::sin(a) * 1.41421356f;

            // JITTER also moves the next spawn: 0.1 to 1.9 of the interval,
            // the same on average, so short grains stop ringing at the spawn
            // rate and turn to noise.
            spawnJitter = 1.f + jitterAmt * 0.9f * (2.f * nextRandom() - 1.f);
            return;
        }
    }

    // tanh-shaped, exactly +/-1 at +/-3 and held there beyond.
    static float softLimit(float x)
    {
        if (x > 3.f) return 1.f;
        if (x < -3.f) return -1.f;
        return x * (27.f + x * x) / (27.f + 9.f * x * x);
    }

    void processGrains(float& left, float& right)
    {
        // FEEDBACK: the last grain output goes back in with the input,
        // through a 20 Hz DC blocker and a soft limiter, so the loop can sit
        // at or past unity and stay bounded. The limiter's ceiling is a
        // quarter of full scale: a loop that has saturated is feeding every
        // grain the same waveform, the grains then add COHERENTLY (about
        // 2x), and a full-scale ceiling left the output sitting at 2.0 rms
        // (1.0 at half scale). At a quarter it settles near 0.5 rms, the
        // level of loud programme material.
        if (feedbackGain > 0.f)
        {
            dcL += dcCoef * (fbL - dcL);
            dcR += dcCoef * (fbR - dcR);
            left  += feedbackGain * 0.25f * softLimit(4.f * (fbL - dcL));
            right += feedbackGain * 0.25f * softLimit(4.f * (fbR - dcR));
        }

        // Write, then move the heads.
        ringL[ringWrite & ringMask] = left;
        ringR[ringWrite & ringMask] = right;
        ringWrite++;

        // The lag may use what the buffer has left after a STEP hold, the
        // JITTER reach and a slowed grain's drift (all of MAX_LAG_SEC at
        // ordinary sample rates; less at the very high ones, where the
        // buffer is capped).
        double maxLag = (double)ringMask - 64.0 - (double)grainSamples - (double)stepSamples - (double)jitterSamples;
        if (maxLag > (double)(MAX_LAG_SEC * sampleRate)) maxLag = (double)(MAX_LAG_SEC * sampleRate);
        if (maxLag < 64.0) maxLag = 64.0;
        lag += 1.0 - rate;
        if (lag < 32.0) lag = 32.0;
        if (lag > maxLag)
        {
            lag = maxLag;
            catchUpPending = true;
            if (++catchUpWait > (int)(LATE_JUMP_SEC * sampleRate)) catchUp();
        }

        if (stepSamples <= 0) heldLag = lag;
        else
        {
            if (holdCounter <= 0) { heldLag = lag; holdCounter = stepSamples; }
            holdCounter--;
            // The held head stays put in the SOURCE, so its lag grows as the
            // write head moves on.
            heldLag += 1.0;
            if (heldLag > maxLag + (double)stepSamples + 1.0) heldLag = lag;
        }

        spawnAcc += 1.f;
        const float spawnAt = spawnInterval * spawnJitter;
        if (spawnAcc >= spawnAt)
        {
            spawnAcc = (spawnAcc > 1.e8f) ? 0.f : spawnAcc - spawnAt;
            spawnGrain();
        }

        const float maxDelay = (float)ringMask - 4.f;
        const float* hann = Fft::table().window;
        float sumL = 0.f, sumR = 0.f;
        float envSum = 0.f, envSqSum = 0.f, monoSum = 0.f, monoAbs = 0.f, monoSq = 0.f;
        for (int i = 0; i < MAX_GRAINS; i++)
        {
            Grain& g = grains[i];
            if (!g.active) continue;

            // Hann envelope from the FFT's window table.
            const float pos = (float)g.age * g.durationRecip * (float)FFT_SIZE;
            const int pi = (int)pos;
            const float pf = pos - (float)pi;
            const float e0 = hann[pi & (FFT_SIZE - 1)];
            const float e1 = hann[(pi + 1) & (FFT_SIZE - 1)];
            const float envv = e0 + pf * (e1 - e0);

            float delay = g.readOffset + (1.f - g.pitchRate) * (float)g.age;
            if (delay < 2.f) delay = 2.f;
            if (delay > maxDelay) delay = maxDelay;

            float l, r;
            readRing(delay, l, r);
            sumL += l * envv * g.gainL;
            sumR += r * envv * g.gainR;

            // For the measured gain: this grain's mono contribution.
            const float m = 0.5f * (l + r) * envv;
            envSum += envv;
            envSqSum += envv * envv;
            monoSum += m;
            monoAbs += std::fabs(m);
            monoSq += m * m;

            g.age++;
            if (g.age >= g.duration) g.active = false;
        }
        // The cross terms of the sum, and the most they could be if every
        // grain were in step: their ratio is the grains' coherence.
        avEnv      += gainSlew * (envSum - avEnv);
        avEnvSq    += gainSlew * (envSqSum - avEnvSq);
        avCross    += gainSlew * ((monoSum * monoSum - monoSq) - avCross);
        avCrossMax += gainSlew * ((monoAbs * monoAbs - monoSq) - avCrossMax);

        left = sumL * outGain;
        right = sumR * outGain;
        fbL = left;
        fbR = right;
    }

    // ── The hop ─────────────────────────────────────────────────────────────
    static void shiftOla(float* buf)
    {
        std::memmove(buf, buf + HOP_SIZE, (FFT_SIZE - HOP_SIZE) * sizeof(float));
        std::fill(buf + (FFT_SIZE - HOP_SIZE), buf + FFT_SIZE, 0.f);
    }

    // Windowed frame of the last FFT_SIZE samples of `in` (a ring indexed by
    // ringPos, which now points at the oldest sample) into fftRe / fftIm.
    void loadFrame(const float* in)
    {
        const float* w = Fft::table().window;
        for (int i = 0; i < FFT_SIZE; i++)
        {
            fftRe[i] = in[(ringPos + i) & (FFT_SIZE - 1)] * w[i];
            fftIm[i] = 0.f;
        }
    }

    // Broad spectral shape: a box average of the magnitudes whose width
    // grows with frequency, by prefix sums (SpectralSmear's trick).
    void smoothEnvelope(const float* m, float* e, int bins, const int* halfWidth)
    {
        cum[0] = 0.f;
        for (int k = 0; k < bins; k++) cum[k + 1] = cum[k] + m[k];
        for (int k = 0; k < bins; k++)
        {
            const int hw = halfWidth[k];
            const int a = std::max(0, k - hw);
            const int b = std::min(bins - 1, k + hw);
            e[k] = (cum[b + 1] - cum[a]) / (float)(b - a + 1);
        }
    }

    // The FOCUS/WIDTH band at one log2 frequency: flat across the middle half
    // of WIDTH, raised-cosine shoulders, in octaves around FOCUS.
    static float bandWeight(float log2Hz, float centre, float half, bool full)
    {
        if (full) return 1.f;
        const float d = std::fabs(log2Hz - centre);
        if (d >= half) return 0.f;
        if (d <= 0.5f * half) return 1.f;
        return 0.5f * (1.f + std::cos(PI_F * (d - 0.5f * half) / (0.5f * half)));
    }

    // TONAL, from its own TONAL_FFT frame: the flatness (geometric over
    // arithmetic mean) of the power spectrum after the broad shape is
    // divided out, inside the band.
    float tonalFlatness(float centre, float half, bool full)
    {
        const float* w = Fft::table().windowLong;
        for (int i = 0; i < TONAL_FFT; i++)
        {
            fftRe[i] = anaLong[(longPos + i) & (TONAL_FFT - 1)] * w[i];
            fftIm[i] = 0.f;
        }
        Fft::transform(fftRe, fftIm, TONAL_FFT, false);
        for (int k = 0; k < TONAL_BINS; k++)
            mag[k] = std::sqrt(fftRe[k] * fftRe[k] + fftIm[k] * fftIm[k]);
        smoothEnvelope(mag, env, TONAL_BINS, longEnvHalfWidth);

        float wSum = 0.f, logSum = 0.f, whiteSum = 0.f;
        for (int k = 1; k < TONAL_BINS; k++)
        {
            const float bw = bandWeight(longBinLog2[k], centre, half, full);
            if (bw <= 0.f) continue;
            const float r = mag[k] / (env[k] + 1.e-9f);
            const float p = r * r + 1.e-9f;
            wSum += bw;
            logSum += bw * std::log(p);
            whiteSum += bw * p;
        }
        if (wSum < 1.e-6f) return 1.f;
        return std::exp(logSum / wSum) / (whiteSum / wSum + 1.e-12f);
    }

    void analyse(const Controls& c)
    {
        const bool full = widthIsFull(c.width);
        const float centre = std::log2(focusHzFor(c.focus));
        const float half = 0.5f * widthOctFor(c.width);

        // TONAL first: it borrows the FFT buffers the main frame then reuses.
        const float flat = tonalFlatness(centre, half, full);

        loadFrame(anaIn);
        Fft::transform(fftRe, fftIm, FFT_SIZE, false);

        // A full-scale sine reads 1.0 at its bin.
        const float norm = 4.f / (float)FFT_SIZE;
        for (int k = 0; k < BINS; k++)
            mag[k] = std::sqrt(fftRe[k] * fftRe[k] + fftIm[k] * fftIm[k]) * norm;

        smoothEnvelope(mag, liveEnv, BINS, envHalfWidth);

        // IMPRINT's copy of the live shape: up at once, down over SMOOTH (at
        // least 30 ms), so the shape hangs on after the input lets go.
        const float relSec = std::max(0.030f, smoothSecFor(c.smooth));
        const float rel = std::exp(-(float)HOP_SIZE / (relSec * sampleRate));
        for (int k = 0; k < BINS; k++)
            liveEnvHeld[k] = std::max(liveEnv[k], liveEnvHeld[k] * rel);

        // The band. DC is never listened to.
        weight[0] = 0.f;
        for (int k = 1; k < BINS; k++) weight[k] = bandWeight(binLog2[k], centre, half, full);

        // One pass for LEVEL, BRIGHT and the flux.
        float wSum = 0.f, power = 0.f, magSum = 0.f, centroidSum = 0.f, flux = 0.f;
        for (int k = 1; k < BINS; k++)
        {
            const float m = mag[k];
            const float lm = std::log(1.f + Calibration::FLUX_GAIN * m);
            const float w = weight[k];
            if (w > 0.f)
            {
                wSum += w;
                power += w * m * m;
                magSum += w * m;
                centroidSum += w * m * binLog2[k];
                const float d = lm - prevLogMag[k];
                if (d > 0.f) flux += w * d;
            }
            prevLogMag[k] = lm;
        }
        if (wSum < 1.e-6f) wSum = 1.e-6f;
        flux /= wSum;

        // LEVEL. sum(m^2) over the positive bins is 3 * rms^2 under a Hann.
        const float levelDb = 10.f * std::log10(power / 3.f + 1.e-12f);
        levelTarget = clamp01((levelDb - Calibration::LEVEL_DB_LO) / (Calibration::LEVEL_DB_HI - Calibration::LEVEL_DB_LO));
        lastLevelDb = levelDb;

        if (levelDb > Calibration::GATE_DB)
        {
            const float centroid = centroidSum / (magSum + 1.e-12f);
            brightTarget = clamp01((centroid - Calibration::BRIGHT_LOG2_LO) / (Calibration::BRIGHT_LOG2_HI - Calibration::BRIGHT_LOG2_LO));
            lastCentroid = centroid;
            tonalTarget = clamp01((Calibration::FLAT_NOISE - flat) / (Calibration::FLAT_NOISE - Calibration::FLAT_TONE));
            lastFlatness = flat;
        }

        // ONSET. The threshold is a multiple of the flux's running upper
        // quartile over MEDIAN_SEC. A quantile, not the mean: the hits
        // themselves must not raise the bar they are measured against. The
        // upper quartile, not the median: in a narrow band the flux is zero
        // half the time and the median sits at nothing, so everything fired
        // (14 onsets a second from a voice through a 2-octave bass band).
        // After a hit the detector waits for the flux to fall back and for
        // REFRACTORY_SEC.
        const float threshold = fluxMedian * Calibration::threshMult(c.sens) + Calibration::threshFloor(c.sens);
        if (refractory > 0) refractory--;
        if (flux > threshold && onsetArmed && refractory == 0)
        {
            hopOnset = true;
            onsetArmed = false;
            refractory = std::max(1, (int)(Calibration::REFRACTORY_SEC * sampleRate / (float)HOP_SIZE));
            display.onsetSerial++;
        }
        else if (flux < 0.7f * threshold) onsetArmed = true;

        fluxHistory[fluxHistoryPos] = flux;
        fluxHistoryPos = (fluxHistoryPos + 1) % medianHops;
        std::memcpy(fluxSorted, fluxHistory, sizeof(float) * (size_t)medianHops);
        const int q = (medianHops * 3) / 4;
        std::nth_element(fluxSorted, fluxSorted + q, fluxSorted + medianHops);
        fluxMedian = fluxSorted[q];
        lastFlux = flux;

        // The display.
        display.flux = flux;
        display.threshold = threshold;
        for (int col = 0; col < DISPLAY_COLS; col++)
        {
            const int a = colEdge[col];
            const int b = std::max(a + 1, colEdge[col + 1]);
            float peak = 0.f, wPeak = 0.f;
            for (int k = a; k < b && k < BINS; k++)
            {
                if (mag[k] > peak) peak = mag[k];
                if (weight[k] > wPeak) wPeak = weight[k];
            }
            const float db = 20.f * std::log10(peak + 1.e-9f);
            display.col[col] = clamp01((db + 72.f) / 72.f);
            display.weight[col] = wPeak;
        }
    }

    // One channel's grain frame, re-shaped toward the live envelope.
    void imprintFrame(int ch, float amount)
    {
        loadFrame(stftIn[ch]);
        Fft::transform(fftRe, fftIm, FFT_SIZE, false);

        if (amount > 0.f)
        {
            const float norm = 4.f / (float)FFT_SIZE;
            for (int k = 0; k < BINS; k++)
                mag[k] = std::sqrt(fftRe[k] * fftRe[k] + fftIm[k] * fftIm[k]) * norm;
            smoothEnvelope(mag, env, BINS, envHalfWidth);

            // gain = (live / grain) ^ amount, capped at +18 dB so a bin the
            // grains barely have is not blown up into noise.
            static constexpr float FLOOR = 1.e-5f;       // -100 dB
            static constexpr float MAX_GAIN = 8.f;
            for (int k = 0; k < BINS; k++)
            {
                float ratio = (liveEnvHeld[k] + FLOOR) / (env[k] + FLOOR);
                float g = (amount >= 0.999f) ? ratio : std::exp(amount * std::log(ratio));
                if (g > MAX_GAIN) g = MAX_GAIN;
                fftRe[k] *= g;
                fftIm[k] *= g;
            }
        }

        for (int k = HALF_SIZE + 1; k < FFT_SIZE; k++)
        {
            fftRe[k] =  fftRe[FFT_SIZE - k];
            fftIm[k] = -fftIm[FFT_SIZE - k];
        }
        Fft::transform(fftRe, fftIm, FFT_SIZE, true);

        const float* w = Fft::table().window;
        const float inv = 1.f / WOLA_NORM;
        float* acc = stftOut[ch];
        for (int i = 0; i < FFT_SIZE; i++) acc[i] += fftRe[i] * w[i] * inv;
    }
};

} // namespace spectral_granulator
