// lab.cpp: Spectral Granulator's listening lab and calibration tool. See README.md.
//
//   ./lab stats  <in.wav> [more.wav ...]
//       Per clip: the distribution of each raw analysis quantity (band level
//       in dB, centroid in log2 Hz, whitened flatness, flux / running mean)
//       and the onset rate at three SENS settings. This is what
//       spectral_granulator::Calibration was set from.
//
//   ./lab render <in.wav> <out_dir> [start_sec] [len_sec] [name_prefix ...]
//       One 32-bit float WAV per row of VARIANTS plus 00_dry.wav, and
//       features.csv (time, onset, level, bright, tonal at the default
//       controls, every 256 samples). With name prefixes, only the variants
//       whose names start with one of them, and no dry file or CSV.
//
// Input is scaled so the file's full scale is the engine's +/-1, which is
// what the module feeds it from a +/-5 V Rack signal.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "vgLib-2.0/AudioFile.h"
#include "modules/SpectralGranulator/SpectralGranulatorEngine.hpp"

using namespace spectral_granulator;

struct Clip { std::vector<float> l, r; float sr = 48000.f; };

static bool loadClip(const std::string& path, float startSec, float lenSec, Clip& c)
{
    AudioFile<float> f;
    if (!f.load(path)) { std::printf("cannot load %s\n", path.c_str()); return false; }
    c.sr = (float)f.getSampleRate();
    int total = f.getNumSamplesPerChannel();
    int start = std::min(total, (int)(startSec * c.sr));
    int n = lenSec > 0.f ? std::min(total - start, (int)(lenSec * c.sr)) : total - start;
    int chR = f.getNumChannels() > 1 ? 1 : 0;
    c.l.assign(f.samples[0].begin() + start, f.samples[0].begin() + start + n);
    c.r.assign(f.samples[chR].begin() + start, f.samples[chR].begin() + start + n);
    return true;
}

static float pct(std::vector<float> v, float p)
{
    if (v.empty()) return 0.f;
    std::sort(v.begin(), v.end());
    return v[(size_t)(p * (float)(v.size() - 1))];
}

static const char* baseName(const char* p)
{
    const char* s = std::strrchr(p, '/');
    return s ? s + 1 : p;
}

// ── stats ───────────────────────────────────────────────────────────────────
static int stats(int argc, char** argv)
{
    std::printf("%-28s %-17s %-17s %-17s %-14s %s\n", "clip", "level dB 10/50/90", "centroid 10/50/90",
                "flatness 10/50/90", "flux/med  50/99", "onsets/s @ sens .25 .5 .75");
    for (int a = 2; a < argc; a++)
    {
        Clip clip;
        if (!loadClip(argv[a], 0.f, 60.f, clip)) continue;
        int n = (int)clip.l.size();
        std::vector<float> lev, cen, flat, ratio;
        float rate[3] = {0, 0, 0};
        static const float SENS[3] = {0.25f, 0.5f, 0.75f};
        for (int s = 0; s < 3; s++)
        {
            Engine e; e.setSampleRate(clip.sr);
            Controls c; c.sens = SENS[s];
            Ext ext;
            int onsets = 0;
            for (int i = 0; i < n; i++)
            {
                Out o = e.process(clip.l[i], clip.r[i], c, ext);
                if (o.onset) onsets++;
                if (s == 1 && (i % HOP_SIZE) == HOP_SIZE - 1 && i > (int)clip.sr)
                {
                    if (e.debugLevelDb() > -50.f)
                    {
                        lev.push_back(e.debugLevelDb());
                        cen.push_back(e.debugCentroidLog2());
                        flat.push_back(e.debugFlatness());
                    }
                    ratio.push_back(e.debugFlux() / (e.debugFluxMedian() + 1e-6f));
                }
            }
            rate[s] = (float)onsets / ((float)n / clip.sr);
        }
        std::printf("%-28.28s %5.0f %5.0f %5.0f %5.1f %5.1f %5.1f %5.2f %5.2f %5.2f %6.2f %6.2f   %5.1f %5.1f %5.1f\n",
                    baseName(argv[a]),
                    pct(lev, .1f), pct(lev, .5f), pct(lev, .9f),
                    pct(cen, .1f), pct(cen, .5f), pct(cen, .9f),
                    pct(flat, .1f), pct(flat, .5f), pct(flat, .9f),
                    pct(ratio, .5f), pct(ratio, .99f),
                    rate[0], rate[1], rate[2]);
    }
    return 0;
}

