#pragma once

#include "AudioFile.h"

struct AudioBuffer
{
  std::vector<float> buffer;

  void clear()
  {
    buffer.resize(0);
  }

  void push_back(float audio)
  {
    buffer.push_back(audio);
  }

  unsigned int size()
  {
    return(buffer.size());
  }

  float read(unsigned int index)
  {
    if(index >= buffer.size()) return 0.0;
    return buffer[index];
  }
};

struct SampleMC
{
	std::string path;
	std::string filename;
	std::string display_name;
	bool loading;
  bool loaded = false;
  bool queued_for_loading = false;
  std::string queued_path = "";
  unsigned int sample_length = 0;

  std::vector<AudioBuffer> audio_buffers;

  // Here, number_of_samples is the number of float values in a sample.
  // I might want to rename this to avoid confusion with "samples" meaning
  // .wav files.
  unsigned int number_of_samples;
  unsigned int number_of_channels;
  unsigned int sample_rate;

  AudioFile<float> audioFile; // For loading samples and saving samples

	SampleMC()
	{
		loading = false;
		filename = "[ empty ]";
    display_name = "[ empty ]";
		path = "";
		sample_rate = 0;
		number_of_channels = 0;

    audioFile.setNumChannels(2);
    audioFile.setSampleRate(44100);
	}

  ~SampleMC()
  {
    for (auto &audio_buffer : audio_buffers) // access by reference to avoid copying
    {
      audio_buffer.clear();
    }
  }

  void load(std::string path)
  {
    this->loading = true;
    this->loaded = false;

    // float audio = 0;

    // If file fails to load, abandon operation
    if(! audioFile.load(path))
    {
      this->loading = false;
      this->loaded = false;
      return;
    }

    // Read details about the loaded sample
    this->number_of_samples = audioFile.getNumSamplesPerChannel();
    this->number_of_channels = audioFile.getNumChannels();
    this->sample_rate = audioFile.getSampleRate();

    for (unsigned int channel_index = 0; channel_index < this->number_of_channels; channel_index++)
    {
      AudioBuffer audio_buffer;

      audio_buffer.clear();

      for (unsigned int sample_index = 0; sample_index < number_of_samples; sample_index++)
      {
        audio_buffer.push_back(audioFile.samples[channel_index][sample_index]);
      }

      audio_buffers.push_back(audio_buffer);
    }

    // Store sample length and file information to this object for the rest
    // of the patch to reference.
    this->sample_length = number_of_samples;
    this->filename = system::getFilename(path);
    this->display_name = filename;
    this->display_name.erase(this->display_name.length()-4); // remove the .wav extension
    this->path = path;

    this->loading = false;
    this->loaded = true;
	};

  float read(unsigned int channel, unsigned int index)
  {
    float audio;

    if (channel < audio_buffers.size())
    {
      audio = audio_buffers[channel].read(index);
    }
    else
    {
      audio = 0;
    }

    return(audio);
  }

  unsigned int size()
  {
    return(sample_length);
  }

};
