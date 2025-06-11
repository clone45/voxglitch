#pragma once

struct GrooveboxToExpanderMessage
{
  bool track_triggers[8];
  bool message_received = true;

  GrooveboxToExpanderMessage()
  {
    for(unsigned int i=0; i<8; i++)
    {
      track_triggers[i] = false;
    }
  }
};
