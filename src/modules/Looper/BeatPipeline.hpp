// BeatPipeline.hpp
//
// Ellis 2007 dynamic-programming beat tracker with a custom tempo front-end
// and a low-band-energy downbeat selector.
//
// Pipeline:
//   stereo PCM -> sum to mono -> downsample to 22050 Hz
//   -> STFT (N=2048, hop=220, Hann, fps ~= 100.23)
//   -> log-frequency triangular filterbank (24 bands/octave, ~27.5..11000 Hz)
//   -> log10(1 + sum(|X|*F)) per band, per frame
//   -> two spectral-flux ODFs: full-band and low-band (40..150 Hz)
//   -> tempo estimation (integer-bar search for short loops, ACF+Rayleigh for longer)
//   -> half/double parity check against the low-band ODF
//   -> Ellis DP over the full-band ODF
//   -> 4/4 phase fold using low-band energy, beat-1 selection, virtual-t0 projection
//   -> (bpm, beat_one_sec, confidence)
//
// See the research document for full derivation.  Everything here is one-shot
// offline analysis; no state is retained between calls.

#pragma once

#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
#include <dsp/fft.hpp>

namespace beat_detect {

struct BeatResult
{
  bool  valid           = false;
  float bpm             = 120.0f;
  float beat_one_sec    = 0.0f;   // Can be negative (virtual-t0 projection)
  float beat_period_sec = 0.5f;
  float confidence      = 0.0f;   // 0..1
};

// --- constants -------------------------------------------------------------

static const float kWorkSR    = 22050.0f;   // analysis sample rate
static const int   kFFTSize   = 2048;
static const int   kHopSize   = 220;
static const float kFPS       = kWorkSR / (float)kHopSize;  // ~100.23
static const int   kBandsPerOctave = 24;
static const float kFLow      = 27.5f;
static const float kFHigh     = 11000.0f;
static const float kLowBandLo = 40.0f;
static const float kLowBandHi = 150.0f;
static const float kBpmMin    = 60.0f;
static const float kBpmMax    = 200.0f;
static const float kBpmCenter = 120.0f;
static const float kLogNormalSigmaOctaves = 0.6f;
static const float kDPTightness = 100.0f;

// --- helpers ---------------------------------------------------------------

// Log-normal prior on BPM, centered at 120, sigma = 0.6 octaves.
static inline float logNormalPrior(float bpm, float center, float sigma_octaves)
{
  if (bpm <= 0.0f) return 0.0f;
  float r = std::log2(bpm / center);
  return std::exp(-(r * r) / (2.0f * sigma_octaves * sigma_octaves));
}

// Sum ODF values at positions phase, phase+P, phase+2P, ... within [0, odf.size()).
// Returns the mean value at hit frames (0 if no hits).
static inline float gridScore(const std::vector<float>& odf, float period, float phase)
{
  if (odf.empty() || period <= 0.0f) return 0.0f;
  float s = 0.0f;
  int n = 0;
  float t = phase;
  int size = (int)odf.size();
  while (true)
  {
    int i = (int)std::floor(t + 0.5f);
    if (i >= size) break;
    if (i >= 0) { s += odf[i]; ++n; }
    t += period;
  }
  return n > 0 ? s / (float)n : 0.0f;
}

// Fold a BPM into [60, 200] by doubling or halving.
static inline float foldBPM(float bpm)
{
  if (bpm <= 0.0f) return kBpmCenter;
  while (bpm < kBpmMin && bpm > 0.0f) bpm *= 2.0f;
  while (bpm > kBpmMax) bpm *= 0.5f;
  return bpm;
}

// --- mono + resample -------------------------------------------------------

// Simple polyphase-style resample: windowed-sinc lowpass applied via direct
// convolution, then linear interpolation onto the target rate.  Not the
// cheapest possible implementation, but accurate enough and still completes
// in a fraction of a second on samples up to ten minutes.
static std::vector<float> resampleAndMonoize(const std::vector<float>& left,
                                              const std::vector<float>& right,
                                              float in_sr,
                                              float out_sr,
                                              std::atomic<bool>& cancel)
{
  size_t N = std::min(left.size(), right.size());

  // Sum to mono.
  std::vector<float> mono;
  mono.reserve(N);
  for (size_t i = 0; i < N; i++) mono.push_back(0.5f * (left[i] + right[i]));

  if (std::abs(in_sr - out_sr) < 0.5f) return mono;

  // Anti-aliasing lowpass if downsampling. 32-tap Hamming-windowed sinc,
  // cutoff at 0.45 * target_SR (leaves some transition band headroom).
  if (out_sr < in_sr)
  {
    const int taps = 32;
    const float cutoff = 0.45f * out_sr;     // Hz
    const float fc_norm = cutoff / in_sr;    // normalized to input SR

    std::vector<float> h(taps);
    float sum = 0.0f;
    for (int i = 0; i < taps; i++)
    {
      float m = (float)i - 0.5f * (float)(taps - 1);
      float sinc = (m == 0.0f) ? 2.0f * fc_norm
                               : std::sin(2.0f * (float)M_PI * fc_norm * m) / ((float)M_PI * m);
      float w = 0.54f - 0.46f * std::cos(2.0f * (float)M_PI * i / (float)(taps - 1));
      h[i] = sinc * w;
      sum += h[i];
    }
    // Normalize for unity gain at DC.
    if (sum > 0.0f) { for (int i = 0; i < taps; i++) h[i] /= sum; }

    std::vector<float> out(mono.size(), 0.0f);
    int half = taps / 2;
    for (size_t n = 0; n < mono.size(); n++)
    {
      if ((n & 0x3FFF) == 0 && cancel.load()) return {};
      float acc = 0.0f;
      for (int k = 0; k < taps; k++)
      {
        long idx = (long)n + (long)k - half;
        if (idx >= 0 && idx < (long)mono.size())
          acc += mono[idx] * h[k];
      }
      out[n] = acc;
    }
    mono.swap(out);
  }

  // Linear interpolation to target SR.
  double ratio = (double)in_sr / (double)out_sr;
  size_t out_len = (size_t)std::floor((double)mono.size() / ratio);
  std::vector<float> result;
  result.reserve(out_len);
  for (size_t n = 0; n < out_len; n++)
  {
    double t = (double)n * ratio;
    size_t i0 = (size_t)t;
    if (i0 + 1 >= mono.size()) break;
    float frac = (float)(t - (double)i0);
    result.push_back(mono[i0] * (1.0f - frac) + mono[i0 + 1] * frac);
  }
  return result;
}

// --- filterbank ------------------------------------------------------------

struct SparseBand
{
  int start_bin;
  std::vector<float> weights;  // weights[i] applies to bin (start_bin + i)
  float center_hz;
};

struct FilterBank
{
  std::vector<SparseBand> bands;
  std::vector<bool> is_low_band;    // kick band 40..150 Hz
};

static FilterBank buildLogFilterbank(float fs, int fft_size)
{
  FilterBank fb;
  int num_bands = (int)std::ceil(std::log2(kFHigh / kFLow) * (float)kBandsPerOctave);
  fb.bands.resize(num_bands);
  fb.is_low_band.resize(num_bands);

  int num_bins = fft_size / 2 + 1;
  float bin_hz = fs / (float)fft_size;

  std::vector<float> centers(num_bands);
  for (int b = 0; b < num_bands; b++)
    centers[b] = kFLow * std::pow(2.0f, (float)b / (float)kBandsPerOctave);

  for (int b = 0; b < num_bands; b++)
  {
    float lower = (b > 0) ? centers[b - 1] : centers[0] / std::pow(2.0f, 1.0f / (float)kBandsPerOctave);
    float upper = (b < num_bands - 1) ? centers[b + 1] : centers[num_bands - 1] * std::pow(2.0f, 1.0f / (float)kBandsPerOctave);
    float center = centers[b];

    int lo_bin = (int)std::floor(lower / bin_hz);
    int hi_bin = (int)std::ceil (upper / bin_hz);
    lo_bin = std::max(0, lo_bin);
    hi_bin = std::min(num_bins - 1, hi_bin);

    SparseBand& sb = fb.bands[b];
    sb.center_hz = center;
    sb.start_bin = lo_bin;
    sb.weights.reserve(hi_bin - lo_bin + 1);
    for (int k = lo_bin; k <= hi_bin; k++)
    {
      float freq = (float)k * bin_hz;
      float w = 0.0f;
      if (freq >= lower && freq <= upper)
      {
        if (freq <= center && center > lower) w = (freq - lower) / (center - lower);
        else if (freq > center && upper > center) w = (upper - freq) / (upper - center);
      }
      sb.weights.push_back(w);
    }

    fb.is_low_band[b] = (center >= kLowBandLo && center <= kLowBandHi);
  }

  return fb;
}

// --- ODF: STFT + filterbank + spectral flux --------------------------------

struct ODFResult
{
  std::vector<float> full;
  std::vector<float> low;
  int num_frames = 0;
};

static ODFResult computeODFs(const std::vector<float>& mono, std::atomic<bool>& cancel)
{
  ODFResult r;
  if (mono.size() < (size_t)kFFTSize) return r;

  // Pre-compute Hann window
  std::vector<float> window(kFFTSize);
  for (int i = 0; i < kFFTSize; i++)
    window[i] = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * (float)i / (float)(kFFTSize - 1)));

  // Filterbank
  FilterBank fb = buildLogFilterbank(kWorkSR, kFFTSize);
  int num_bands = (int)fb.bands.size();

  // FFT
  rack::dsp::RealFFT fft(kFFTSize);
  std::vector<float> frame(kFFTSize);
  std::vector<float> spectrum(2 * kFFTSize);       // PFFFT rfft output is 2*N ordered
  std::vector<float> magnitude(kFFTSize / 2 + 1);
  std::vector<float> L_prev(num_bands, 0.0f);
  std::vector<float> L_curr(num_bands, 0.0f);

  int num_frames = 1 + ((int)mono.size() - kFFTSize) / kHopSize;
  if (num_frames < 2) return r;

  r.full.reserve(num_frames);
  r.low.reserve(num_frames);

  for (int f = 0; f < num_frames; f++)
  {
    if ((f & 0xFF) == 0 && cancel.load()) { r = ODFResult(); return r; }

    int start = f * kHopSize;
    for (int i = 0; i < kFFTSize; i++)
      frame[i] = mono[start + i] * window[i];

    fft.rfft(frame.data(), spectrum.data());

    // Canonical ordering: [0]=F(0) real, [1]=F(N/2) real, [2k]/[2k+1]=re/im of F(k)
    magnitude[0]          = std::abs(spectrum[0]);
    magnitude[kFFTSize/2] = std::abs(spectrum[1]);
    for (int k = 1; k < kFFTSize / 2; k++)
    {
      float re = spectrum[2 * k];
      float im = spectrum[2 * k + 1];
      magnitude[k] = std::sqrt(re * re + im * im);
    }

    // Per-band log magnitude
    for (int b = 0; b < num_bands; b++)
    {
      const SparseBand& sb = fb.bands[b];
      float sum = 0.0f;
      for (size_t i = 0; i < sb.weights.size(); i++)
        sum += magnitude[sb.start_bin + (int)i] * sb.weights[i];
      L_curr[b] = std::log10(1.0f + sum);
    }

    // Spectral flux: half-wave-rectified difference across bands
    if (f == 0)
    {
      r.full.push_back(0.0f);
      r.low.push_back(0.0f);
    }
    else
    {
      float full_sum = 0.0f;
      float low_sum  = 0.0f;
      for (int b = 0; b < num_bands; b++)
      {
        float diff = L_curr[b] - L_prev[b];
        if (diff > 0.0f)
        {
          full_sum += diff;
          if (fb.is_low_band[b]) low_sum += diff;
        }
      }
      r.full.push_back(full_sum);
      r.low.push_back(low_sum);
    }

    L_prev.swap(L_curr);
  }

  r.num_frames = (int)r.full.size();

  // Normalize each ODF by its std dev (detrend by subtracting mean first).
  auto normalize = [](std::vector<float>& v) {
    if (v.empty()) return;
    double sum = 0.0;
    for (float x : v) sum += x;
    double mean = sum / (double)v.size();
    double var = 0.0;
    for (float x : v) { double d = x - mean; var += d * d; }
    var /= (double)v.size();
    float sd = (float)std::sqrt(var);
    if (sd < 1e-6f) sd = 1e-6f;
    for (float& x : v) x = (float)((x - mean) / sd);
  };
  normalize(r.full);
  normalize(r.low);

  return r;
}

