#pragma once

#define MAX_BUFFER_SIZE 44100
#define MIN_BUFFER_SIZE 10

struct SatanonautAudioBuffer
{
  int read_head = 0;
  unsigned int write_head = 0;

	float playBuffer[MAX_BUFFER_SIZE];
  int sample_position = 0;
  float feedback = 0.0;

  uint32_t buffer_size = 44100;

	SatanonautAudioBuffer()
	{
	}

	virtual ~SatanonautAudioBuffer() {}

	virtual void push(float audio)
	{
    write_head++;
    if(write_head >= buffer_size || write_head >= MAX_BUFFER_SIZE) write_head = 0;

    if(feedback == 0)
    {
      playBuffer[write_head] = audio;
    }
    else
    {
      float existing_audio = playBuffer[write_head];
      float mixed_audio = (existing_audio * feedback) + (audio * (1.0 - feedback));
      playBuffer[write_head] = mixed_audio;
    }

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

  void setFeedback(float new_feedback)
  {
    feedback = new_feedback;
  }

  unsigned int getWriteHead()
  {
    return(write_head);
  }

  void purge()
  {
    for(unsigned int i=0; i < MAX_BUFFER_SIZE; i++)
    {
      playBuffer[i] = 0.0;
    }
  }
};
