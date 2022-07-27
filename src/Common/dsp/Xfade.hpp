#pragma once

struct Xfade
{
  // Old code
  // =========
  // double smooth_constant = 128.0;
  // double crossfade_rate = (crossfade_constant / APP->engine->getSampleRate());


  // The HIGHER the smooth_constant, the faster the smoothing will take effect
  // 2048 seems to work well and doesn't have a noticeable affect on the
  // snappiness of drum sounds.  Low values, such as 256 or lower, will have
  // a dramatic affect on the punch of drums.
  double crossfade_constant = 2048.0;
  double crossfade_percentage = 0.0;
  bool crossfading = false;

  double crossfade_rate = .05;

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

};
