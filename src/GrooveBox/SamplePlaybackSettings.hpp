namespace groove_box
{

struct SamplePlaybackSettings
{
  float volume = default_volume;
  float pan = default_pan;
  float pitch = default_pitch;
  float ratchet = default_ratchet;
  float offset = default_offset;
  float probability = default_probability;
  float loop = default_loop;
  bool reverse = default_reverse;

  void copy(SamplePlaybackSettings *src_settings)
  {
    this->volume = src_settings->volume;
    this->pan = src_settings->pan;
    this->pitch = src_settings->pitch;
    this->ratchet = src_settings->ratchet;
    this->offset = src_settings->offset;
    this->probability = src_settings->probability;
    this->loop = src_settings->loop;
    this->reverse = src_settings->reverse;
  }
};

}
