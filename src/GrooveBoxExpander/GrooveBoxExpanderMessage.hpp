#pragma once

struct GrooveBoxExpanderMessage
{
  bool message_received = true;
  bool mutes[8];
  bool solos[8];

  GrooveBoxExpanderMessage()
  {
    for(unsigned int i=0; i<8; i++)
    {
      mutes[i] = false;
      solos[i] = false;
    }
  }
};
