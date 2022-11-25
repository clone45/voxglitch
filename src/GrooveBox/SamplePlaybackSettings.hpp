namespace groove_box
{

  struct SamplePlaybackSettings
  {
    private:
    
      std::array<float, NUMBER_OF_FUNCTIONS> parameters;

    public:

      SamplePlaybackSettings()
      {
        parameters = default_parameter_values;
      }

      float getParameter(unsigned int parameter_index)
      {
        return (parameters.at(parameter_index));
      }

      void setParameter(unsigned int parameter_index, float parameter_value)
      {
        parameters.at(parameter_index) = parameter_value;
      }

      void copy(SamplePlaybackSettings *src_settings)
      {
        parameters = src_settings->parameters;
      }
  };

}
