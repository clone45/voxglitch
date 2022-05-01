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
  Track tracks[NUMBER_OF_TRACKS];

  void setSamplePlayer(unsigned int track_index, SamplePlayer *sample_player)
  {
    tracks[track_index].setSamplePlayer(sample_player);
  }

  Track *getTrack(unsigned int track_index)
  {
    return(&tracks[track_index]);
  }

  void copy(MemorySlot *src_memory)
  {
    for(unsigned int i=0; i<NUMBER_OF_TRACKS; i++)
    {
      this->tracks[i].copy(&src_memory->tracks[i]);
    }
  }

};

}
