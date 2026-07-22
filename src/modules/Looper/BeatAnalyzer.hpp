// BeatAnalyzer.hpp
//
// Thin wrapper around runBeatPipeline().  Analysis runs synchronously on
// the UI thread when analyze() is called; the audio thread picks up the
// result via tryConsume() on its next process() tick.  The atomic +
// mutex pair protects the hand-off from UI thread → audio thread.

#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "BeatPipeline.hpp"

namespace beat_detect {

class BeatAnalyzer
{
public:
  BeatAnalyzer() {}

  BeatAnalyzer(const BeatAnalyzer&) = delete;
  BeatAnalyzer& operator=(const BeatAnalyzer&) = delete;

  // Runs the beat-detection pipeline synchronously on the calling thread
  // and stores the result.  tryConsume() will return true on the next
  // audio-thread tick so the module can apply the BPM to its params.
  void analyze(const std::vector<float>& left,
               const std::vector<float>& right,
               float sample_rate)
  {
    result_ready_.store(false);
    BeatResult r = runBeatPipeline(left, right, sample_rate, cancel_flag_);
    {
      std::lock_guard<std::mutex> lk(result_mutex_);
      result_ = r;
    }
    result_ready_.store(true);
  }

  // If a result is ready, copies it into `out` and clears the flag.  Safe
  // to call from the audio thread.
  bool tryConsume(BeatResult& out)
  {
    if (!result_ready_.exchange(false)) return false;
    std::lock_guard<std::mutex> lk(result_mutex_);
    out = result_;
    return true;
  }

private:
  // runBeatPipeline takes a reference to an atomic cancel flag so it can
  // bail early from long-running work.  In the synchronous configuration
  // we never set it, but we still need a valid atomic to pass in.
  std::atomic<bool> cancel_flag_{false};
  std::atomic<bool> result_ready_{false};
  std::mutex        result_mutex_;
  BeatResult        result_;
};

} // namespace beat_detect