// --- Tempo estimation ------------------------------------------------------

// Integer-bar search for short loops.  Assumes the sample is an integer number
// of bars in 4/4 for bars in {1, 2, 3, 4, 6, 8}.  For each plausible bars
// count, picks the best phase by grid score, boosted by low-band agreement
// and the log-normal prior.
static void tempoIntegerBar(const std::vector<float>& odf_full,
                             const std::vector<float>& odf_low,
                             float duration_sec,
                             float& out_bpm, int& out_phase_frames)
{
  out_bpm = kBpmCenter;
  out_phase_frames = 0;

  const int bars_try[] = {1, 2, 3, 4, 6, 8};
  float best_score = -1e30f;

  for (size_t bi = 0; bi < sizeof(bars_try)/sizeof(bars_try[0]); bi++)
  {
    int bars = bars_try[bi];
    float bpm = 60.0f * 4.0f * (float)bars / duration_sec;
    if (bpm < kBpmMin || bpm > kBpmMax) continue;

    float period_frames = 60.0f * kFPS / bpm;
    int max_phase = (int)std::ceil(period_frames);
    float prior = logNormalPrior(bpm, kBpmCenter, kLogNormalSigmaOctaves);

    for (int p = 0; p < max_phase; p++)
    {
      float full_gs = gridScore(odf_full, period_frames, (float)p);
      float low_gs  = gridScore(odf_low,  period_frames, (float)p);
      float score = full_gs * (1.0f + low_gs) * prior;
      if (score > best_score)
      {
        best_score = score;
        out_bpm = bpm;
        out_phase_frames = p;
      }
    }
  }
}

