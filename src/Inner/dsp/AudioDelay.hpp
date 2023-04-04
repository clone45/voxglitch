#define MAX_BUFFER_SIZE 44100
#define MIN_BUFFER_SIZE 10

#include <stdint.h>

struct AudioDelay
{
  unsigned int write_head = 0;

  float buffer[MAX_BUFFER_SIZE];

  float feedback = 0.9;
  float mix = 0.5;
  uint32_t buffer_size = 44100;

  AudioDelay()
  {
    for(unsigned int i=0; i<MAX_BUFFER_SIZE; i++)
    {
      buffer[i] = 0.0;
    }
  }

  float process(float audio)
  {
    unsigned int read_index = (write_head + 1) % MAX_BUFFER_SIZE;
    float read_audio = 0.0;

    if (read_index < MAX_BUFFER_SIZE)
    {
      read_audio = (mix * buffer[read_index]) + ((1.0 - mix) * audio);
    }
    else
    {
      read_audio = audio;
    }

    // Defensive programming in case the audio explodes for some reason
    read_audio = clamp(read_audio, -100.0, 100.0);

    // Increment delay write head.  Wrap it to 0 if it's out of bounds.
    write_head++;
    if (write_head >= buffer_size || write_head >= MAX_BUFFER_SIZE) write_head = 0;

    if (feedback == 0)
    {
      buffer[write_head] = audio;
    }
    else
    {
      float existing_audio = buffer[write_head];
      float mixed_audio = (existing_audio * feedback) + audio;

      buffer[write_head] = mixed_audio;
    }

    return(read_audio);
  };

  uint32_t getBufferSize()
  {
    return (buffer_size);
  }

  uint32_t getMaxBufferSize()
  {
    return (MAX_BUFFER_SIZE);
  }

  void setBufferSize(uint32_t new_buffer_size)
  {
    if (new_buffer_size > MAX_BUFFER_SIZE) new_buffer_size = MAX_BUFFER_SIZE;
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
    for (unsigned int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
      buffer[i] = 0.0;
    }
  }
};