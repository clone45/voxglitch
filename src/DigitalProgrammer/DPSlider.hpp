struct DPSlider
{
  double value = 0.0;
  unsigned int snap_division_index = 0;

  // constructor
  DPSlider()
  {
  }

  // Returns the 'raw' output from the sequencer, which ranges from 0 to 214,
  // which is the height in pixels of the draw area.  To get the value with
  // the selected range applied, see getOutput()

  double getValue()
  {
    return(value);
  }

  double getOutput(unsigned int voltage_range_index)
  {
    return(rescale(value, 0.0, 1.0, voltage_ranges[voltage_range_index][0], voltage_ranges[voltage_range_index][1]));
  }

  void setValue(double value)
  {
    this->value = value;
  }
};
