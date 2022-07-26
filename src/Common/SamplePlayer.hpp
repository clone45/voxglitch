/*

common version of SamplePlayer

*/

struct SamplePlayer
{
  Sample sample;
  double playback_position = 0.0f;
  bool playing = false;
  double step_amount = 0.0;

  void getStereoOutput(float *left_output, float *right_output, unsigned int interpolation)
  {
    unsigned int sample_index = playback_position; // convert float to int

    if((playing == false) || ((unsigned int) sample_index >= this->sample.size()) || (sample.loaded == false))
    {
      *left_output = 0;
      *right_output = 0;
    }
    else
    {
      if(interpolation == 0)
      {
        // Normal version, using sample index
        this->sample.read(sample_index, left_output, right_output);
      }
      else
      {
        // Read sample using Linear Interpolation, sending in double
        this->sample.readLI(playback_position, left_output, right_output);
      }
    }
  }

  void trigger(float sample_start = 0, bool reverse = false)
  {
    if(! reverse) // if forward playback
    {
      this->playback_position = sample_start * this->sample.size();
    }
    else // if reverse playback
    {
      this->playback_position = (( 1 - sample_start) * this->sample.size());
    }

    this->playing = true;
  }

  // Left off here.  Have to think about computing the pitch instead
  // of passing in track_pitch
  //
  // Parameters:
  //   pitch - range should be bipolar, ranging from ±5.0
  //   sample_start - unipolor, from 0.0 to 1.0
  //   sample_end - unipolor, from 0.0 to 1.0
  //   loop - boolean (true or false)

  void step(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false)
  {
    if(this->playing && this->sample.loaded)
    {
      playback_position += getSampleIncrement(pitch);

      // If settings loop is greater than 0 and the sample position is past the
      // selected loop length, then loop.  Note:  If loop is set to 1, then
      // the entire sample will loop.

      unsigned int sample_size = sample.size() * sample_end;

      if(loop > 0)
      {
        float loop_position = (sample_start * sample_size) + ((sample_size - sample_start) * loop);
        if(playback_position >= loop_position) playback_position = (sample_start * sample_size);
      }
      else if(playback_position >= sample_size) stop();
    }
  }

  void stepReverse(float pitch = 0.0, float sample_start = 0.0, float sample_end = 1.0, bool loop = false)
	{
    if(this->playing && this->sample.loaded)
    {
      // Step the playback position backward.
  		playback_position -= getSampleIncrement(pitch);

      unsigned int sample_size = sample.size() * sample_end;

      // If the playback position is past the beginning, end or loop sample playback
      if(loop > 0)
      {
        float playback_start = (1.0 - sample_start) * sample_size;
        float loop_position = playback_start - (loop * (sample_size - playback_start));

        if(playback_position <= loop_position) playback_position = playback_start;
      }
      else if(playback_position <= 0) stop();
    }
	}

  double getSampleIncrement(float pitch_cv_input)
  {
    return(this->step_amount * exp2(pitch_cv_input));
  }

  /*
  void step(SamplePlaybackSettings *settings, float track_pitch)
  {
  if(playing && sample.loaded)
  {
  // Step the playback position forward.
  playback_position = playback_position + step_amount; // sample rate playback

  // computed pitch includes pitch from expander
  float computed_pitch = (settings->pitch * 2.0) - 1.0;
  computed_pitch = clamp(computed_pitch + track_pitch, -1.0, 1.0);

  // Add more or less to the playback position based on the pitch setting
  // playback_position = playback_position + (step_amount * ((settings->pitch * 2.0) - 1.0)); // 5 octave range

  playback_position = playback_position + (step_amount * computed_pitch);

  // If settings loop is greater than 0 and the sample position is past the
  // selected loop length, then loop.  Note:  If loop is set to 1, then
  // the entire sample will loop.

  unsigned int sample_size = sample.size() * settings->sample_end;

  if(settings->loop > 0)
  {
  float loop_position = (settings->sample_start * sample_size) + ((sample_size - settings->sample_start) * settings->loop);
  if(playback_position >= loop_position) playback_position = (settings->sample_start * sample_size);
}
else
{
// If the playback position is past the playback length, end sample playback
if(playback_position >= sample_size) stop();
}
}
}
*/
void stepReverse(SamplePlaybackSettings *settings, float track_pitch)
{
  if(playing && sample.loaded)
  {
    // float step_amount = sample.sample_rate / rack_sample_rate;

    // Step the playback position forward.
    playback_position = playback_position - step_amount;

    // computed pitch includes pitch from expander
    float computed_pitch = (settings->pitch * 2.0) - 1.0;
    computed_pitch = clamp(computed_pitch + track_pitch, -1.0, 1.0);

    playback_position = playback_position - (step_amount * computed_pitch); // 5 octave range

    unsigned int sample_size = sample.size() * settings->sample_end;

    // If the playback position is past the beginning, end or loop sample playback
    if(settings->loop > 0)
    {
      float playback_start = (1.0 - settings->sample_start) * sample_size;
      float loop_position = playback_start - (settings->loop * (sample_size - playback_start));

      if(playback_position <= loop_position)
      {
        playback_position = playback_start;
      }
    }
    else
    {
      // If the playback position is past the playback length, end sample playback
      if(playback_position <= 0)
      {
        stop();
      }
    }
  }
}

void stop()
{
  playing = false;
}

void loadSample(std::string path)
{
  sample.load(path);
  updateStepAmount();
}

std::string getFilename()
{
  return(sample.filename);
}

void setFilename(std::string filename)
{
  sample.filename = filename;
}

std::string getPath()
{
  return(sample.path);
}

void setPath(std::string path)
{
  sample.path = path;
}

void updateSampleRate()
{
  // The variable rack_sample_rate is not necessary
  // rack_sample_rate = APP->engine->getSampleRate();
  updateStepAmount();
}

void updateStepAmount()
{
  step_amount = (sample.sample_rate / APP->engine->getSampleRate());
}

void initialize()
{
  sample.unload();
  this->playback_position = 0.0f;
  this->playing = false;
  this->setFilename("");
  this->setPath("");
}
};
