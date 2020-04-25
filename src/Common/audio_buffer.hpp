#pragma once

#include "dr_wav.h"
#define MAX_BUFFER_SIZE 176400

struct AudioBuffer
{
  unsigned int start = 0;
	drwav_uint64 length = MAX_BUFFER_SIZE;
	float leftPlayBuffer[MAX_BUFFER_SIZE];
	float rightPlayBuffer[MAX_BUFFER_SIZE];

	AudioBuffer()
	{
		// length = 0;
	}

	virtual ~AudioBuffer() {}

	virtual void push(float left_audio, float right_audio)
	{
    start++;
    if(start >= MAX_BUFFER_SIZE) start = 0;

    leftPlayBuffer[start] = left_audio;
    rightPlayBuffer[start] = right_audio;
	};

  float getLeftValue(unsigned int sample_position)
  {
    sample_position = ((sample_position + start) % MAX_BUFFER_SIZE);
    float output_voltage_left = leftPlayBuffer[sample_position];
    return(output_voltage_left);
  }

  float getRightValue(unsigned int sample_position)
  {
    sample_position = ((sample_position + start) % MAX_BUFFER_SIZE);
    float output_voltage_right = rightPlayBuffer[sample_position];
    return(output_voltage_right);
  }
};
