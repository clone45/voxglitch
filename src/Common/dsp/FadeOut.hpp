#pragma once

struct FadeOut
{
  float fade_out_counter = 0;
  float fade_out_from = 0;
  float fading_out = false;

  void trigger()
  {
    fade_out_counter = APP->engine->getSampleRate() / 4.0;
    fade_out_from = fade_out_counter;
    fading_out = true;
  }

  void reset()
  {
    fading_out = false;
  }

  bool process(float *left_audio_ptr, float *right_audio_ptr)
  {
    if (! fading_out) return(false);

    fade_out_counter--;

    if(fade_out_counter <= 0)
    {
      // Fade out completed!
      fading_out = false;
      *left_audio_ptr = 0;
      *right_audio_ptr = 0;

      return(true);
    }
    else
    {
      // Fade out not completed.  Return false
      float fade_amount = fade_out_counter / fade_out_from;
      *left_audio_ptr *= fade_amount;
      *right_audio_ptr *= fade_amount;

      return(false);
    }
  }

  void setFadefadeRate(float fade_rate)
  {
    fade_out_counter = APP->engine->getSampleRate() / fade_rate;
  }
};
