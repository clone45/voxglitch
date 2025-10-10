#pragma once

#include "AudioFile.h"
#include "../modules/Netrunner/AudioLoader.hpp"  // FFmpeg-based audio loader

struct SampleAudioBuffer
{
  std::vector<float> left_buffer;
  std::vector<float> right_buffer;
  unsigned int interpolation = 1;
  unsigned int virtual_size = 0;

  void clear()
  {
    // Trick to free up memory in buffers
    std::vector<float>().swap(left_buffer);
    std::vector<float>().swap(right_buffer);

    left_buffer.resize(0);
    right_buffer.resize(0);
  }

  void push_back(float audio_left, float audio_right)
  {
    left_buffer.push_back(audio_left);
    right_buffer.push_back(audio_right);
  }

  unsigned int size()
  {
    return(left_buffer.size());
  }

  void read(unsigned int index, float *left_audio_ptr, float *right_audio_ptr)
  {
    if((index >= left_buffer.size()) || (index >= right_buffer.size()))
    {
      *left_audio_ptr = 0;
      *right_audio_ptr = 0;
    }
    else
    {
      *left_audio_ptr = left_buffer[index];
      *right_audio_ptr = right_buffer[index];
    }
  }

  // Read sample using linear interpolation
  void readLI(double position, float *left_audio_ptr, float *right_audio_ptr)
  {
    unsigned int index = std::floor(position); // convert float to int

    // If out of bounds, return zeros
    if((index >= (left_buffer.size() - 1)) || ((index >= right_buffer.size() - 1)))
    {
      *left_audio_ptr = 0;
      *right_audio_ptr = 0;
    }
    // Else, compute and return the interpolated values
    else
    {
      float distance = position - (float) index;
      *left_audio_ptr = left_buffer[index] + ((left_buffer[index + 1] - left_buffer[index]) * distance);
      *right_audio_ptr = right_buffer[index] + ((right_buffer[index + 1] - right_buffer[index]) * distance);
    }
  }
};

struct Sample
{
  std::string path = "";
  std::string filename;
  std::string display_name;
  bool loading = false;
  bool loaded = false;
  bool queued_for_loading = false;
  std::string queued_path = "";
  unsigned int sample_length = 0;
  SampleAudioBuffer sample_audio_buffer;
  float sample_rate = 44100.0;                // This is the sample rate in which the sample was recorded
  unsigned int channels = 0;
  AudioFile<float> audioFile;                 // For loading samples and saving samples

  Sample()
  {
    sample_audio_buffer.clear();
		loading = false;
		filename = "[ empty ]";
    display_name = "[ empty ]";
		path = "";
		sample_rate = 0;
		channels = 0;

    audioFile.setNumChannels(2);
    audioFile.setSampleRate(44100);
  }

  ~Sample()
  {
    sample_audio_buffer.clear();
  }

  bool load(std::string path)
  {
    // Set loading flags
    this->loading = true;
    this->loaded = false;

    // Use FFmpeg AudioLoader
    vgLib_v2::AudioLoader loader;
    vgLib_v2::AudioData audioData;
    std::string error;

    bool success = loader.loadFile(path, audioData, error);

    if (!success)
    {
      WARN("Failed to load sample from %s - %s", path.c_str(), error.c_str());
      this->loading = false;
      this->loaded = false;
      return false;
    }

    // Store audio properties
    this->channels = audioData.channels;
    this->sample_rate = audioData.sampleRate;
    sample_audio_buffer.clear();

    // Convert AudioData (interleaved) to Sample format (separate L/R buffers)
    if (audioData.channels == 1)
    {
      // Mono: duplicate to both channels
      for (int64_t i = 0; i < audioData.totalSamples; i++)
      {
        float sample = audioData.samples[i];
        sample_audio_buffer.push_back(sample, sample);
      }
    }
    else if (audioData.channels == 2)
    {
      // Stereo: de-interleave
      for (int64_t i = 0; i < audioData.totalSamples; i++)
      {
        float left = audioData.samples[i * 2];
        float right = audioData.samples[i * 2 + 1];
        sample_audio_buffer.push_back(left, right);
      }
    }
    else
    {
      // Multi-channel: use first two channels
      for (int64_t i = 0; i < audioData.totalSamples; i++)
      {
        float left = audioData.samples[i * audioData.channels];
        float right = audioData.samples[i * audioData.channels + 1];
        sample_audio_buffer.push_back(left, right);
      }
    }

    // Store sample length and file information
    this->sample_length = sample_audio_buffer.size();
    this->filename = system::getFilename(path);
    this->display_name = filename;

    // Remove file extension from display name (handle various extensions)
    size_t lastDot = this->display_name.find_last_of('.');
    if (lastDot != std::string::npos)
    {
      this->display_name = this->display_name.substr(0, lastDot);
    }

    this->path = path;

    this->loading = false;
    this->loaded = true;

    return true;
  };


  bool isLoaded()
  {
    return(this->loaded);
  }

  // Where to put recording code and how to save it?
  void initialize_recording()
  {
    // Clear out audioFile data.  audioFile represents the audio file information
    // that can be loaded or saved.  In this case, we're going to be saving incoming audio pretty soon.
    audioFile.samples[0].resize(0);
    audioFile.samples[1].resize(0);

    // Also clear out the sample audio information
    sample_audio_buffer.clear();
    sample_length = 0;
  }

  void record_audio(float left, float right)
  {
    // Store incoming audio both in the audioFile for saving and in the sample for playback
    audioFile.samples[0].push_back(left);
    audioFile.samples[1].push_back(right);

    sample_audio_buffer.push_back(left, right);
    sample_length = sample_audio_buffer.size();
  }

  void save_recorded_audio(std::string path)
  {
    audioFile.save(path);
  }

  // Read stereo audio from the buffer at position _index_
  void read(unsigned int index, float *left_audio_ptr, float *right_audio_ptr)
  {
    sample_audio_buffer.read(index, left_audio_ptr, right_audio_ptr);
  }

  // Read sample, applying Linear Interpolation
  void readLI(double position, float *left_audio_ptr, float *right_audio_ptr)
  {
    sample_audio_buffer.readLI(position, left_audio_ptr, right_audio_ptr);
  }

  unsigned int size()
  {
    return(sample_length);
  }

  void setSize(unsigned int sample_length)
  {
    this->sample_length = sample_length;
  }

  float getSampleRate()
  {
    return(this->sample_rate);
  }

  void unload()
  {
    this->sample_audio_buffer.clear();
    this->sample_length = 0;
    this->filename = "";
    this->display_name = "";
    this->loaded = false;
  }

  std::string getFilename()
  {
    return(this->filename);
  }

  std::string getPath()
  {
    return(this->path);
  }

};
