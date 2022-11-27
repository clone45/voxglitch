namespace groove_box
{

//
// Memory
//
// Memory holds a collection of tracks.  Memory slots allow the musician to switch
// between different arrangements.  However, memory slots do not have different
// sample settings.  Each track (track #1, #2 .. #8) has one sample loaded into
// memory, and the sample selection are shared amongst all memory slots.

struct MemorySlot
{
  std::array<Track, NUMBER_OF_TRACKS> tracks;

  void setSamplePlayer(unsigned int track_index, SamplePlayer *sample_player)
  {
    tracks.at(track_index).setSamplePlayer(sample_player);
  }

  // TODO:  I may eventually use an array of slew limiters to avoid having all
  // of these similar functions.
  void setVolumeSlewLimiter(unsigned int track_index, rack::dsp::SlewLimiter *slew_limiter)
  {
    tracks.at(track_index).setVolumeSlewLimiter(slew_limiter);
  }

  void setPanSlewLimiter(unsigned int track_index, rack::dsp::SlewLimiter *slew_limiter)
  {
    tracks.at(track_index).setPanSlewLimiter(slew_limiter);
  }

  void setFilterCutoffSlewLimiter(unsigned int track_index, rack::dsp::SlewLimiter *slew_limiter)
  {
    tracks.at(track_index).setFilterCutoffSlewLimiter(slew_limiter);
  }

  void setFilterResonanceSlewLimiter(unsigned int track_index, rack::dsp::SlewLimiter *slew_limiter)
  {
    tracks.at(track_index).setFilterResonanceSlewLimiter(slew_limiter);
  }

  Track *getTrack(unsigned int track_index)
  {
    return(&tracks.at(track_index));
  }

  void copy(MemorySlot *src_memory)
  {
    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      this->tracks.at(i).copy(&src_memory->tracks[i]);
    }
  }

  void initialize()
  {
    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      this->tracks.at(i).initialize();
    }
  }

};

}
