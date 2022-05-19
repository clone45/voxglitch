#pragma once

struct GrooveBoxMessage
{
  bool track_triggers[8];
  bool message_received = true;

  GrooveBoxMessage()
  {
    for(unsigned int i=0; i<8; i++)
    {
      track_triggers[i] = false;
    }
  }
};
