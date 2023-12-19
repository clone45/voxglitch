#pragma once

struct StereoFadeOut
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

  //
  // The rate is accumulated until it reaches one.  Larger rates accumulate
  // faster, resulting in shorter fade outs
  //
  // Example rates:
  //
  // sample_time : 1 second
  // sample_time * 2 : 1/2 second
  // sample_time * 4 : 1/4 second
  // sample_time * 8 : 1/8 second
  // sample_time * 16 : 1/16 second

  bool process(float *left_audio_ptr, float *right_audio_ptr, float rate)
  {
    if (! fading_out) return(false);

    ramp -= rate;

    if(ramp <= 0)
    {
      // Fade out completed!
      fading_out = false;
      ramp = 1;
      *left_audio_ptr = 0;
      *right_audio_ptr = 0;

      return(true);
    }
    else
    {
      // Fade out not completed.  Return false
      *left_audio_ptr *= ramp;
      *right_audio_ptr *= ramp;

      return(false);
    }
  }

  bool isFadingOut()
  {
    return(fading_out);
  }
};
