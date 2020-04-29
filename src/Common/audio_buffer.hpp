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

  float getLeftValue(unsigned int sample_position)
  {
    sample_position = sample_position + start;
    if(sample_position >= MAX_BUFFER_SIZE) sample_position = sample_position - MAX_BUFFER_SIZE;
    return(leftPlayBuffer[sample_position]);
  }

  float getRightValue(unsigned int sample_position)
  {
    sample_position = sample_position + start;
    if(sample_position >= MAX_BUFFER_SIZE) sample_position = sample_position - MAX_BUFFER_SIZE;
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