// Autocorrelation-based tempo estimation with Rayleigh weighting for longer
// samples.  Returns chosen BPM and the best phase for the Ellis DP to start
// from.
static void tempoACF(const std::vector<float>& odf_full,
                      float& out_bpm, int& out_phase_frames)
{
  out_bpm = kBpmCenter;
  out_phase_frames = 0;
  int T = (int)odf_full.size();
  if (T < 16) return;

  int min_lag = (int)std::floor(60.0f * kFPS / kBpmMax);  // ~30
  int max_lag = (int)std::ceil (60.0f * kFPS / kBpmMin);  // ~100
  max_lag = std::min(max_lag, T - 1);
  if (min_lag < 2 || max_lag <= min_lag) return;

  std::vector<float> acf(max_lag + 1, 0.0f);
  for (int lag = min_lag; lag <= max_lag; lag++)
  {
    float s = 0.0f;
    int cnt = T - lag;
    for (int i = 0; i < cnt; i++) s += odf_full[i] * odf_full[i + lag];
    acf[lag] = (cnt > 0) ? s / (float)cnt : 0.0f;
  }

  // Rayleigh weight: r(tau) = (tau / beta^2) * exp(-tau^2 / (2 beta^2)),
  // beta = fps * 60 / 120 (peak at 120 BPM).
  float beta = kFPS * 60.0f / kBpmCenter;
  for (int lag = min_lag; lag <= max_lag; lag++)
  {
    float r = ((float)lag / (beta * beta)) * std::exp(-(float)(lag * lag) / (2.0f * beta * beta));
    acf[lag] *= r;
  }

  // Top-3 peaks
  std::vector<std::pair<float,int> > peaks;
  for (int lag = min_lag + 1; lag < max_lag; lag++)
    if (acf[lag] > acf[lag - 1] && acf[lag] > acf[lag + 1])
      peaks.push_back(std::make_pair(acf[lag], lag));

  std::sort(peaks.begin(), peaks.end(),
            [](const std::pair<float,int>& a, const std::pair<float,int>& b){ return a.first > b.first; });

  float best_score = -1e30f;
  int best_lag = (int)beta;

  int candidates = std::min<int>(3, (int)peaks.size());
  for (int i = 0; i < candidates; i++)
  {
    int lag = peaks[i].second;
    float bpm = 60.0f * kFPS / (float)lag;
    float prior = logNormalPrior(bpm, kBpmCenter, kLogNormalSigmaOctaves);
    float score = peaks[i].first * prior;
    if (score > best_score) { best_score = score; best_lag = lag; }
  }
  if (best_score < -1e29f)
  {
    // No peaks found — fallback to Rayleigh peak
    best_lag = (int)beta;
  }

  out_bpm = foldBPM(60.0f * kFPS / (float)best_lag);

  // Find best phase on the full-band ODF
  float period_frames = 60.0f * kFPS / out_bpm;
  int max_phase = (int)std::ceil(period_frames);
  float best_gs = -1e30f;
  for (int p = 0; p < max_phase; p++)
  {
    float gs = gridScore(odf_full, period_frames, (float)p);
    if (gs > best_gs) { best_gs = gs; out_phase_frames = p; }
  }
}

