#pragma once

#define MAX_BUFFER_SIZE 44100
#define MIN_BUFFER_SIZE 10

struct SatanonautStereoAudioBuffer
{
  int read_head = 0;
  unsigned int write_head = 0;

	float buffer_left[MAX_BUFFER_SIZE];
  float buffer_right[MAX_BUFFER_SIZE];
  float feedback = 0.0;

  uint32_t buffer_size = 44100;

	SatanonautStereoAudioBuffer()
	{
	}

	virtual ~SatanonautStereoAudioBuffer() {}

	virtual void push(float audio_left, float audio_right)
	{
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

      float mixed_audio_left = (existing_audio_left * feedback) + (audio_left * (1.0 - feedback));
      float mixed_audio_right = (existing_audio_right * feedback) + (audio_right * (1.0 - feedback));

      // float mixed_audio = (existing_audio * feedback);
      buffer_left[write_head] = mixed_audio_left;
      buffer_right[write_head] = mixed_audio_right;
    }

	};

  std::pair<float, float> valueAt(int sample_position)
  {
    if(buffer_size <= 0) buffer_size = 1;
    if(buffer_size > MAX_BUFFER_SIZE) buffer_size = MAX_BUFFER_SIZE - 1;

    float output_left = 0;
    float output_right = 0;

    unsigned int index = sample_position % buffer_size;

    if(index < sizeof(buffer_left)) // very paranoid!
    {
      output_left = buffer_left[index];
      output_right = buffer_right[index];
    }

    return { output_left, output_right };
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
      buffer_left[i] = 0.0;
      buffer_right[i] = 0.0;
    }
  }
};
