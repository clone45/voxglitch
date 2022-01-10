struct DPSlider
{
  double value = 0.0;

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

  void setValue(double value)
  {
    this->value = value;
  }
};