// Half/double octave correction using full-band grid scores at the current
// period, its halved value (= double tempo), and its doubled value (= half
// tempo).  Compares per-hit average ODF energy:
//
//   - If the finer grid (half period, double tempo) scores almost as high
//     as our current grid, we're at half-tempo -- every one of our beats
//     is a true beat, but we're missing the beats in between.  Double.
//
//   - If the coarser grid (double period, half tempo) scores clearly higher
//     than our current grid, our current grid is alternating between true
//     beats and non-beats -- we're at double-tempo.  Halve.
//
// Uses full-band ODF rather than low-band, because modern 4-on-the-floor
// patterns put kicks on every quarter, which makes the low-band signal
// degenerate exactly in the half-tempo case we're trying to catch.
static void octaveParityCheck(const std::vector<float>& odf_full,
                               float& bpm, float& period_frames, int phase_frames)
{
  if (period_frames < 4.0f) return;

  float score_P = gridScore(odf_full, period_frames, (float)phase_frames);
  if (score_P < 1e-6f) return;

  // Half-tempo error?  Try the double-tempo grid.  A ratio near 1 means the
  // finer grid captures just as much per-hit -- the midpoints are also real
  // beats that we were missing.
  if (bpm * 2.0f <= kBpmMax)
  {
    float score_hP = gridScore(odf_full, period_frames * 0.5f, (float)phase_frames);
    if (score_hP >= 0.93f * score_P)
    {
      bpm *= 2.0f;
      period_frames *= 0.5f;
      return;
    }
  }

  // Double-tempo error?  Try the half-tempo grid.  A ratio clearly above 1
  // means every other of our beats carries significantly more weight than
  // the current grid's per-hit average -- we were counting mids as beats.
  if (bpm * 0.5f >= kBpmMin)
  {
    float score_2P = gridScore(odf_full, period_frames * 2.0f, (float)phase_frames);
    if (score_2P > 1.15f * score_P)
    {
      bpm *= 0.5f;
      period_frames *= 2.0f;
    }
  }
}

