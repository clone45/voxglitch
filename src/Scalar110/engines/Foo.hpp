// TODO
namespace scalar_110
{
  struct Foo : Engine
  {
    // All engines should have these variables
    std::string knob_labels[8] = {"V1","V2","V3","Clk Div","Equation","","",""};
    StepParams default_params;

    // Engine specific variables
    uint8_t w = 0;
    uint32_t t = 0;
    uint32_t v[3];
    unsigned int clock_division = 0;
    unsigned int clock_division_counter = 0;
    unsigned int selected_equation = 0;
    bool playing = false;

    // float p[NUMBER_OF_PARAMETERS];

    Foo() // constructor
    {
      this->default_params.p[0] = 0; // v1
      this->default_params.p[1] = 0; // v2
      this->default_params.p[2] = 0; // v3
      this->default_params.p[3] = 0.25; // clk div of 4 == 4 / 16 = .25
      this->default_params.p[4] = 0; // equation
      this->default_params.p[5] = 0;
      this->default_params.p[6] = 0;
      this->default_params.p[7] = 0;
    }

    StepParams *getDefaultParams() override
    {
      return(&default_params);
    }

    //
    // There should always be 8 parameters
    //
    std::pair<float, float> process() override
    {
      float output = 0;

      if(playing)
      {
        clock_division_counter ++;
        if(clock_division_counter >= clock_division)
        {
          t = t + 1;
          clock_division_counter = 0;
        }

        // t = t + 1;

        switch(selected_equation) {

          case 0: // Exploratorium
            w = ((mod(t,(v[1]+(mod(t,v[2])))))^(t>>(v[3]>>5)))*2;
            break;

          case 1: // Toner
            w = ((t>>( mod((t>>12), (v[3]>>4)) ))+( mod((v[1]|t),v[2])))<<2;
            break;
        }

        output = ((w / 256.0) * 10.0) - 5.0;
      }

      return { output, output };
    }

    std::string getKnobLabel(unsigned int knob_number) override
    {
      return(knob_labels[knob_number]);
    }

    void reset() override
    {
      playing = false;
    }

    void trigger(StepParams *step_parameters) override
    {
      playing = true;

      // p0 through p3 are variable values used in the equations
      v[0] = step_parameters->p[0] * 128.0;
      v[1] = step_parameters->p[1] * 128.0;
      v[2] = step_parameters->p[2] * 128.0;

      // parameter idea: stereo widening

      // p[3] is the clock division
      clock_division = step_parameters->p[3] * 16;

      selected_equation = step_parameters->p[4] * 2;

      // Reset counter on trigger
      t = 0;
    }


    ~Foo() // destructor
    {
      // do nothing
    }

    void LCDDraw(const Widget::DrawArgs &args) override
    {
      const auto vg = args.vg;

      nvgBeginPath(vg);
      nvgRect(vg, 0, 0, LCD_DISPLAY_WIDTH, LCD_DISPLAY_HEIGHT - 20);
      nvgFillColor(vg, nvgRGBA(0, 20, 10, 100));
      nvgFill(vg);
    }

    //
    // Helper math functions that can't die by division by 0
    //
    uint32_t mod(uint32_t a, uint32_t b)
    {
      if(b == 0) return(0);
      return(a % b);
    }

    uint32_t div(uint32_t a, uint32_t b)
    {
      if(b == 0) return(0);
      return(a / b);
    }
  };
}
