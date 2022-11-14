namespace groove_box
{

struct SamplePlaybackSettings
{
  float parameters[NUMBER_OF_FUNCTIONS];

  SamplePlaybackSettings()
  {
    for(unsigned int i=0; i<NUMBER_OF_FUNCTIONS; i++)
    {
      parameters[i] = default_parameter_values[i];
    }
  }

  float getParameter(unsigned int parameter_index)
  {
    return(parameters[parameter_index]);
  }

  void setParameter(unsigned int parameter_index, float parameter_value)
  {
    parameters[parameter_index] = parameter_value;
  }  
  /*
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
  float filter_cutoff = default_filter_cutoff;
  float filter_resonance = default_filter_resonance;
  */

  void copy(SamplePlaybackSettings *src_settings)
  {
    for(unsigned int i=0; i<NUMBER_OF_FUNCTIONS; i++)
    {
      parameters[i] = src_settings->parameters[i];
    }
    /*
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
    this->filter_cutoff = src_settings->filter_cutoff;
    this->filter_resonance = src_settings->filter_resonance;
    */
  }
};

}
