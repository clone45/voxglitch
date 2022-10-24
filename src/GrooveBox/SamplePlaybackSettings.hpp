namespace groove_box
{

struct SamplePlaybackSettings
{
  float volume = default_volume;
  float pan = default_pan;
  float pitch = default_pitch;
  float ratchet = default_ratchet;
  float sample_start = default_sample_start;
  float sample_end = default_sample_end;
  float probability = default_probability;
  float loop = default_loop;
  bool reverse = default_reverse;
  float attack = default_attack;
  float release = default_release;
  float delay_mix = default_delay_mix;
  float delay_length = default_delay_length;
  float delay_feedback = default_delay_feedback;

  void copy(SamplePlaybackSettings *src_settings)
  {
    this->volume = src_settings->volume;
    this->pan = src_settings->pan;
    this->pitch = src_settings->pitch;
    this->ratchet = src_settings->ratchet;
    this->sample_start = src_settings->sample_start;
    this->sample_end = src_settings->sample_end;
    this->probability = src_settings->probability;
    this->loop = src_settings->loop;
    this->reverse = src_settings->reverse;
    this->attack = src_settings->attack;
    this->release = src_settings->release;
    this->delay_mix = src_settings->delay_mix;
    this->delay_length = src_settings->delay_length;
    this->delay_feedback = src_settings->delay_feedback;
  }

  void randomize()
  {
    this->volume = rack::random::uniform(); 	
    this->pan = rack::random::uniform(); 
    this->pitch = rack::random::uniform(); 
    this->ratchet = rack::random::uniform(); 
    this->sample_start = rack::random::uniform(); 
    this->sample_end = rack::random::uniform(); 
    this->probability = rack::random::uniform(); 
    this->loop = rack::random::uniform(); 
    this->reverse = rack::random::uniform(); 
    this->attack = rack::random::uniform(); 
    this->release = rack::random::uniform(); 
    this->delay_mix = rack::random::uniform(); 
    this->delay_length = rack::random::uniform(); 
    this->delay_feedback = rack::random::uniform(); 

    if(sample_start > sample_end) sample_start = 0;
  }


};

}
