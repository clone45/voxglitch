#pragma once

#include "AudioFile.h"

struct Sample
{
	std::string path;
	std::string filename;
	bool loading;
  unsigned int total_sample_count;
	std::vector<float> leftPlayBuffer;
	std::vector<float> rightPlayBuffer;
	unsigned int sample_rate;
	unsigned int channels;
	bool loaded = false;
  AudioFile<float> audioFile; // For loading samples and saving samples

	Sample()
	{
		leftPlayBuffer.resize(0);
		rightPlayBuffer.resize(0);
		loading = false;
		filename = "[ empty ]";
		path = "";
		sample_rate = 0;
		channels = 0;

    audioFile.setNumChannels(2);
    audioFile.setSampleRate(44100);
	}

	virtual ~Sample() {}

	virtual void load(std::string path)
	{
    float left;
    float right;

    if(! audioFile.load(path))
    {
      this->loading = false;
      this->loaded = false;
      return;
    }

    // TODO: check if file loaded, otherwise
    //   this->loading = false;
    //   this->loaded = false;
    //   return

    int sampleRate = audioFile.getSampleRate();
    int numSamples = audioFile.getNumSamplesPerChannel();
    int numChannels = audioFile.getNumChannels();
    // int bitDepth = audioFile.getBitDepth();

    this->channels = numChannels;
    this->sample_rate = sampleRate;
    this->leftPlayBuffer.clear();
    this->rightPlayBuffer.clear();

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

      this->leftPlayBuffer.push_back(left);
      this->rightPlayBuffer.push_back(right);
    }

    this->total_sample_count = leftPlayBuffer.size();
    this->filename = rack::string::filename(path);
    this->path = path;

    this->loading = false;
    this->loaded = true;
	};

  // Where to put recording code and how to save it?
  void initialize_recording()
  {
    // Clear out audioFile data.  audioFile represents the .wav file information
    // that can be loaded or saved.  In this case, we're going to be saving incoming
    // audio pretty soon.
    audioFile.samples[0].resize(0);
    audioFile.samples[1].resize(0);

    // Also clear out the sample audio information
    this->leftPlayBuffer.resize(0);
    this->rightPlayBuffer.resize(0);
  }

  void record_audio(float left, float right)
  {
    // Store incoming audio both in the audioFile for saving and in the sample for playback
    audioFile.samples[0].push_back(left);
    audioFile.samples[1].push_back(right);
    this->leftPlayBuffer.push_back(left);
    this->rightPlayBuffer.push_back(right);

    total_sample_count = leftPlayBuffer.size();
  }

  void save_recorded_audio(std::string path)
  {
    audioFile.save(path);
  }

};
