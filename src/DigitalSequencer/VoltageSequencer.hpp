struct VoltageSequencer : Sequencer
{
  std::array<double, MAX_SEQUENCER_STEPS> sequence;
  unsigned int voltage_range_index = 0; // see voltage_ranges in DigitalSequencer.h
  unsigned int snap_division_index = 0;
  bool sample_and_hold = false;

  // constructor
  VoltageSequencer()
  {
    sequence.fill(0.0);
  }

  // Returns the 'raw' output from the sequencer, which ranges from 0 to 214,
  // which is the height in pixels of the draw area.  To get the value with
  // the selected range applied, see getOutput()

  double getValue(int index)
  {
    return(sequence[index]);
  }

  // Same as GetValue, but if no index is provided, returns the value at the
  // current playback position.
  double getValue()
  {
    return(sequence[getPlaybackPosition()]);
  }

  // Returns the value of the sequencer at a specific index after the selected
  // voltage range has been applied.
  double getOutput(int index)
  {
    return(rescale(sequence[index], 0.0, DRAW_AREA_HEIGHT, voltage_ranges[voltage_range_index][0], voltage_ranges[voltage_range_index][1]));
  }

  double getOutput()
  {
    return(rescale(sequence[getPlaybackPosition()], 0.0, DRAW_AREA_HEIGHT, voltage_ranges[voltage_range_index][0], voltage_ranges[voltage_range_index][1]));
  }

  void setValue(int index, double value)
  {
    if(snap_division_index > 0)
    {
      double division = DRAW_AREA_HEIGHT / snap_divisions[snap_division_index];
      sequence[index] = division * roundf(value / division);
    }
    else
    {
      sequence[index] = value;
    }
  }

  void shiftLeft()
  {
    double temp = sequence[0];
    for(unsigned int i=0; i < this->sequence_length-1; i++)
    {
      sequence[i] = sequence[i+1];
    }
    sequence[this->sequence_length-1] = temp;
  }

  void shiftRight()
  {
    double temp = sequence[this->sequence_length - 1];

    for(unsigned int i=this->sequence_length-1; i>0; i--)
    {
      sequence[i] = sequence[i-1];
    }

    sequence[0] = temp;
  }

  void randomize()
  {
    for(unsigned int i=0; i < this->sequence_length; i++)
    {
      this->setValue(i, fmod(std::rand(), DRAW_AREA_HEIGHT));
    }
  }
};
