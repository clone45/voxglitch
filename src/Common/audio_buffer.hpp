#pragma once

#include "dr_wav.h"
#define MAX_BUFFER_SIZE 176400

struct AudioBuffer
{
  unsigned int start = 0;
	float leftPlayBuffer[MAX_BUFFER_SIZE];
	float rightPlayBuffer[MAX_BUFFER_SIZE];
  unsigned int buffer_size = MAX_BUFFER_SIZE;
  bool frozen = false;

	AudioBuffer()
	{
	}

	virtual ~AudioBuffer() {}

	virtual void push(float left_audio, float right_audio)
	{
    if(! frozen)
    {
      start++;
      if(start >= MAX_BUFFER_SIZE) start = 0;

      leftPlayBuffer[start] = left_audio;
      rightPlayBuffer[start] = right_audio;
    }
	};

  unsigned int incrementPlaybackPosition(unsigned int sample_position)
  {
    sample_position = sample_position + start;
    if(sample_position > MAX_BUFFER_SIZE) sample_position -= MAX_BUFFER_SIZE;
    clamp(sample_position, 0, MAX_BUFFER_SIZE);
    return(sample_position);
  }

  float getLeftValue(unsigned int sample_position)
  {
    sample_position = incrementPlaybackPosition(sample_position);
    return(leftPlayBuffer[sample_position]);
  }

  float getRightValue(unsigned int sample_position)
  {
    sample_position = incrementPlaybackPosition(sample_position);
    return(rightPlayBuffer[sample_position]);
  }

  void setBufferSize(unsigned int buffer_size)
  {
    this->buffer_size = buffer_size;
  }

  unsigned int getBufferSize()
  {
    return(this->buffer_size);
  }
};
