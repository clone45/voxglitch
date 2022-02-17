// TODO
namespace scalar_110
{
  struct Foo : Engine
  {
    std::string knob_labels[8] = {"Ratcht","Pan","Prob %","Speed","Offset","Level","",""};
    uint8_t w = 0;
    uint32_t t = 0;
    uint32_t v[3];
    unsigned int clock_division = 0;
    unsigned int clock_division_counter = 0;

    float p[NUMBER_OF_PARAMETERS];
    // uint32_t p1, p2, p3;

    Foo() // constructor
    {
      for(unsigned int i=0; i<NUMBER_OF_PARAMETERS; i++)
      {
        p[i] = 0.0;
      }
    }

    //
    // There should always be 8 parameters
    //
    std::pair<float, float> process() override
    {
      clock_division_counter ++;
      if(clock_division_counter >= clock_division)
      {
        t = t + 1;
        clock_division_counter = 0;
      }

      // t = t + 1;

      // todo: inroduce 't' and create the 8 knobs on the front panels
      w = ((mod(t,(v[1]+(mod(t,v[2])))))^(t>>(v[3]>>5)))*2;
      float output = ((w / 256.0) * 10.0) - 5.0;
      return { output, output };
    }

    void trigger(StepParams *step_parameters) override
    {
      // p0 through p3 are variable values used in the equations
      v[0] = step_parameters->p[0] * 128.0;
      v[1] = step_parameters->p[1] * 128.0;
      v[2] = step_parameters->p[2] * 128.0;

      // p[3] is the clock division
      clock_division = step_parameters->p[3] * 16;

      // Reset counter on trigger
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
