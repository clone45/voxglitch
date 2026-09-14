// VX Drums SampleModel: the one-shot sample voice with the grain stretch.
//
//   g++ -std=c++11 -O2 -I ../../src/modules/VXDrums -o sample_test sample_test.cpp && ./sample_test

#include "VXDrumVoices.hpp"
#include "check.hpp"

#include <cmath>

using namespace vx_drums;

// A file of `n` frames holding the ramp i / n, at `rate`.
static SampleData ramp(int n, double rate = 44100.0)
{
    SampleData d;
    d.sampleRate = rate;
    d.mono.resize(n);
    for (int i = 0; i < n; i++) d.mono[i] = (float)i / (float)n;
    d.finalize();
    return d;
}

// A drum-shaped file: loud for the first `loud` frames, then a quiet tail.
static SampleData drum(int loud, int tail, float tailLevel = 0.01f)
{
    SampleData d;
    d.mono.resize(loud + tail);
    for (int i = 0; i < loud; i++) d.mono[i] = 1.f;
    for (int i = 0; i < tail; i++) d.mono[loud + i] = tailLevel;
    d.finalize();
    return d;
}

static SampleModel voice(const SampleData* d, double sr = 44100.0)
{
    SampleModel m;
    m.setSampleRate(sr);
    m.setData(d);
    const ModelSpec& ms = modelSpec(MODEL_SAMPLE);
    m.setTune(ms.tune.def);
    m.setDecay(ms.decay.def);
    m.setShape(ms.shape.def);
    return m;
}

// Ticks until the voice stops (or `limit`), returning how many samples were heard.
static int playOut(SampleModel& m, Rng& rng, int limit, double* last = nullptr)
{
    int n = 0;
    for (; n < limit && m.playing; n++)
    {
        double v = m.tick(rng);
        if (last) *last = v;
    }
    return n;
}

static void silence()
{
    std::printf("no file\n");
    Rng rng;
    SampleModel m = voice(nullptr);
    m.strike(1.0, rng);
    check("a strike with no file does not play", !m.playing);
    checkEq("and ticks silence", (long long)(m.tick(rng) * 1000.0), 0);

    SampleData empty;
    m.setData(&empty);
    m.strike(1.0, rng);
    check("an empty file does not play either", !m.playing);
}

static void passthrough()
{
    std::printf("full decay = passthrough\n");
    const int N = 4410;
    SampleData d = ramp(N);
    Rng rng;
    SampleModel m = voice(&d);
    m.strike(1.0, rng);
    check("plays after a strike", m.playing);

    double maxErr = 0.0;
    int heard = 0;
    for (int i = 0; i < N && m.playing; i++, heard++)
    {
        double v = m.tick(rng);
        double err = std::fabs(v - (double)d.mono[i]);
        if (err > maxErr) maxErr = err;
    }
    check("every sample comes back as recorded (< 1e-6)", maxErr < 1e-6);
    checkEq("for the whole file", heard, N);
    playOut(m, rng, 100000);
    check("then it stops on its own", !m.playing);
}

static void amplitude()
{
    std::printf("amplitude\n");
    SampleData d = ramp(1000);
    Rng rng;
    SampleModel m = voice(&d);
    m.strike(1.5, rng);
    for (int i = 0; i < 500; i++) m.tick(rng);
    double v = m.tick(rng);
    double want = 1.5 * d.mono[500];
    check("the strike amplitude scales the output", std::fabs(v - want) < 1e-6);
}

