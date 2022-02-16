namespace scalar_110
{
  struct Engine
  {
    std::string knob_labels[8] = {"placeholder","placeholder","placeholder","placeholder","placeholder","placeholder","placeholder","placeholder"};
    virtual std::pair<float, float> process(StepParams *parameters) = 0;
    virtual void trigger() = 0;
    // virtual ~Engine() = 0;
  };

  struct engineWidget : ModuleWidget
  {
    engineWidget(Module* module)
    {
      setModule(module);
    }
  };

}
