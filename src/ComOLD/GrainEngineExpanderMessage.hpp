#pragma once

struct GrainEngineExpanderMessage
{
  unsigned int sample_slot = 0;
  bool message_received = true;
  std::string path = "";
  std::string filename = "";
};
