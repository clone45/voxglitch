namespace groove_box
{

  struct SamplePlaybackSettings
  {
    float parameters[NUMBER_OF_FUNCTIONS];

    SamplePlaybackSettings()
    {
      for (unsigned int i = 0; i < NUMBER_OF_FUNCTIONS; i++)
      {
        parameters[i] = default_parameter_values[i];
      }
    }

    float getParameter(unsigned int parameter_index)
    {
      return (parameters[parameter_index]);
    }

    void setParameter(unsigned int parameter_index, float parameter_value)
    {
      parameters[parameter_index] = parameter_value;
    }

    void copy(SamplePlaybackSettings *src_settings)
    {
      for (unsigned int i = 0; i < NUMBER_OF_FUNCTIONS; i++)
      {
        parameters[i] = src_settings->parameters[i];
      }
    }
  };

}
