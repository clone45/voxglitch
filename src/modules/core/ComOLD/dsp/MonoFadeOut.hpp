#pragma once

struct MonoFadeOut
{
  float ramp = 1;
  bool fading_out = false;

  void trigger()
  {
    fading_out = true;
    ramp = 1;
  }

  void reset()
  {
    fading_out = false;
  }

  bool process(float *audio_ptr, float rate)
  {
    if (! fading_out) return(false);

    ramp -= rate;

    if(ramp <= 0)
    {
      // Fade out completed!
      fading_out = false;
      ramp = 1;
      *audio_ptr = 0;

      return(true);
    }
    else
    {
      // Fade out not completed.  Return false
      *audio_ptr *= ramp;

      return(false);
    }
  }

  bool isFadingOut()
  {
    return(fading_out);
  }
};
