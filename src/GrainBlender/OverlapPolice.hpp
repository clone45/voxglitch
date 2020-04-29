#include <bitset>

struct OverlapPolice
{
  std::bitset<MAX_BUFFER_SIZE> playback_position_log;

  void logPlaybackPosition(unsigned int playback_position)
  {
    playback_position = playback_position % MAX_BUFFER_SIZE;
    playback_position_log[playback_position] = 1;
  }

  bool checkPlaybackPosition(unsigned int playback_position)
  {
    playback_position = playback_position % MAX_BUFFER_SIZE;

    if((playback_position_log[playback_position] % MAX_BUFFER_SIZE == 1) ||
       (playback_position_log[playback_position + 1] % MAX_BUFFER_SIZE == 1) ||
       (playback_position_log[playback_position - 1] % MAX_BUFFER_SIZE == 1) ||
       (playback_position_log[playback_position + 2] % MAX_BUFFER_SIZE == 1) ||
       (playback_position_log[playback_position - 2] % MAX_BUFFER_SIZE == 1))
    {
      return(true);
    }
    else
    {
      return(false);
    }

    return(false);
  }

  void reset()
  {
    playback_position_log.reset();
  }
};
