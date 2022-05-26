#pragma once

struct ExpanderToGrooveboxMessage
{
  bool message_received = true;
  bool mutes[8];
  bool solos[8];
  float track_volumes[8];
  float track_pans[8];
  float track_pitches[8];

  ExpanderToGrooveboxMessage()
  {
    for(unsigned int i=0; i<8; i++)
    {
      mutes[i] = false;
      solos[i] = false;
      track_volumes[i] = 1.0;
      track_pans[i] = 0.0; // track pans range from -1 to 1
      track_pitches[i] = 1.0;
    }
  }
};
