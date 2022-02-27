/*
the sample bank holds a vector of samples (check!)
the sample bank contains the code to load samples given a folder (check!)
have 12 different sample players, one per track (check!)
sample players point to one of the samples in the sample bank (almost!)
  -- the pointer can change quickly if a Sample Engine's sample selector is modulated
each sample player is responsible for sample playback functions
SampleBank has pass-through playback and change sample methods
*/

#pragma once

struct SampleBank
{
private:
  SampleBank() { }

public:

	std::vector<Sample> samples;
  SamplePtrPlayer sample_ptr_players[NUMBER_OF_TRACKS];
  std::string loaded_filenames[NUMBER_OF_SAMPLES] = {""};

  // Code for making Sample Bank a "Singleton"
  // =====================================================
  static SampleBank& get_instance()
  {
    static SampleBank instance;
    return instance;
  }

  // The copy constructor is deleted, to prevent client code from creating new
  // instances of this class by copying the instance returned by get_instance()
  SampleBank(SampleBank const&) = delete;

  // The move constructor is deleted, to prevent client code from moving from
  // the object returned by get_instance(), which could result in other clients
  // retrieving a reference to an object with unspecified state.
  SampleBank(SampleBank&&) = delete;

  // =====================================================


  void assign(unsigned int sample_number, unsigned int track_number)
  {
      sample_ptr_players[track_number].assign(&samples[sample_number]);
  }

  void trigger(unsigned int track_number)
  {
    sample_ptr_players[track_number].trigger();
  }

  void step(float rack_sample_rate)
  {
    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      sample_ptr_players[i].step(rack_sample_rate);
    }
  }

	std::pair<float, float> getOutput(unsigned int track_number)
  {
    return sample_ptr_players[track_number].getStereoOutput();
  }

  void loadSamplesFromPath(const char *path)
  {
    // Clear out any old .wav files
    // Reminder: this->samples is a vector, and vectors have a .clear() menthod.
    // this->samples.clear();

    // Load all .wav files found in the folder specified by 'path'
    // this->rootDir = std::string(path);

    std::vector<std::string> dirList = system::getEntries(path);

    // TODO: Decide on a maximum memory consuption allowed and abort if
    // that amount of member would be exhausted by loading all of the files
    // in the folder.
    for (auto file : dirList)
    {
      if (
        // Something happened in Rack 2 where the extension started to include
        // the ".", so I decided to check for both versions, just in case.
        (rack::string::lowercase(system::getExtension(file)) == "wav") ||
        (rack::string::lowercase(system::getExtension(file)) == ".wav")
      )
      {
        // Create new multi-channel sample.  This structure is defined in Common/sample_mc.hpp
        Sample new_sample;

        // Load the sample data from the disk
        new_sample.load(file);

        // Reminder: .push_back is a method of vectors that pushes the object
        // to the end of a list.
        this->samples.push_back(new_sample);
      }
    }
  }


};
