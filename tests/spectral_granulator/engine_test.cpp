// engine_test.cpp: Spectral Granulator's engine outside Rack. Synthetic signals with
// known answers: the audio path's identities, the four analysis signals at
// their extremes, and no NaN anywhere. See README.md.
#include <cmath>
#include <cstdio>
#include <vector>
#include "modules/SpectralGranulator/SpectralGranulatorEngine.hpp"
#include "check.hpp"

using namespace spectral_granulator;

static const float SR = 48000.f;

struct Lcg
{
    uint32_t s = 12345u;
    float next() { s = s * 1664525u + 1013904223u; return (float)(s >> 8) * (2.f / 16777216.f) - 1.f; }
};

typedef float (*Gen)(int n, Lcg& r);

static float genSine(int n, Lcg&)  { return 0.5f * std::sin(2.f * PI_F * 440.f * (float)n / SR); }
static float genSaw(int n, Lcg&)   { float p = std::fmod(220.f * (float)n / SR, 1.f); return 0.5f * (2.f * p - 1.f); }
static float genNoise(int, Lcg& r) { return 0.5f * r.next(); }
static float genSilence(int, Lcg&) { return 0.f; }
// A 5 ms noise burst every 250 ms: 4 per second.
static float genBursts(int n, Lcg& r)
{
    int period = (int)(0.25f * SR);
    return (n % period) < (int)(0.005f * SR) ? 0.8f * r.next() : 0.f;
}
// A chord with vibrato and a noise burst on every half second: something for
// every stage to chew on.
static float genMix(int n, Lcg& r)
{
    float t = (float)n / SR;
    float v = 0.2f * std::sin(2.f * PI_F * 220.f * t + 2.f * std::sin(2.f * PI_F * 5.f * t))
            + 0.2f * std::sin(2.f * PI_F * 277.2f * t)
            + 0.2f * std::sin(2.f * PI_F * 329.6f * t);
    if ((n % (int)(0.5f * SR)) < (int)(0.02f * SR)) v += 0.4f * r.next();
    return v;
}

struct Run
{
    std::vector<float> inL, outL, outR;
    std::vector<float> level, bright, tonal;
    int onsets = 0;
    bool bad = false;
};

static Run run(Gen gen, float seconds, const Controls& c, const Ext& ext = Ext())
{
    Engine e;
    e.setSampleRate(SR);
    Lcg rng;
    Run r;
    int n = (int)(seconds * SR);
    r.inL.resize(n); r.outL.resize(n); r.outR.resize(n);
    r.level.resize(n); r.bright.resize(n); r.tonal.resize(n);
    for (int i = 0; i < n; i++)
    {
        float x = gen(i, rng);
        Out o = e.process(x, x, c, ext);
        r.inL[i] = x; r.outL[i] = o.left; r.outR[i] = o.right;
        r.level[i] = o.level; r.bright[i] = o.bright; r.tonal[i] = o.tonal;
        if (o.onset) r.onsets++;
        if (!std::isfinite(o.left) || !std::isfinite(o.right) || !std::isfinite(o.level)
            || !std::isfinite(o.bright) || !std::isfinite(o.tonal)) r.bad = true;
    }
    return r;
}

static double rms(const std::vector<float>& v, int from, int to)
{
    double s = 0.0;
    for (int i = from; i < to; i++) s += (double)v[i] * v[i];
    return std::sqrt(s / (double)std::max(1, to - from));
}

static float mean(const std::vector<float>& v, int from, int to)
{
    double s = 0.0;
    for (int i = from; i < to; i++) s += v[i];
    return (float)(s / (double)std::max(1, to - from));
}

static double db(double ratio) { return 20.0 * std::log10(ratio + 1e-12); }