// --- Ellis DP --------------------------------------------------------------

static std::vector<int> beatDP(const std::vector<float>& localscore,
                                float period,
                                float tightness)
{
  std::vector<int> beats;
  const int T = (int)localscore.size();
  if (T < 4 || period < 2.0f) return beats;

  int minGap = std::max(1, (int)std::round(period * 0.5f));
  int maxGap = std::max(minGap + 1, (int)std::round(period * 2.0f));

  std::vector<float> cumscore(T, 0.0f);
  std::vector<int>   backlink(T, -1);

  // Precompute transition costs
  std::vector<float> txcost(maxGap + 1, -1e30f);
  for (int g = minGap; g <= maxGap; ++g)
  {
    float logr = std::log((float)g / period);
    txcost[g] = -tightness * logr * logr;
  }

  float score_max = 1e-12f;
  for (int i = 0; i < T; i++) if (localscore[i] > score_max) score_max = localscore[i];
  float scoreThreshFirst = 0.01f * score_max;
  bool firstBeat = true;

  for (int i = 0; i < T; i++)
  {
    float best = -1e30f;
    int bestLoc = -1;

    int lo = std::max(0, i - maxGap);
    int hi = i - minGap;
    for (int loc = hi; loc >= lo; --loc)
    {
      float s = cumscore[loc] + txcost[i - loc];
      if (s > best) { best = s; bestLoc = loc; }
    }

    if (bestLoc < 0)
    {
      cumscore[i] = localscore[i];
      backlink[i] = -1;
    }
    else if (firstBeat && localscore[i] < scoreThreshFirst)
    {
      cumscore[i] = localscore[i];
      backlink[i] = -1;
    }
    else
    {
      cumscore[i] = localscore[i] + best;
      backlink[i] = bestLoc;
      firstBeat = false;
    }
  }

  // Backtrace start: global argmax of cumscore within the trailing period.
  int startSearchFrom = std::max(0, T - (int)std::round(period));
  int startBeat = startSearchFrom;
  float bestCum = cumscore[startBeat];
  for (int i = startSearchFrom + 1; i < T; i++)
    if (cumscore[i] > bestCum) { bestCum = cumscore[i]; startBeat = i; }

  int cur = startBeat;
  while (cur >= 0)
  {
    beats.push_back(cur);
    cur = backlink[cur];
  }
  std::reverse(beats.begin(), beats.end());
  return beats;
}

