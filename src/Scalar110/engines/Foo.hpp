// TODO
namespace scalar_110
{
  struct Foo : Engine
  {
    std::string knob_labels[8] = {"Ratcht","Pan","Prob %","Speed","Offset","Level","",""};
    uint8_t w = 0;
    uint32_t t = 0;

    uint32_t p1 = 0;
    uint32_t p2 = 0;
    uint32_t p3 = 0;

    //
    // There should always be 8 parameters
    //
    std::pair<float, float> process(StepParams *step_parameters) override
    {
      t = t + 1;

      // todo: inroduce 't' and create the 8 knobs on the front panels
      w = ((mod(t,(p1+(mod(t,p2)))))^(t>>(p3>>5)))*2;
      float output = ((w / 256.0) * 10.0) - 5.0;
      return { output, output };
    }

    void trigger(StepParams *step_parameters) override
    {
      p1 = step_parameters->p[0] * 128.0;
      p2 = step_parameters->p[1] * 128.0;
      p3 = step_parameters->p[2] * 128.0;
      t = 0;
    }


    ~Foo() // destructor
    {
      // do nothing
    }

    uint32_t mod(uint32_t a, uint32_t b)
    {
      if(b == 0) return(0);
      return(a % b);
    }
  };
}
