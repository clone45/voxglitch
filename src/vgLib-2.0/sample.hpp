#pragma once

#include "AudioFile.h"
#include "dr_mp3.h"
#include <algorithm>

struct SampleAudioBuffer
{
  std::vector<float> left_buffer;
  std::vector<float> right_buffer;
  unsigned int interpolation = 1;
  unsigned int virtual_size = 0;
  float read_offset = 0.0f;

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

  void setOffset(float offset)
  {
    read_offset = std::max(0.0f, std::min(1.0f, offset)); // Clamp to 0.0-1.0
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

// Helper function to detect MP3 files by extension
static bool isMP3File(std::string path) {
  if (path.length() < 4) return false;
  std::string ext = path.substr(path.length() - 4);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return ext == ".mp3";
}

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
    // Detect file type and route to appropriate loader
    if (isMP3File(path)) {
      return loadMP3(path);
    }

    // Set loading flags
    this->loading = true;
    this->loaded = false;

    // initialize some transient variables
    float left = 0;
    float right = 0;

    // Load the audio file
    if(! audioFile.load(path))
    {
      this->loading = false;
      this->loaded = false;
      return(false);
    }

    // Read details about the loaded sample
    uint32_t sampleRate = audioFile.getSampleRate();
    int numSamples = audioFile.getNumSamplesPerChannel();
    int numChannels = audioFile.getNumChannels();

    this->channels = numChannels;
    this->sample_rate = sampleRate;
    sample_audio_buffer.clear();

    //
    // Copy the sample data from the AudioFile object to floating point vectors
    //
    for(int i = 0; i < numSamples; i++)
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

    // Now that the audioFile has been read into memory, clear it out
    audioFile.samples[0].resize(0);
    audioFile.samples[1].resize(0);

    return(true);
  };

  bool loadMP3(std::string path)
  {
    // Set loading flags
    this->loading = true;
    this->loaded = false;

    drmp3_config config;
    drmp3_uint64 totalFrameCount = 0;

    // Load entire MP3 file into memory as float samples
    float* pSampleData = drmp3_open_file_and_read_pcm_frames_f32(
        path.c_str(),
        &config,
        &totalFrameCount,
        NULL
    );

    if (pSampleData == NULL) {
      this->loading = false;
      this->loaded = false;
      return false;
    }

    // Store metadata
    this->channels = config.channels;
    this->sample_rate = config.sampleRate;
    sample_audio_buffer.clear();

    // Copy samples to our buffer structure
    // drmp3 returns interleaved samples: [L, R, L, R, ...]
    for (drmp3_uint64 i = 0; i < totalFrameCount; i++) {
      float left, right;

      if (config.channels == 2) {
        left = pSampleData[i * 2];
        right = pSampleData[i * 2 + 1];
      } else if (config.channels == 1) {
        left = pSampleData[i];
        right = left; // Mono to stereo
      } else {
        // For >2 channels, just use first two
        left = pSampleData[i * config.channels];
        right = pSampleData[i * config.channels + 1];
      }

      sample_audio_buffer.push_back(left, right);
    }

    // Free the loaded data from dr_mp3
    drmp3_free(pSampleData, NULL);

    // Store sample metadata
    this->sample_length = sample_audio_buffer.size();
    this->filename = system::getFilename(path);
    this->display_name = filename;

    // Remove extension (.mp3)
    if (this->display_name.length() > 4) {
      this->display_name.erase(this->display_name.length() - 4);
    }

    this->path = path;
    this->loading = false;
    this->loaded = true;

    return true;
  }

  bool isLoaded()
  {
    return(this->loaded);
  }

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

  void setOffset(float offset)
  {
    sample_audio_buffer.setOffset(offset);
  }

  float getOffset()
  {
    return(sample_audio_buffer.read_offset);
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
