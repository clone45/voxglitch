namespace scalar_110
{
  struct Engine
  {
    virtual std::pair<float, float> process() = 0;
    virtual void trigger(StepParams *step_parameters) = 0;
    virtual void reset() = 0;
  };
}
