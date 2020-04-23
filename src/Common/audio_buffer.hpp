#pragma once

#include "dr_wav.h"

struct AudioBuffer
{
	drwav_uint64 length;
	std::vector<float> leftPlayBuffer;
	std::vector<float> rightPlayBuffer;

	AudioBuffer()
	{
		leftPlayBuffer.resize(0);
		rightPlayBuffer.resize(0);
		length = 0;
	}

	virtual ~AudioBuffer() {}

	virtual void push(float left_audio, float right_audio)
	{
		leftPlayBuffer.insert(left_audio);
		rightPlayBuffer.insert(right_audio);
    length = leftPlayBuffer.size();
	};

  

};
