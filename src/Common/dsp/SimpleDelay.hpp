#pragma once

#define MAX_BUFFER_SIZE 44100
#define MIN_BUFFER_SIZE 10

struct SimpleDelay
{
  unsigned int write_head = 0;

	float buffer_left[MAX_BUFFER_SIZE];
  float buffer_right[MAX_BUFFER_SIZE];

  float feedback = 0.9;
  float mix = 0.5;
  uint32_t buffer_size = 44100;

	virtual void process(float audio_left, float audio_right, float &read_audio_left, float &read_audio_right)
	{
    unsigned int read_index = write_head + 1;

    if(read_index < sizeof(buffer_left))
    {
      read_audio_left = (mix * buffer_left[read_index]) + ((1.0 - mix) * audio_left);
      read_audio_right = (mix * buffer_right[read_index]) + ((1.0 - mix) * audio_right);
    }
    else
    {
      read_audio_left = audio_left;
      read_audio_right = audio_right;
    }

    write_head++;
    if(write_head >= buffer_size || write_head >= MAX_BUFFER_SIZE) write_head = 0;

    if(feedback == 0)
    {
      buffer_left[write_head] = audio_left;
      buffer_right[write_head] = audio_right;
    }
    else
    {
      float existing_audio_left = buffer_left[write_head];
      float existing_audio_right = buffer_right[write_head];

      // float mixed_audio_left = (existing_audio_left * feedback) + (audio_left * (1.0 - feedback));
      // float mixed_audio_right = (existing_audio_right * feedback) + (audio_right * (1.0 - feedback));

      float mixed_audio_left = (existing_audio_left * feedback) + audio_left;
      float mixed_audio_right = (existing_audio_right * feedback) + audio_right;

      // float mixed_audio = (existing_audio * feedback);
      buffer_left[write_head] = mixed_audio_left;
      buffer_right[write_head] = mixed_audio_right;
    }

	};

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
    if(new_buffer_size > MAX_BUFFER_SIZE) new_buffer_size = MAX_BUFFER_SIZE;
    buffer_size = new_buffer_size;
  }

  void setFeedback(float new_feedback)
  {
    feedback = new_feedback;
  }

  void setMix(float new_mix)
  {
    mix = new_mix;
  }

  void purge()
  {
    for(unsigned int i=0; i < MAX_BUFFER_SIZE; i++)
    {
      buffer_left[i] = 0.0;
      buffer_right[i] = 0.0;
    }
  }
};
