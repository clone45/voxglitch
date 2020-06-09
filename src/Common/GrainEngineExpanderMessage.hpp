#pragma once

struct GrainEngineExpanderMessage
{
  unsigned int sample_slot = 0;
  bool message_received = false;
  // Sample *sample = NULL;
  std::string path = "";
  std::string filename = "";
};