// ── render ──────────────────────────────────────────────────────────────────
// A variant is a name and a function that sets the controls it cares about on
// top of the defaults, in real units through the engine's *Norm helpers.
struct Variant { const char* name; void (*set)(Controls&); };

static const Variant VARIANTS[] = {
    // Baselines: the smear with nothing steering it.
    { "01_smear_4x",             [](Controls& c) { c.stretch = stretchNorm(4); } },
    { "02_smear_16x",            [](Controls& c) { c.stretch = stretchNorm(16); } },
    // ONSET -> RESYNC: every hit pulls the read head back to the present.
    { "03_resync_all_8x",        [](Controls& c) { c.stretch = stretchNorm(8); c.resync = 1; } },
    { "04_resync_half_16x",      [](Controls& c) { c.stretch = stretchNorm(16); c.resync = .5f; } },
    { "05_resync_sens_low_16x",  [](Controls& c) { c.stretch = stretchNorm(16); c.resync = 1; c.sens = .2f; } },
    { "06_resync_bass_only_16x", [](Controls& c) { c.stretch = stretchNorm(16); c.resync = 1; c.focus = focusNorm(90); c.width = widthNorm(2); } },
    // LEVEL -> STRETCH.
    { "07_level_stretch_pos",    [](Controls& c) { c.stretch = 0; c.stretchAmt = .7f; } },
    { "08_level_stretch_neg",    [](Controls& c) { c.stretch = stretchNorm(32); c.stretchAmt = -.7f; c.resync = .5f; } },
    // BRIGHT -> GRAIN.
    { "09_bright_grain_neg",     [](Controls& c) { c.stretch = stretchNorm(6); c.grain = grainNorm(.3f); c.grainAmt = -.35f; c.resync = 1; } },
    { "10_bright_grain_pos",     [](Controls& c) { c.stretch = stretchNorm(6); c.grain = grainNorm(.03f); c.grainAmt = .35f; c.resync = 1; } },
    // TONAL -> BLEND (what is sent to the grains).
    { "11_tonal_send_8x",        [](Controls& c) { c.stretch = stretchNorm(8); c.blend = 0; c.blendAmt = 1; c.resync = .5f; } },
    { "12_noisy_send_8x",        [](Controls& c) { c.stretch = stretchNorm(8); c.blend = 1; c.blendAmt = -1; c.resync = .5f; } },
    // Pitch sets and STEP.
    { "13_octdown_resync",       [](Controls& c) { c.pitchSet = 1; c.stretch = stretchNorm(8); c.resync = 1; } },
    { "14_fifths_step_resync",   [](Controls& c) { c.pitchSet = 4; c.step = stepNorm(.1f); c.stretch = stretchNorm(11); c.resync = 1; } },
    { "15_five_step_tonal_send", [](Controls& c) { c.pitchSet = 5; c.step = stepNorm(.055f); c.stretch = stretchNorm(11); c.grain = grainNorm(.075f); c.blend = 0; c.blendAmt = 1; c.resync = .5f; } },
    { "16_minor_sparse",         [](Controls& c) { c.pitchSet = 6; c.stretch = stretchNorm(16); c.grain = grainNorm(.19f); c.density = densityNorm(3); c.resync = .5f; } },
    // IMPRINT.
    { "17_imprint_16x",          [](Controls& c) { c.stretch = stretchNorm(16); c.imprint = 1; } },
    { "18_imprint_octaves_16x",  [](Controls& c) { c.pitchSet = 3; c.step = stepNorm(.03f); c.stretch = stretchNorm(16); c.imprint = 1; } },
    { "19_imprint_hold_16x",     [](Controls& c) { c.stretch = stretchNorm(16); c.imprint = 1; c.smooth = smoothNorm(.6f); } },
    { "20_imprint_half_five",    [](Controls& c) { c.pitchSet = 5; c.step = stepNorm(.055f); c.stretch = stretchNorm(11); c.imprint = .5f; c.resync = .5f; } },
    { "21_all_steered",          [](Controls& c) { c.pitchSet = 4; c.step = stepNorm(.03f); c.stretch = stretchNorm(3); c.grain = grainNorm(.15f); c.blend = .2f; c.imprint = .6f; c.resync = .7f; c.sens = .6f; c.stretchAmt = .5f; c.grainAmt = -.3f; c.blendAmt = .8f; } },

    // ── Batch 2 (2026-11-21): the widened ranges and the four new controls ──
    // Tiny grains: below ~10 ms the spawn rate is a pitch of its own.
    { "30_grain_2ms",            [](Controls& c) { c.grain = grainNorm(.002f); c.stretch = stretchNorm(4); } },
    { "31_grain_5ms_16deep",     [](Controls& c) { c.grain = grainNorm(.005f); c.density = densityNorm(16); c.stretch = stretchNorm(8); c.resync = 1; } },
    { "32_grain_3ms_bright",     [](Controls& c) { c.grain = grainNorm(.003f); c.grainAmt = .5f; c.stretch = stretchNorm(4); c.resync = 1; } },
    { "33_grain_1s_sparse",      [](Controls& c) { c.grain = grainNorm(1); c.density = densityNorm(.5f); c.stretch = stretchNorm(2); c.pitchSet = 4; } },
    // Short steps and the far end of STRETCH.
    { "34_step_3ms",             [](Controls& c) { c.step = stepNorm(.003f); c.stretch = stretchNorm(8); c.resync = 1; } },
    { "35_stretch_128x_resync",  [](Controls& c) { c.stretch = 1; c.resync = 1; } },
    // TUNE, JITTER, SPREAD, FEEDBACK.
    { "36_tune_minus7",          [](Controls& c) { c.tune = -7; c.stretch = stretchNorm(4); c.resync = 1; } },
    { "37_jitter_cloud",         [](Controls& c) { c.jitter = jitterNorm(.4f); c.stretch = stretchNorm(8); c.spread = 1; } },
    { "38_jitter_tiny_grains",   [](Controls& c) { c.grain = grainNorm(.004f); c.jitter = jitterNorm(.05f); c.density = densityNorm(12); c.stretch = stretchNorm(4); c.spread = 1; } },
    { "39_feedback_70",          [](Controls& c) { c.feedback = .64f; c.stretch = stretchNorm(4); c.resync = .5f; } },
    { "40_shimmer",              [](Controls& c) { c.pitchSet = 2; c.feedback = .7f; c.stretch = stretchNorm(2); c.grain = grainNorm(.12f); c.jitter = jitterNorm(.02f); c.spread = 1; c.blend = .6f; } },
    { "41_feedback_full_fifths", [](Controls& c) { c.pitchSet = 4; c.feedback = 1; c.stretch = stretchNorm(8); c.jitter = jitterNorm(.1f); c.resync = .3f; } },
    { "42_tonal_send_shimmer_imprint", [](Controls& c) { c.pitchSet = 2; c.feedback = .6f; c.stretch = stretchNorm(6); c.blend = 0; c.blendAmt = 1; c.imprint = .7f; c.spread = 1; c.sens = .75f; c.resync = .5f; } },
};

