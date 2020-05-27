#pragma once

struct GrainEngineExpanderMessage
{
	std::string path;
  unsigned int sample_slot = 0;
  bool message_received = false;
};