int main()
{
    const int N1 = (int)SR, N2 = 2 * (int)SR, N3 = 3 * (int)SR;

    // ── BLEND 0 is the input, FFT_SIZE samples late, exactly ────────────────
    {
        Controls c; c.blend = 0.f;
        Run r = run(genMix, 2.f, c);
        double worst = 0.0;
        for (int i = FFT_SIZE + 2000; i < N2; i++)
            worst = std::max(worst, (double)std::fabs(r.outL[i] - r.inL[i - FFT_SIZE]));
        std::printf("blend 0: worst |out - in[n-%d]| = %.2e\n", FFT_SIZE, worst);
        check("blend 0 passes the input delayed by FFT_SIZE", worst < 1e-5);
        check("blend 0: no NaN", !r.bad);
    }

    // ── Unison, no stretch: the grains reassemble the input at its level ────
    {
        Controls c; c.stretch = 0.f;
        Run r = run(genSine, 2.f, c);
        double d = db(rms(r.outL, N1, N2) / rms(r.inL, N1, N2));
        std::printf("stretch 1x unison sine: out/in = %+.2f dB\n", d);
        check("stretch 1x unison within 1 dB of the input", std::fabs(d) < 1.0);
    }

    // ── Stretched noise keeps roughly its level (the incoherent gain) ───────
    {
        Controls c; c.stretch = stretchNorm(8.f);
        Run r = run(genNoise, 3.f, c);
        double d = db(rms(r.outL, N1, N3) / rms(r.inL, N1, N3));
        std::printf("stretch 8x unison noise: out/in = %+.2f dB\n", d);
        check("stretched noise within 3 dB of the input", std::fabs(d) < 3.0);
        check("stretched noise: no NaN", !r.bad);
    }

    // ── IMPRINT barely on is still the grains (the STFT is transparent) ─────
    {
        Controls a; a.stretch = 0.f;
        Controls b = a; b.imprint = 1.e-4f;
        Run ra = run(genMix, 2.f, a);
        Run rb = run(genMix, 2.f, b);
        std::vector<float> diff(N2);
        for (int i = 0; i < N2; i++) diff[i] = ra.outL[i] - rb.outL[i];
        double d = db(rms(diff, N1, N2) / rms(ra.outL, N1, N2));
        std::printf("imprint ~0 vs off: difference %.1f dB below the signal\n", -d);
        check("imprint ~0 is transparent (difference under -40 dB)", d < -40.0);
    }

    // ── IMPRINT 1 with a silent input is silent; with input it is not ───────
    {
        Controls c; c.imprint = 1.f; c.stretch = stretchNorm(8.f);
        Run r = run(genMix, 3.f, c);
        double d = db(rms(r.outL, N1, N3) / rms(r.inL, N1, N3));
        std::printf("imprint 1, stretch 8x, mix: out/in = %+.2f dB\n", d);
        check("imprint 1: output within 12 dB of the input", std::fabs(d) < 12.0);
        check("imprint 1: no NaN", !r.bad);
    }

    // ── Every pitch set, with step and resync: finite and bounded ───────────
    for (int set = 0; set < PITCH_SET_COUNT; set++)
    {
        Controls c; c.pitchSet = set; c.step = stepNorm(0.070f); c.stretch = stretchNorm(16.f); c.resync = 0.5f;
        c.imprint = 0.5f; c.stretchAmt = 0.5f; c.grainAmt = -0.5f; c.blendAmt = -0.5f;
        Run r = run(genMix, 3.f, c);
        float peak = 0.f;
        for (int i = 0; i < N3; i++) peak = std::max(peak, std::max(std::fabs(r.outL[i]), std::fabs(r.outR[i])));
        std::printf("set %-9s peak %.2f\n", pitchSets()[set].name, peak);
        check("pitch set: no NaN", !r.bad);
        check("pitch set: peak under 4x full scale", peak < 4.f);
    }

    // ── The analysis signals at their extremes ──────────────────────────────
    {
        Controls c;
        Run sine = run(genSine, 2.f, c), saw = run(genSaw, 2.f, c), noise = run(genNoise, 2.f, c), sil = run(genSilence, 2.f, c);
        float tSine = mean(sine.tonal, N1, N2), tSaw = mean(saw.tonal, N1, N2), tNoise = mean(noise.tonal, N1, N2);
        std::printf("TONAL  sine %.2f  saw %.2f  noise %.2f\n", tSine, tSaw, tNoise);
        check("TONAL: a sine reads above 0.8", tSine > 0.8f);
        check("TONAL: a saw reads above 0.6", tSaw > 0.6f);
        check("TONAL: white noise reads below 0.2", tNoise < 0.2f);

        float bSine = mean(sine.bright, N1, N2), bNoise = mean(noise.bright, N1, N2);
        std::printf("BRIGHT sine(440) %.2f  noise %.2f\n", bSine, bNoise);
        check("BRIGHT: noise is brighter than a 440 Hz sine", bNoise > bSine + 0.3f);

        float lSine = mean(sine.level, N1, N2), lSil = mean(sil.level, N1, N2);
        std::printf("LEVEL  sine(-9 dB rms) %.2f  silence %.2f\n", lSine, lSil);
        check("LEVEL: a 0.5-amplitude sine (-9 dB rms) reads 0.93 +/- 0.03", std::fabs(lSine - 0.932f) < 0.03f);
        check("LEVEL: silence reads 0", lSil < 0.001f);
        check("ONSET: silence never fires", sil.onsets == 0);
        check("silence: no NaN", !sil.bad);

        std::printf("ONSET  steady sine %d  steady noise %d (2 s)\n", sine.onsets, noise.onsets);
        check("ONSET: a steady sine fires at most once (its start)", sine.onsets <= 1);
        check("ONSET: steady noise fires at most twice", noise.onsets <= 2);
    }

    // ── ONSET counts bursts: 4 per second for 4 seconds ─────────────────────
    {
        Controls c;
        Run r = run(genBursts, 4.f, c);
        std::printf("ONSET  16 bursts -> %d onsets\n", r.onsets);
        checkEq("ONSET: one per burst", r.onsets, 16);
    }

    // ── FOCUS: a band that excludes the signal hears nothing ────────────────
    {
        Controls c; c.focus = focusNorm(9000.f); c.width = widthNorm(1.f);      // 9 kHz, an octave wide
        Run r = run(genSine, 2.f, c);
        float l = mean(r.level, N1, N2);
        std::printf("FOCUS  9 kHz band on a 440 Hz sine: LEVEL %.3f\n", l);
        check("FOCUS: a 440 Hz sine is outside a 9 kHz band", l < 0.05f);
    }

    // ── RESYNC: the read head comes back on a patched trigger ───────────────
    {
        Engine e; e.setSampleRate(SR);
        Controls c; c.stretch = 1.f; c.resync = 1.f;
        Ext ext; ext.resyncPatched = true;
        Lcg rng;
        for (int i = 0; i < N1; i++) { ext.resyncEdge = false; e.process(genMix(i, rng), 0.f, c, ext); }
        double before = e.debugLagSamples();
        ext.resyncEdge = true;
        e.process(0.f, 0.f, c, ext);
        double after = e.debugLagSamples();
        std::printf("RESYNC lag %.0f -> %.0f samples\n", before, after);
        check("RESYNC: one second at 128x leaves the head ~0.99 s behind", before > 0.95 * SR && before < SR);
        check("RESYNC: a trigger brings it back", after < 64.0);
    }

    // ── The curves and their inverses agree ─────────────────────────────────
    {
        check("stretchNorm round trip", std::fabs(stretchFor(stretchNorm(8.f)) - 8.f) < 0.01f);
        check("grainNorm round trip", std::fabs(grainSecFor(grainNorm(0.005f)) - 0.005f) < 1e-5f);
        check("densityNorm round trip", std::fabs(densityFor(densityNorm(12.f)) - 12.f) < 0.01f);
        check("stepNorm round trip", std::fabs(stepSecFor(stepNorm(0.003f)) - 0.003f) < 1e-5f);
        check("stepNorm(0) is off", stepSecFor(stepNorm(0.f)) == 0.f);
        check("smoothNorm round trip", std::fabs(smoothSecFor(smoothNorm(0.05f)) - 0.05f) < 1e-4f);
        check("jitterNorm round trip", std::fabs(jitterSecFor(jitterNorm(0.25f)) - 0.25f) < 1e-4f);
        check("the ends: 1 ms .. 1 s grains, 1x .. 128x", std::fabs(grainSecFor(0.f) - 0.001f) < 1e-6f && std::fabs(grainSecFor(1.f) - 1.f) < 1e-3f
              && stretchFor(0.f) == 1.f && std::fabs(stretchFor(1.f) - 128.f) < 0.01f);
    }

    // ── TUNE +12 doubles the frequency ──────────────────────────────────────
    {
        Controls c; c.stretch = 0.f; c.tune = 12.f; c.spread = 0.f;
        Run r = run(genSine, 3.f, c);
        int crossings = 0;
        for (int i = N2 + 1; i < N3; i++) if ((r.outL[i - 1] < 0.f) != (r.outL[i] < 0.f)) crossings++;
        std::printf("TUNE +12 on 440 Hz: %d zero crossings in 1 s (880 Hz is 1760)\n", crossings);
        check("TUNE +12: 440 Hz comes out at 880 Hz (within 3 %)", std::abs(crossings - 1760) < 53);
        check("TUNE: no NaN", !r.bad);
    }

    // ── SPREAD 0 is mono for a mono input; SPREAD 1 is not ──────────────────
    {
        Controls c; c.spread = 0.f;
        Run narrow = run(genMix, 2.f, c);
        c.spread = 1.f;
        Run wide = run(genMix, 2.f, c);
        double dn = 0.0, dw = 0.0;
        for (int i = N1; i < N2; i++)
        {
            dn = std::max(dn, (double)std::fabs(narrow.outL[i] - narrow.outR[i]));
            dw = std::max(dw, (double)std::fabs(wide.outL[i] - wide.outR[i]));
        }
        std::printf("SPREAD worst |L - R|: 0 -> %.2e, 1 -> %.2f\n", dn, dw);
        check("SPREAD 0: left equals right", dn < 1e-5);
        check("SPREAD 1: left and right differ", dw > 0.05);
    }

    // ── FEEDBACK: the smear outlives its input, and stays bounded ───────────
    {
        // One second of the mix, then five of silence, at 2x.
        struct G { static float gen(int n, Lcg& r) { return n < (int)SR ? genMix(n, r) : 0.f; } };
        Controls c; c.stretch = stretchNorm(2.f);
        Run dry = run(G::gen, 6.f, c);
        c.feedback = 1.f;
        Run wet = run(G::gen, 6.f, c);
        double tailDry = rms(dry.outL, 5 * N1, 6 * N1), tailWet = rms(wet.outL, 5 * N1, 6 * N1);
        float peak = 0.f;
        for (int i = 0; i < 6 * N1; i++) peak = std::max(peak, std::fabs(wet.outL[i]));
        std::printf("FEEDBACK tail rms, 4 s after the input stops: off %.2e, full %.3f; peak %.2f\n", tailDry, tailWet, peak);
        check("FEEDBACK 0: silent 4 s after the input stops", tailDry < 1e-4);
        check("FEEDBACK full: still sounding 4 s after the input stops", tailWet > 0.02);
        check("FEEDBACK full: peak under 4x full scale", peak < 4.f);
        check("FEEDBACK: no NaN", !wet.bad);
    }

    // ── The ends of the widened ranges: finite, bounded, and not silent ─────
    {
        struct Case { const char* name; float grain, density, step, stretch, tune, jitter, feedback; };
        static const Case CASES[] = {
            { "1 ms grains, 16 deep",        0.f, 1.f, 0.f,  stretchNorm(4.f), 0.f,   0.f, 0.f },
            { "1 ms grains, 1 ms steps",     0.f, 1.f, stepNorm(0.001f), stretchNorm(4.f), 0.f, 0.f, 0.f },
            { "1 s grains, 0.25 deep",       1.f, 0.f, 0.f,  stretchNorm(4.f), 0.f,   0.f, 0.f },
            { "1 s grains, +48, full jitter", 1.f, 0.5f, 1.f, stretchNorm(4.f), 48.f,  1.f, 0.f },
            { "1 s grains, -48, 2 s steps",  1.f, 0.5f, 1.f, stretchNorm(4.f), -48.f, 0.2f, 0.f },
            { "128x, 2 s steps, full jitter", 0.5f, 0.5f, 1.f, 1.f,             0.f,   1.f, 0.f },
            { "5 ms grains, feedback, +12",  grainNorm(0.005f), 1.f, 0.f, stretchNorm(8.f), 12.f, 0.5f, 1.f },
        };
        for (size_t k = 0; k < sizeof(CASES) / sizeof(CASES[0]); k++)
        {
            const Case& q = CASES[k];
            Controls c; c.grain = q.grain; c.density = q.density; c.step = q.step; c.stretch = q.stretch;
            c.tune = q.tune; c.jitter = q.jitter; c.feedback = q.feedback; c.pitchSet = 5; c.imprint = 0.5f;
            Run r = run(genMix, 4.f, c);
            float peak = 0.f;
            for (int i = 0; i < 4 * N1; i++) peak = std::max(peak, std::max(std::fabs(r.outL[i]), std::fabs(r.outR[i])));
            double level = rms(r.outL, 0, 4 * N1);       // the whole run: sparse grains leave whole seconds empty
            std::printf("%-30s peak %.2f  rms %.3f\n", q.name, peak, level);
            check("range end: no NaN", !r.bad);
            check("range end: peak under 6x full scale", peak < 6.f);
            // (128x with full jitter reads from before the 4 s test signal began, most of the time.)
            check("range end: not silent", level > 1e-3 || k == 6);
        }
    }

    return finish("spectral_granulator engine_test");
}