static Controls toControls(const Variant& v)
{
    Controls c;
    v.set(c);
    return c;
}

static bool writeWav(const std::string& path, const std::vector<float>& l, const std::vector<float>& r, float sr)
{
    AudioFile<float> out;
    out.setSampleRate((uint32_t)sr);
    out.setBitDepth(32);
    out.setAudioBufferSize(2, (int)l.size());
    for (size_t i = 0; i < l.size(); i++) { out.samples[0][i] = l[i]; out.samples[1][i] = r[i]; }
    return out.save(path, AudioFileFormat::Wave);
}

static int render(int argc, char** argv)
{
    if (argc < 4) { std::printf("usage: lab render <in.wav> <out_dir> [start_sec] [len_sec] [name_prefix ...]\n"); return 1; }
    std::string outDir = argv[3];
    float start = argc > 4 ? (float)std::atof(argv[4]) : 0.f;
    float len = argc > 5 ? (float)std::atof(argv[5]) : 20.f;
    // Any further arguments choose variants by the start of their names ("3" is all of batch 2).
    std::vector<std::string> only;
    for (int a = 6; a < argc; a++) only.push_back(argv[a]);
    Clip clip;
    if (!loadClip(argv[2], start, len, clip)) return 1;
    int n = (int)clip.l.size();
    if (only.empty()) writeWav(outDir + "/00_dry.wav", clip.l, clip.r, clip.sr);

    double sumIn = 0.0;
    for (int i = 0; i < n; i++) sumIn += (double)clip.l[i] * clip.l[i] + (double)clip.r[i] * clip.r[i];
    bool anyBad = false;

    std::printf("%-32s %7s %7s %8s\n", "variant", "peak", "rms dB", "onsets/s");
    for (size_t vi = 0; vi < sizeof(VARIANTS) / sizeof(VARIANTS[0]); vi++)
    {
        const Variant& v = VARIANTS[vi];
        bool wanted = only.empty();
        for (size_t k = 0; k < only.size(); k++) if (std::string(v.name).compare(0, only[k].size(), only[k]) == 0) wanted = true;
        if (!wanted) continue;
        Engine e; e.setSampleRate(clip.sr);
        Controls c = toControls(v);
        Ext ext;
        std::vector<float> l(n), r(n);
        float peak = 0.f; double sumOut = 0.0; int onsets = 0; bool bad = false;
        for (int i = 0; i < n; i++)
        {
            Out o = e.process(clip.l[i], clip.r[i], c, ext);
            l[i] = o.left; r[i] = o.right;
            if (o.onset) onsets++;
            if (!std::isfinite(o.left) || !std::isfinite(o.right)) bad = true;
            peak = std::max(peak, std::max(std::fabs(o.left), std::fabs(o.right)));
            sumOut += (double)o.left * o.left + (double)o.right * o.right;
        }
        anyBad = anyBad || bad;
        writeWav(outDir + "/" + v.name + ".wav", l, r, clip.sr);
        std::printf("%-32s %7.2f %+7.1f %8.1f%s\n", v.name, peak,
                    10.0 * std::log10((sumOut + 1e-12) / (sumIn + 1e-12)), (float)onsets / ((float)n / clip.sr),
                    bad ? "  NaN!" : "");
    }

    // The analysis signals at the default controls.
    if (only.empty())
    {
        Engine e; e.setSampleRate(clip.sr);
        Controls c; Ext ext;
        FILE* f = std::fopen((outDir + "/features.csv").c_str(), "w");
        if (f)
        {
            std::fprintf(f, "time,onset,level,bright,tonal\n");
            bool onset = false;
            for (int i = 0; i < n; i++)
            {
                Out o = e.process(clip.l[i], clip.r[i], c, ext);
                onset = onset || o.onset;
                if ((i % HOP_SIZE) == HOP_SIZE - 1)
                {
                    std::fprintf(f, "%.4f,%d,%.3f,%.3f,%.3f\n", (float)i / clip.sr, onset ? 1 : 0, o.level, o.bright, o.tonal);
                    onset = false;
                }
            }
            std::fclose(f);
        }
    }
    return anyBad ? 1 : 0;
}

int main(int argc, char** argv)
{
    if (argc >= 3 && std::strcmp(argv[1], "stats") == 0) return stats(argc, argv);
    if (argc >= 3 && std::strcmp(argv[1], "render") == 0) return render(argc, argv);
    std::printf("usage: lab stats <in.wav>...   |   lab render <in.wav> <out_dir> [start_sec] [len_sec]\n");
    return 1;
}
