namespace scalar_110
{

//
// Pattern
//
// A pattern is a collection of tracks.  Patterns allow the musician to switch
// between different arrangements.  However, patterns do not have different
// sample settings.  Each track (track #1, #2 .. #8) has one sample loaded into
// memory, and the sample selection are shared amongst all patterns.

struct Pattern
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

};

}
