namespace scalar_110
{
  struct Engine
  {
    std::string knob_labels[8] = {"placeholder","placeholder","placeholder","placeholder","placeholder","placeholder","placeholder","placeholder"};
    virtual std::pair<float, float> process() = 0;
    virtual void trigger(StepParams *step_parameters) = 0;
  };

  struct engineWidget : ModuleWidget
  {
    engineWidget(Module* module)
    {
      setModule(module);
    }
  };

}