static void decayEnvelope()
{
    std::printf("decay\n");
    const int N = 44100;   // 1 s
    SampleData d = ramp(N);
    Rng rng;

    // Full clockwise = no envelope: the whole file, untouched (passthrough
    // above already checks the values).
    SampleModel m = voice(&d);
    m.strike(1.0, rng);
    checkEq("at the knob's max the whole file plays", playOut(m, rng, 10 * N), N);

    // 0.25 s: 60 dB down at 0.25 s, silent (-80 dB) at about 0.33 s.
    m.setDecay(0.25);
    m.strike(1.0, rng);
    for (int i = 0; i < 11025; i++) m.tick(rng);   // 0.25 s in
    double v = m.tick(rng);
    double want = d.mono[11025] * 0.001;
    check("at DECAY seconds the level is 60 dB down", std::fabs(v - want) < want * 0.05);
    int n = 11026 + playOut(m, rng, 10 * N);
    check("and the hit ends near 1.33 x DECAY", std::fabs((double)n - 1.333 * 11025.0) < 600.0);

    // DECAY only ever shortens: a file shorter than the decay still ends at
    // its own end.
    SampleData shortFile = ramp(4410);
    SampleModel s2 = voice(&shortFile);
    s2.setDecay(2.0);
    s2.strike(1.0, rng);
    checkEq("a short file ends at its own end", playOut(s2, rng, 10 * N), 4410);

    // Pitch and decay are independent: +12 st at 0.25 s decay still ends at ~0.33 s.
    m.setTune(12.0);
    m.setDecay(0.25);
    m.strike(1.0, rng);
    int both = playOut(m, rng, 10 * N);
    check("+12 st does not change the decay time", std::fabs((double)both - 1.333 * 11025.0) < 600.0);
    m.setTune(0.0);
}

static void pitch()
{
    std::printf("tune\n");
    const int N = 44100;
    SampleData d = ramp(N);
    Rng rng;

    SampleModel m = voice(&d);
    m.setTune(12.0);
    m.strike(1.0, rng);
    int up = playOut(m, rng, 10 * N);
    check("+12 st plays in about half the samples", std::fabs((double)up - 0.5 * N) <= 2.0 * 441.0);

    m.setTune(-12.0);
    m.strike(1.0, rng);
    int down = playOut(m, rng, 10 * N);
    check("-12 st plays in about twice the samples", std::fabs((double)down - 2.0 * N) <= 2.0 * 441.0);

}

static void rateConversion()
{
    std::printf("file rate vs engine rate\n");
    const int N = 22050;   // 1 s at 22.05 kHz
    SampleData d = ramp(N, 22050.0);
    Rng rng;
    SampleModel m = voice(&d, 44100.0);
    m.strike(1.0, rng);
    int n = playOut(m, rng, 10 * N);
    check("a 22.05 kHz file plays for 1 s at 44.1 kHz", std::fabs((double)n - 44100.0) <= 2.0 * 441.0);

    SampleModel hi = voice(&d, 96000.0);
    hi.strike(1.0, rng);
    int n96 = playOut(hi, rng, 10 * 96000);
    check("and for 1 s at 96 kHz", std::fabs((double)n96 - 96000.0) <= 2.0 * 960.0);
}

static void startOffset()
{
    std::printf("start\n");
    Rng rng;

    // The knob follows the file's ENERGY, not its length: on a drum whose
    // energy is all in the first 10 %, half the knob is inside that 10 %.
    SampleData d = drum(1000, 9000);
    check("start 0 is the head of the file", d.startFrame(0.0) == 0.0);
    check("start 100 % is the end", d.startFrame(1.0) >= 9999.0);
    double half = d.startFrame(0.5);
    check("start 50 % on a drum is inside the loud part (~frame 550)", half > 450.0 && half < 650.0);
    double ninety = d.startFrame(0.9);
    check("start 90 % is at the end of the loud part (~frame 990)", ninety > 900.0 && ninety < 1000.0);
    double tail = d.startFrame(0.95);
    check("the top tenth of the knob sweeps the tail by position (95 % ~ frame 5000)", tail > 4500.0 && tail < 5500.0);

    // A silent file falls back to plain position.
    SampleData silent;
    silent.mono.assign(1000, 0.f);
    silent.finalize();
    check("a silent file maps the knob to position", std::fabs(silent.startFrame(0.5) - 500.0) <= 1.0);
    SampleData raw;
    raw.mono.assign(1000, 0.5f);
    check("an unfinalized file maps the knob to position", std::fabs(raw.startFrame(0.25) - 250.0) <= 1.0);

    // A ramp: energy grows as i^3, so the blended curve's midpoint is ~78 %
    // of the file (pure energy would say 79 %, pure position 50 %).
    SampleData r = ramp(10000);
    double rh = r.startFrame(0.5);
    check("start 50 % on a ramp is near the energy midpoint (~78 %)", rh > 7700.0 && rh < 7900.0);

    // The voice starts there, fading in over 1.5 ms so a mid-file start does
    // not click, and plays the rest.
    SampleModel m = voice(&r);
    m.setShape(0.5);
    m.strike(1.0, rng);
    double first = m.tick(rng);
    check("a mid-file start opens from silence", first < 0.02);
    for (int i = 0; i < 80; i++) m.tick(rng);   // past the 66-sample fade
    double after = m.tick(rng);
    check("and is at full level after the fade", std::fabs(after - r.mono[(int)rh + 81]) < 1e-6);
    int n = 82 + playOut(m, rng, 100000);
    check("and plays the remaining file", std::fabs((double)n - (10000.0 - rh)) <= 2.0);

    // The head of the file has no fade (the passthrough check above relies on it).
    m.setShape(0.0);
    m.strike(1.0, rng);
    m.tick(rng);
    double second = m.tick(rng);
    check("a start at the head plays the second sample as recorded", std::fabs(second - r.mono[1]) < 1e-6);
}

