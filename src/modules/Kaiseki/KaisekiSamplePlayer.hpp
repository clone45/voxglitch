#pragma once

/*
  KaisekiSamplePlayer.hpp

  Direct copy of TracksSamplePlayer for Kaiseki module.
  A specialized sample player that extends SamplePlayer with advanced looping
  capabilities including one-shot, finite loop, and infinite loop modes.
*/

#include "../../vgLib-2.0/SamplePlayer.hpp"

struct KaisekiSamplePlayer : public SamplePlayer
{
  
  // Loop configuration
  int repeat_count = -1;          // -1 = infinite, 0 = one-shot, N = finite loops
  
  // Loop state tracking
  int current_loop_iteration = 0; // How many times we've looped so far
  bool in_loop_section = true;    // Whether we're currently in the looping part
  bool loop_completed = false;    // Whether finite loops have finished
  
  // Loop boundaries (in samples)
  unsigned int loop_start = 0;
  unsigned int loop_end = 0;      // 0 means use sample end
  
  // Set loop configuration
  void setRepeatCount(int repeat) {
    repeat_count = repeat;
    resetLoopState();
  }
  
  // Set loop boundaries based on cue points
  void setLoopBoundaries(unsigned int start_sample, unsigned int end_sample = 0) {
    loop_start = start_sample;
    loop_end = end_sample; // 0 means use full sample length
    resetLoopState();
  }
  
  // Reset loop state (called when triggering or changing boundaries)
  void resetLoopState() {
    current_loop_iteration = 0;
    in_loop_section = true;
    loop_completed = false;
  }
  
  // Trigger sample playback with optional start position and reverse
  void trigger(float sample_start = 0.0, bool reverse = false) {
    SamplePlayer::trigger(sample_start, reverse);
    resetLoopState();
  }
  
  // Get stereo output - same interface as SamplePlayer
  void getStereoOutput(float *left_output, float *right_output, unsigned int interpolation) {
    SamplePlayer::getStereoOutput(left_output, right_output, interpolation);
  }
  
  // Enhanced step method with loop logic
  void step(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false) {
    if (!playing || !sample.loaded) {
      return;
    }
    
    // Calculate effective loop boundaries
    unsigned int sample_size = sample.size();
    unsigned int effective_loop_start = loop_start;
    unsigned int effective_loop_end = (loop_end > 0) ? loop_end : (unsigned int)(sample_size * sample_end);
    
    // Step the underlying sample player (but don't let it handle looping)
    double sample_increment = getSampleIncrement(pitch);
    playback_position += sample_increment;
    
    // Handle different repeat modes
    if (repeat_count == 0) {
      // One-shot mode: play once to end, no looping
      if (playback_position >= sample_size) {
        stop();
      }
    } else if (repeat_count > 0 && loop_completed) {
      // Finite loops completed, continue to end of file
      if (playback_position >= sample_size) {
        stop();
      }
    } else {
      // In looping section (infinite or finite loops not yet completed)
      if (playback_position >= effective_loop_end) {
        if (repeat_count == -1) {
          // Infinite loop: always loop back
          playback_position = effective_loop_start;
        } else {
          // Finite loop: count iterations
          current_loop_iteration++;
          if (current_loop_iteration >= repeat_count) {
            // Finite loops completed, continue forward
            loop_completed = true;
            in_loop_section = false;
            // Don't loop back, let playback continue
          } else {
            // Still have loops remaining
            playback_position = effective_loop_start;
          }
        }
      }
    }
  }
  
  // Reverse step with loop logic
  void stepReverse(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false) {
    if (!playing || !sample.loaded) {
      return;
    }
    
    // Calculate effective loop boundaries for reverse playback
    unsigned int sample_size = sample.size();
    unsigned int effective_loop_start = loop_start;
    unsigned int effective_loop_end = (loop_end > 0) ? loop_end : (unsigned int)(sample_size * sample_end);
    
    // Step backward
    double sample_increment = getSampleIncrement(pitch);
    playback_position -= sample_increment;
    
    // Handle different repeat modes for reverse playback
    if (repeat_count == 0) {
      // One-shot mode: play once to beginning, no looping
      if (playback_position <= 0) {
        stop();
      }
    } else if (repeat_count > 0 && loop_completed) {
      // Finite loops completed, continue to beginning
      if (playback_position <= 0) {
        stop();
      }
    } else {
      // In looping section (infinite or finite loops not yet completed)
      if (playback_position <= effective_loop_start) {
        if (repeat_count == -1) {
          // Infinite loop: always loop back
          playback_position = effective_loop_end;
        } else {
          // Finite loop: count iterations
          current_loop_iteration++;
          if (current_loop_iteration >= repeat_count) {
            // Finite loops completed, continue backward
            loop_completed = true;
            in_loop_section = false;
            // Don't loop back, let playback continue
          } else {
            // Still have loops remaining
            playback_position = effective_loop_end;
          }
        }
      }
    }
  }
  
  // Jump to a specific position (used for cue point navigation)
  void setPosition(unsigned int position) {
    playback_position = position;
    // Reset loop state when jumping to new position
    resetLoopState();
  }
  
  // Stop playback
  void stop() {
    SamplePlayer::stop();
    resetLoopState();
  }
  
  // Pass-through methods to maintain SamplePlayer interface
  bool loadSample(std::string path) {
    bool result = SamplePlayer::loadSample(path);
    if (result) {
      resetLoopState();
    }
    return result;
  }

  // Apply a pre-loaded sample (for async loading)
  void applySample(std::unique_ptr<Sample> newSample) {
    // Clear current sample
    sample.sample_audio_buffer.clear();

    // Copy the loaded sample data
    sample = *newSample;

    // Reset loop state for new sample
    resetLoopState();
  }
  
  void releaseSample() {
    SamplePlayer::releaseSample();
    resetLoopState();
  }
  
  std::string getFilename() {
    return SamplePlayer::getFilename();
  }
  
  void setFilename(std::string filename) {
    SamplePlayer::setFilename(filename);
  }
  
  std::string getPath() {
    return SamplePlayer::getPath();
  }
  
  void setPath(std::string path) {
    SamplePlayer::setPath(path);
  }
  
  void updateSampleRate() {
    SamplePlayer::updateSampleRate();
  }
  
  unsigned int getSampleRate() {
    return SamplePlayer::getSampleRate();
  }
  
  void setOffset(unsigned int offset) {
    SamplePlayer::setOffset(offset);
  }
  
  bool isLoaded() {
    return SamplePlayer::isLoaded();
  }
  
  void initialize() {
    SamplePlayer::initialize();
    resetLoopState();
  }
  
  // Note: playback_position and playing are inherited from SamplePlayer
  
  // Get loop status information (for debugging/display)
  int getCurrentLoopIteration() const {
    return current_loop_iteration;
  }
  
  bool isLoopCompleted() const {
    return loop_completed;
  }
  
  int getRepeatCount() const {
    return repeat_count;
  }
};