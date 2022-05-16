#pragma once

struct GrooveBoxExpanderMessage
{
  bool message_received = true;
  bool mutes[8];
  bool solos[8];
  float track_volumes[8];

  GrooveBoxExpanderMessage()
  {
    for(unsigned int i=0; i<8; i++)
    {
      mutes[i] = false;
      solos[i] = false;
      track_volumes[i] = 1.0;
    }
  }
};
