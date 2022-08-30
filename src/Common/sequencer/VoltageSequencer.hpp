struct VoltageSequencer : Sequencer
{
  std::vector<double> sequence;
  unsigned int voltage_range_index = 0; // see voltage_ranges in DigitalSequencerXP.hpp
  unsigned int snap_division_index = 0;
  bool sample_and_hold = false;

  double voltage_ranges[8][2] = {
    { 0.0, 10.0 },
    { -10.0, 10.0 },
    { 0.0, 5.0 },
    { -5.0, 5.0 },
    { 0.0, 3.0 },
    { -3.0, 3.0 },
    { 0.0, 1.0},
    { -1.0, 1.0}
  };

  double snap_divisions[8] = { 0,8,10,12,16,24,32,26 };

  // constructor
  VoltageSequencer()
  {
    // sequence.assign(sequence_length, 0.0);
  }

  // This must be called before interacting with the voltage sequencer since
  // it sets the size of the vector and initializes it with "value"
  void assign(unsigned int length, double value)
  {
    sequence.assign(length, value);
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
    return(rescale(sequence[index], 0.0, 1.0, voltage_ranges[voltage_range_index][0], voltage_ranges[voltage_range_index][1]));
  }

  double getOutput()
  {
    return(rescale(sequence[getPlaybackPosition()], 0.0, 1.0, voltage_ranges[voltage_range_index][0], voltage_ranges[voltage_range_index][1]));
  }

  void setValue(int index, double value)
  {
    if(snap_division_index > 0)
    {
      double division = 1 / snap_divisions[snap_division_index];
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
      this->setValue(i, rand() / double(RAND_MAX));
    }
  }

  void clear()
  {
    // sequence.fill(0.0);
    sequence.assign(sequence.size(), 0);
  }
};
