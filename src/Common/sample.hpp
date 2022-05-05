#pragma once

#include "AudioFile.h"

struct SampleAudioBuffer
{
  std::vector<float> left_buffer;
	std::vector<float> right_buffer;

  void clear()
  {
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

  std::pair<float, float> read(unsigned int index)
  {
    if((index >= left_buffer.size()) || (index >= right_buffer.size())) return {0.0, 0.0};
    return {left_buffer[index], right_buffer[index]};
  }
};

struct Sample
{
	std::string path;
	std::string filename;
  std::string display_name;
	bool loading;
  bool loaded = false;
  bool queued_for_loading = false;
  std::string queued_path = "";
  unsigned int sample_length = 0;
  SampleAudioBuffer sample_audio_buffer;
	unsigned int sample_rate;
	unsigned int channels;
  AudioFile<float> audioFile; // For loading samples and saving samples

	Sample()
	{
    sample_audio_buffer.clear();
		loading = false;
		filename = "[ empty ]";
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

  void load(std::string path)
  {
    this->loading = true;
    this->loaded = false;

    float left = 0;
    float right = 0;

    // If file fails to load, abandon operation
    if(! audioFile.load(path))
    {
      this->loading = false;
      this->loaded = false;
      return;
    }

    // Read details about the loaded sample
    int sampleRate = audioFile.getSampleRate();
    int numSamples = audioFile.getNumSamplesPerChannel();
    int numChannels = audioFile.getNumChannels();

    this->channels = numChannels;
    this->sample_rate = sampleRate;
    sample_audio_buffer.clear();

    for (int i = 0; i < numSamples; i++)
    {
      if(numChannels == 2)
      {
        left = audioFile.samples[0][i];  // [channel][sample_index]
        right = audioFile.samples[1][i];
      }
      else if(numChannels == 1)
      {
        left = audioFile.samples[0][i];
        right = left;
      }

      sample_audio_buffer.push_back(left, right);
    }

    // Store sample length and file information to this object for the rest
    // of the patch to reference.
    this->sample_length = sample_audio_buffer.size();
    this->filename = system::getFilename(path);
    this->display_name = filename;
    this->display_name.erase(this->display_name.length()-4); // remove the .wav extension
    this->path = path;

    this->loading = false;
    this->loaded = true;
	};

  // Where to put recording code and how to save it?
  void initialize_recording()
  {
    // Clear out audioFile data.  audioFile represents the .wav file information
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
    if(audioFile.save(path) != true)
    {
      // DEBUG(("Voxglitch sample.hpp::save_recorded_audio() - issue saving file to: " + path).c_str());
    }
  }

  // read(unsigned int index)
  //
  // Usage:
  // ======
  // float left_audio;
  // float right_audio;
  // std::tie(left_audio, right_audio) = sample.getStereoOutput();
  //
  // Output is a float from 0.0 to 1.0

  std::pair<float, float> read(unsigned int index)
  {
    return(sample_audio_buffer.read(index));
  }

  unsigned int size()
  {
    return(sample_length);
  }

};
