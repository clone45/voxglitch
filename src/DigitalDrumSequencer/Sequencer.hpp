struct Sequencer
{
  unsigned int sequence_length = 16;
  unsigned int sequence_playback_position = 0;

  void step()
  {
    sequence_playback_position = (sequence_playback_position + 1) % sequence_length;
  }

  void reset()
  {
    sequence_playback_position = 0;
  }

  unsigned int getPlaybackPosition()
  {
    return(sequence_playback_position);
  }

  unsigned int getLength()
  {
    return(sequence_length);
  }

  void setLength(unsigned int sequence_length)
  {
    this->sequence_length = sequence_length;
  }
};
