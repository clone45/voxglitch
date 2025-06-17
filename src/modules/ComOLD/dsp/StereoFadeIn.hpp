#pragma once

struct StereoFadeIn
{
  float ramp = 0;
  bool fading_in = false;

  void trigger()
  {
    fading_in = true;
    ramp = 0;
  }

  void reset()
  {
    fading_in = false;
  }

  bool process(float *left_audio_ptr, float *right_audio_ptr, float rate)
  {
    if (! fading_in) return(false);

    ramp += rate;

    // If fade in has completed, stop fading in
    if(ramp >= 1)
    {
      fading_in = false;
      ramp = 0;

      // Return true.  This value can be used by the main code to do something
      // once the fade in has completed.
      return(true);
    }
    // .. otherwise, continue to fade in
    else
    {
      // Fade out not completed.  Return false
      *left_audio_ptr *= ramp;
      *right_audio_ptr *= ramp;

      // Return false because we're not done ramping yet
      return(false);
    }
  }

  bool isFadingIn()
  {
    return(fading_in);
  }
};
