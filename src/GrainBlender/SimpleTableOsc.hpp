#define MAX_TABLE_SIZE 44100
#define MAX_OSC_VALUE 1.0

struct SimpleTableOsc
{
  float table[MAX_TABLE_SIZE];
  unsigned int waveform = 0;  // 0 == saw wave up, 1 == saw wave down, 2 == triangle wave
  float position = 0;
  float frequency = 6.0;
  unsigned int index = 0;

  SimpleTableOsc()
  {
    for(unsigned int i = 0; i < MAX_TABLE_SIZE; i++)
    {
      table[i] = ((float) i / (float) MAX_TABLE_SIZE) * MAX_OSC_VALUE;
    }
  }

  void setWaveform(unsigned int waveform)
  {
    this->waveform = waveform;
  }

  void setFrequency(float frequency)
  {
    this->frequency = frequency;
  }

  float next()
  {
    if(waveform == 0)
    {
      position = position + this->frequency;
    }

    if(waveform == 1)
    {
      position = position - this->frequency;
    }

    // Roll over the position if necessary
    if(position > MAX_TABLE_SIZE) position = position - MAX_TABLE_SIZE;
    if(position < 0) position = position + MAX_TABLE_SIZE;

    // Convert position to an integer and ensure it is in-bounds
    index = position;
    index = clamp(index, 0, MAX_TABLE_SIZE - 1);

    return(table[index]);
  }

};
