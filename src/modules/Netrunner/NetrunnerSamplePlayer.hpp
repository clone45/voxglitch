#pragma once

/*
  NetrunnerSamplePlayer.hpp

  A specialized sample player for the Netrunner module that extends SamplePlayer
  with support for async sample loading via applySample().
*/

#include "vgLib-2.0/SamplePlayer.hpp"

struct NetrunnerSamplePlayer : public SamplePlayer
{
  // Apply a pre-loaded sample (for async loading)
  void applySample(std::unique_ptr<Sample> newSample) {
    // Clear current sample buffer
    sample.sample_audio_buffer.clear();

    // Copy the loaded sample data
    sample = *newSample;

    // Update step amount for the new sample
    updateStepAmount();
  }
};
