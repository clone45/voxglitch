#pragma once

struct Xfade
{
  double crossfade_percentage = 0.0;
  bool crossfading = false;

  // 1/20th of a second
  // double crossfade_rate = 1.0/(APP->engine->getSampleRate()/20.0);

  // double crossfade_rate = .005;

  // double crossfade_rate = 240.0 / APP->engine->getSampleRate();

  double crossfade_rate = 1500.0 / APP->engine->getSampleRate(); // last 1500

  void trigger()
  {
    crossfading = true;
    crossfade_percentage = 0.0;
  }

  void step()
  {
    crossfade_percentage += crossfade_rate;

    if(crossfade_percentage >= 1.0)
    {
      crossfading = false;
      crossfade_percentage = 0.0;
    }
  }

  float process(float from_audio, float to_audio)
  {
    if(crossfading)
    {
      return((from_audio * (1 - crossfade_percentage)) + ((to_audio) * crossfade_percentage));
    }
    else
    {
      return(to_audio);
    }
  }

  // computing crossfade rate
  //
  //  sample_increment

  void setCrossfadeRate(double crossfade_rate)
  {
    this->crossfade_rate = crossfade_rate;
  }
};
