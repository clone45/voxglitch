#pragma once

#define MAX_BUFFER_SIZE 44100

struct GalactoAudioBuffer
{
  int read_head = 0;
  unsigned int write_head = 0;

	float playBuffer[MAX_BUFFER_SIZE];
  int sample_position = 0;

  uint32_t buffer_size = 44100;

	GalactoAudioBuffer()
	{
	}

	virtual ~GalactoAudioBuffer() {}

	virtual void push(float audio)
	{
    write_head++;
    if(write_head >= buffer_size || write_head >= MAX_BUFFER_SIZE) write_head = 0;
    playBuffer[write_head] = audio;
	};

  float getOutput(int sample_position)
  {
    if(buffer_size <= 0) buffer_size = 1;
    if(buffer_size > MAX_BUFFER_SIZE) buffer_size = MAX_BUFFER_SIZE - 1;

    unsigned int index = 0;
    float output = 0;

    index = sample_position % buffer_size;
    if(index < sizeof(playBuffer)) output = playBuffer[index]; // very paranoid!

    return(output);
  }

  uint32_t getBufferSize()
  {
    return(buffer_size);
  }

  uint32_t getMaxBufferSize()
  {
    return(MAX_BUFFER_SIZE);
  }

  void setBufferSize(uint32_t new_buffer_size)
  {
    buffer_size = new_buffer_size;
  }
};