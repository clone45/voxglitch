namespace scalar_110
{

struct SamplePlaybackSettings
{
  float volume = 0.5;
  float pan = 0.5;
  float pitch = 0.5;
  float ratchet = 0.0;
  float offset = 0.0;
  float probability = 1.0;
  float loop = 0.0;
  bool reverse = false;

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
