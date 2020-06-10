#pragma once

struct GrainEngineExpanderMessage
{
  unsigned int sample_slot = 0;
  bool message_received = false;
  std::string path = "";
  std::string filename = "";
};