#if 0
// --- (disabled) multi-feature downbeat selector ---------------------------
// Kept only as reference; the plugin now always returns beat_one_sec = 0
// and relies on the user's ±2-beat nudge knob + Double/Half context menu.

struct DownbeatResult
{
  int   beat_index   = 0;
  int   meter        = 4;
  float confidence   = 0.0f;
  bool  anacrusis    = false;
  float t_beat1_sec  = 0.0f;
};

static DownbeatResult selectDownbeat(const std::vector<int>& beats,
                                      const ODFResult& odfs)
{
  DownbeatResult r;
  int N = (int)beats.size();
  if (N < 2) { r.beat_index = 0; return r; }

  const int window_halfframes = (int)std::round(0.05f * kFPS);  // ~50ms

  // --- per-beat features ---
  std::vector<float> kick(N, 0.0f);
  std::vector<float> snare(N, 0.0f);
  std::vector<float> change(N, 0.0f);
  std::vector<float> chromadist(N, 0.0f);

  auto windowMax = [&](const std::vector<float>& odf, int center)
  {
    int lo = std::max(0, center - window_halfframes);
    int hi = std::min((int)odf.size() - 1, center + window_halfframes);
    float m = 0.0f;
    for (int f = lo; f <= hi; f++) if (odf[f] > m) m = odf[f];
    return m;
  };

  // Kick and snare: per-beat max of their band ODFs.
  for (int i = 0; i < N; i++)
  {
    kick[i]  = windowMax(odfs.low,   beats[i]);
    snare[i] = windowMax(odfs.snare, beats[i]);
  }

  // KL spectral change: accumulate magnitude across each beat interval.
  std::vector<double> prev_spec(kKLBinCount, 0.0);
  std::vector<double> curr_spec(kKLBinCount, 0.0);
  for (int i = 0; i < N; i++)
  {
    int f0 = beats[i];
    int f1 = (i + 1 < N) ? beats[i + 1] : odfs.num_frames;
    f0 = std::max(0, std::min(f0, odfs.num_frames));
    f1 = std::max(f0, std::min(f1, odfs.num_frames));
    std::fill(curr_spec.begin(), curr_spec.end(), 0.0);
    for (int f = f0; f < f1; f++)
    {
      size_t base = (size_t)f * (size_t)kKLBinCount;
      for (int k = 0; k < kKLBinCount; k++)
        curr_spec[k] += odfs.mag_kl[base + (size_t)k];
    }
    if (i > 0) change[i] = (float)beatKLChange(prev_spec, curr_spec);
    prev_spec.swap(curr_spec);
  }

  // Chroma cosine distance: accumulate chroma across each beat interval,
  // L2-normalize, compare to previous beat.
  std::vector<double> prev_chroma(kChromaBins, 0.0);
  std::vector<double> curr_chroma(kChromaBins, 0.0);
  for (int i = 0; i < N; i++)
  {
    int f0 = beats[i];
    int f1 = (i + 1 < N) ? beats[i + 1] : odfs.num_frames;
    f0 = std::max(0, std::min(f0, odfs.num_frames));
    f1 = std::max(f0, std::min(f1, odfs.num_frames));
    std::fill(curr_chroma.begin(), curr_chroma.end(), 0.0);
    for (int f = f0; f < f1; f++)
    {
      size_t base = (size_t)f * (size_t)kChromaBins;
      for (int p = 0; p < kChromaBins; p++)
        curr_chroma[p] += odfs.chroma[base + (size_t)p];
    }
    // L2-normalize
    double norm = 0.0;
    for (int p = 0; p < kChromaBins; p++) norm += curr_chroma[p] * curr_chroma[p];
    norm = std::sqrt(norm);
    if (norm > 1e-9)
      for (int p = 0; p < kChromaBins; p++) curr_chroma[p] /= norm;
    if (i > 0) chromadist[i] = (float)chromaCosineDist(prev_chroma.data(), curr_chroma.data());
    prev_chroma.swap(curr_chroma);
  }

  // Median-normalize all per-beat feature streams.
  medianNormalize(kick);
  medianNormalize(snare);
  medianNormalize(change);
  medianNormalize(chromadist);

  // --- score each (meter, phase) candidate ---
  struct Cand { int M; int phi; float score; };
  std::vector<Cand> candidates;

  const int meters[2] = {4, 3};
  for (int mi = 0; mi < 2; mi++)
  {
    int M = meters[mi];
    for (int phi = 0; phi < M; phi++)
    {
      float s = 0.0f;
      bool first = true;
      for (int i = 0; i < N; i++)
      {
        if (i % M != phi) continue;
        float pos_pen = 0.0f;
        if (first)
        {
          // Small penalty if the first confirming hit is well past the midpoint
          // of the sample: fewer subsequent bars to trust.
          pos_pen = std::max(0.0f, (float)i - 0.5f * (float)N) / (float)N;
          first = false;
        }
        s += kWKick   * kick[i]
           + kWChange * change[i]
           + kWChroma * chromadist[i]
           - kWSnare  * snare[i]
           - kWPos    * pos_pen;
      }
      Cand c = { M, phi, s };
      candidates.push_back(c);
    }
  }

  // --- pick winner with a strong 4/4 bias ---
  Cand best4 = { 4, 0, -1e30f };
  Cand best3 = { 3, 0, -1e30f };
  for (const auto& c : candidates)
  {
    if (c.M == 4 && c.score > best4.score) best4 = c;
    if (c.M == 3 && c.score > best3.score) best3 = c;
  }
  // Only prefer 3/4 when it beats 4/4 by the margin.  Compare on absolute
  // score, but guard against negative values making ratios flip sign.
  bool prefer3 = false;
  if (best3.score > 0.0f && best4.score > 0.0f)
    prefer3 = (best3.score > kMeterMargin34 * best4.score);
  else if (best3.score > 0.0f && best4.score <= 0.0f)
    prefer3 = true;

  Cand winner = prefer3 ? best3 : best4;

  // --- confidence: log margin vs runner-up in the same meter ---
  float runner = -1e30f;
  for (const auto& c : candidates)
  {
    if (c.M == winner.M && c.phi != winner.phi && c.score > runner)
      runner = c.score;
  }
  float wscore = std::max(winner.score, 0.0f) + 1e-3f;
  float rscore = std::max(runner,       0.0f) + 1e-3f;
  float conf = std::log(wscore / rscore);

  // --- featureless-signal fallback ---
  auto rangeOf = [](const std::vector<float>& v) {
    if (v.empty()) return 0.0f;
    float lo = v[0], hi = v[0];
    for (float x : v) { if (x < lo) lo = x; if (x > hi) hi = x; }
    return hi - lo;
  };
  if (rangeOf(kick) < 0.3f && rangeOf(change) < 0.3f && rangeOf(chromadist) < 0.3f)
  {
    // Pure ambient / atonal / silent.  Default to phi=0, M=4, low confidence.
    r.beat_index = 0;
    r.meter      = 4;
    r.confidence = 0.0f;
    r.anacrusis  = false;
    r.t_beat1_sec = (float)beats[0] / kFPS;
    return r;
  }

  // --- resolve beat-1 frame index and time ---
  r.beat_index = winner.phi;
  r.meter      = winner.M;
  r.confidence = conf;

  // Median inter-beat interval (in frames) for virtual-time projection.
  std::vector<int> ibis;
  ibis.reserve(N - 1);
  for (int i = 1; i < N; i++) ibis.push_back(beats[i] - beats[i - 1]);
  std::sort(ibis.begin(), ibis.end());
  float median_ibi_frames = ibis.empty() ? (kFPS * 60.0f / kBpmCenter)
                                          : (float)ibis[ibis.size() / 2];
  float beat_period_sec = median_ibi_frames / kFPS;

  int phi = winner.phi;
  float first_hit_sec = (float)beats[phi] / kFPS;

  // Anacrusis handling: if confidence is low and we chose phi > 0, consider
  // projecting beat 1 backwards (possibly before the sample start).  This
  // matches Ableton Warp's convention for pickup bars.
  if (phi > 0 && conf < kConfLow)
  {
    float t_virtual = (float)beats[0] / kFPS - (float)phi * beat_period_sec;
    if (t_virtual > -2.0f * beat_period_sec)
    {
      r.t_beat1_sec = t_virtual;
      r.anacrusis = true;
      return r;
    }
  }

  // Default: use the phi-th beat as beat 1, but fold into [-bar/2, +bar/2]
  // so that the reported value sits near t=0 -- friendlier to the ±2-beat
  // nudge knob than raw absolute time deep into the sample.
  float bar_sec = (float)winner.M * beat_period_sec;
  if (bar_sec > 1e-6f)
    r.t_beat1_sec = first_hit_sec - std::round(first_hit_sec / bar_sec) * bar_sec;
  else
    r.t_beat1_sec = first_hit_sec;

  return r;
}
#endif  // disabled downbeat selector

