#define MAX_TABLE_SIZE 22050
#define NUMBER_OF_WAVEFORMS 4
#define MAX_OSC_VALUE 1.0

// TODO: Frequency can poop out if it's modulated to roughly

struct SimpleTableOsc
{
  float tables[NUMBER_OF_WAVEFORMS][MAX_TABLE_SIZE];
  unsigned int waveform = 0;  // 0 == saw wave up, 1 == saw wave down, 2 == triangle wave
  float position = 0;
  float frequency = 6.0;
  unsigned int index = 0;

  SimpleTableOsc()
  {
    float angle = 0;

    for(unsigned int i = 0; i < MAX_TABLE_SIZE; i++)
    {
      // Computer values for table #0 (sawtooth wave)
      tables[0][i] = ((float) i / (float) MAX_TABLE_SIZE) * MAX_OSC_VALUE;
      tables[1][i] = ((float) (MAX_TABLE_SIZE - i) / (float) MAX_TABLE_SIZE) * MAX_OSC_VALUE;

      // Pre-calcuate table #1: Triangle wave
      if(i <= (MAX_TABLE_SIZE / 2))
      {
        tables[2][i] = ((float) (i * 2) / (float) MAX_TABLE_SIZE) * MAX_OSC_VALUE;
      }
      else
      {
        tables[2][i] = tables[2][MAX_TABLE_SIZE - i];  // Very clever, Mr. Bond
      }

      // Precalculate table #2: Sine wave
      tables[3][i] = ((MAX_OSC_VALUE / 2) * sin(angle)) + (MAX_OSC_VALUE / 2);
      angle += (2 * M_PI) / MAX_TABLE_SIZE;
    }
  }

  void setWaveform(unsigned int waveform)
  {
    if(waveform < NUMBER_OF_WAVEFORMS) this->waveform = waveform;
  }

  void setFrequency(float frequency)
  {
    if(this->frequency != frequency) DEBUG_FLOAT(frequency);
    this->frequency = frequency;
  }

  float next()
  {
    position = position + this->frequency;

    // Roll over the position if necessary
    if(position > MAX_TABLE_SIZE) position = position - MAX_TABLE_SIZE;
    if(position < 0) position = position + MAX_TABLE_SIZE;

    // Convert position to an integer and ensure it is in-bounds
    index = position;
    index = clamp(index, 0, MAX_TABLE_SIZE - 1);

    return(tables[waveform][index]);
  }

};