static void retriggerAndChoke()
{
    std::printf("retrigger, choke, reset\n");
    const int N = 44100;
    SampleData d = ramp(N);
    Rng rng;
    SampleModel m = voice(&d);
    m.strike(1.0, rng);
    for (int i = 0; i < 1000; i++) m.tick(rng);
    m.strike(1.0, rng);
    double v = m.tick(rng);
    check("a retrigger restarts from the beginning", std::fabs(v - d.mono[0]) < 1e-6);

    for (int i = 0; i < 20000; i++) m.tick(rng);
    m.choke();
    int rang = playOut(m, rng, N);
    check("a choke fades out within ~100 ms rather than ringing on", rang > 0 && rang < 4410);
    check("and the voice is then stopped", !m.playing);

    m.strike(1.0, rng);
    m.reset();
    check("reset silences the voice", !m.playing);
    checkEq("and it ticks 0", (long long)(m.tick(rng) * 1000.0), 0);
}

static void swapping()
{
    std::printf("swapping the file\n");
    SampleData a = ramp(1000), b = ramp(2000);
    Rng rng;
    SampleModel m = voice(&a);
    m.strike(1.0, rng);
    m.tick(rng);
    m.setData(&b);
    check("a new file stops the old playback", !m.playing);
    m.strike(1.0, rng);
    int n = playOut(m, rng, 100000);
    checkEq("and the next strike plays the new file", n, 2000);
}

static void spec()
{
    std::printf("spec\n");
    const ModelSpec& ms = modelSpec(MODEL_SAMPLE);
    check("the model resolves from its uuid", modelFromUuid(ms.uuid, NUM_MODELS) == MODEL_SAMPLE);
    check("tune is centred (0 st at half travel)", std::fabs(knobDefault01(ms.tune) - 0.5) < 1e-9);
    check("decay defaults to full (no envelope)", ms.decay.def == ms.decay.max && SampleModel::decayIsFull(ms.decay.def));
    check("start defaults to the head of the file", ms.shape.def == 0.0);

    // The bank serves the sample model, and every column has its own.
    Voices v;
    SampleData d = ramp(100);
    v.setSample(3, &d);
    v.setModel(3, MODEL_SAMPLE);
    check("the column reports the model", v.model(3) == MODEL_SAMPLE);
    check("and the file", v.sample(3) == &d);
    check("other columns are untouched", v.sample(2) == nullptr);
    v.strikeLane(3, 1.0);
    double out[OUTPUTS];
    v.process(out);
    v.process(out);
    check("and it sounds through the column out", out[2 + 3] != 0.0);
}

int main()
{
    silence();
    passthrough();
    amplitude();
    decayEnvelope();
    pitch();
    rateConversion();
    startOffset();
    retriggerAndChoke();
    swapping();
    spec();
    return finish("sample_test");
}