// --- Top-level pipeline ----------------------------------------------------

static BeatResult runBeatPipeline(const std::vector<float>& left,
                                   const std::vector<float>& right,
                                   float in_sr,
                                   std::atomic<bool>& cancel)
{
  BeatResult out;

  if (left.empty() || right.empty() || in_sr <= 0.0f) return out;

  // 1) Mono + downsample to 22050 Hz
  std::vector<float> mono = resampleAndMonoize(left, right, in_sr, kWorkSR, cancel);
  if (cancel.load()) return out;
  if (mono.size() < (size_t)(kFFTSize * 2)) return out;  // Need at least a few frames

  float duration_sec = (float)mono.size() / kWorkSR;

  // 2) ODFs
  ODFResult odfs = computeODFs(mono, cancel);
  if (cancel.load()) return out;
  if (odfs.num_frames < 8) return out;

  // 3) Tempo
  float bpm = kBpmCenter;
  int tempo_phase = 0;
  if (duration_sec < 8.0f)
    tempoIntegerBar(odfs.full, odfs.low, duration_sec, bpm, tempo_phase);
  else
    tempoACF(odfs.full, bpm, tempo_phase);

  bpm = foldBPM(bpm);
  float period_frames = 60.0f * kFPS / bpm;

  // 4) Half/double parity gate.  Run twice in case the first pass corrects
  //    by one octave and the second catches a rare double-octave error.
  octaveParityCheck(odfs.full, bpm, period_frames, tempo_phase);
  octaveParityCheck(odfs.full, bpm, period_frames, tempo_phase);
  bpm = foldBPM(bpm);
  period_frames = 60.0f * kFPS / bpm;

  if (cancel.load()) return out;

  // 5) Ellis DP - we still run it, because the grid-score gates in the
  //    octave check upstream are only reliable if the tempo actually
  //    resolves to a consistent beat sequence.  The beat list itself is
  //    currently unused downstream -- nudge is always left at zero.
  std::vector<int> beats = beatDP(odfs.full, period_frames, kDPTightness);
  if (beats.size() < 2) return out;

  float period_sec = 60.0f / bpm;

  out.valid           = true;
  out.bpm             = bpm;
  out.beat_one_sec    = 0.0f;  // user will nudge manually
  out.beat_period_sec = period_sec;
  out.confidence      = 1.0f;
  return out;
}

} // namespace beat_detect
