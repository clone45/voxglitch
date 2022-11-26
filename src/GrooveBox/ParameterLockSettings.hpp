namespace groove_box
{

  struct ParameterLockSettings
  {
    private:
    
      std::array<float, NUMBER_OF_PARAMETER_LOCKS> parameters;

    public:

      ParameterLockSettings()
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

      void copy(ParameterLockSettings *src_settings)
      {
        parameters = src_settings->parameters;
      }
  };

}
