// TODO
namespace scalar_110
{
  struct LowDrums : Engine
  {
    // All engines should have these variables
    std::string knob_labels[8] = {"Drum","","","","","","",""};
    unsigned int knob_lcd_focus[8] = {
      LCD_PAGE_PARAMETER_VALUES,
      LCD_PAGE_PARAMETER_VALUES,
      LCD_PAGE_PARAMETER_VALUES,
      LCD_PAGE_PARAMETER_VALUES,
      LCD_PAGE_PARAMETER_VALUES,
      LCD_PAGE_PARAMETER_VALUES,
      LCD_PAGE_PARAMETER_VALUES,
      LCD_PAGE_PARAMETER_VALUES
    };
    StepParams default_params;

    // Engine specific variables
    uint8_t w = 0;
    uint32_t t = 0;
    uint32_t v[3];
    unsigned int clock_division = 0;
    unsigned int clock_division_counter = 0;
    unsigned int drum_selection = 0;
    float env = 0.0f;

    // float p[NUMBER_OF_PARAMETERS];

    LowDrums() // constructor
    {
      this->default_params.p[0] = 0;
      this->default_params.p[1] = 0;
      this->default_params.p[2] = 0;
      this->default_params.p[3] = 0;
      this->default_params.p[4] = 0;
      this->default_params.p[5] = 0;
      this->default_params.p[6] = 0;
      this->default_params.p[7] = 0;
    }

    unsigned int getLCDController(unsigned int parameter_number) override
    {
      return(knob_lcd_focus[parameter_number]);
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
      uint32_t sustain = 0;
      float output = 0.0;
      float duration = 0;

      switch(drum_selection) {

        case 0: // long kick
          sustain = 0x7000;
          if(t < sustain) { // TODO: set sustain using parameter
            incrementT();
            output = bytebeatToAudio((((int)sqrt((t>>3)%sustain)<<7&241)/60-1));
          }
          else {
            output = 0; // This will result in a 0 output
          }
          break;

        case 1: // noise snare (finally working!!)
          duration = 11025.0;
          if(t < duration)
          {
            incrementT();
            output = sin((t>>2)*sin((t>>4)));
            output = output * (1.0 - (t/duration));
            output *= 5.0;  // Bring -1 to 1 output into audio range (-5 to +5)
          }
          break;

        case 2: // Hihat #1
            incrementT();
            output = bytebeatToAudio(((div(1000,t)&28)&4241)*90);
          break;

        case 3: // Hihat #2
            incrementT();
            output = bytebeatToAudio((((div(1000,(t>>4))&22)&4281)*90)*sin((t>>3)*sin((t>>2))));
          break;

        case 4: // Zap
            incrementT();
            output = bytebeatToAudio(div(400000,(t>>4))&64);
          break;


          // save this for an effect at the end
          //float audio = sin((t>>2)*sin((t>>4)));
          //w = audio * 256.0;
      }


      return { output, output };
    }

    float bytebeatToAudio(uint8_t w)
    {
      return(((w / 256.0) * 10.0) - 5.0);
    }


    std::string getKnobLabel(unsigned int knob_number) override
    {
      return(knob_labels[knob_number]);
    }

    void incrementT()
    {
      clock_division_counter ++;
      if(clock_division_counter >= clock_division)
      {
        t = t + 1;
        clock_division_counter = 0;
      }
    }

    void trigger(StepParams *step_parameters) override
    {
      /*
      // p0 through p3 are variable values used in the equations
      v[0] = step_parameters->p[0] * 128.0;
      v[1] = step_parameters->p[1] * 128.0;
      v[2] = step_parameters->p[2] * 128.0;

      // parameter idea: stereo widening

      // p[3] is the clock division
      clock_division = step_parameters->p[3] * 16;
      */
      drum_selection = step_parameters->p[0] * 4;
      v[0] = step_parameters->p[0] * 128.0;
      v[1] = step_parameters->p[1] * 128.0;

      // Reset counter on trigger
      t = 0;
      env = 0;
    }

    void reset() override
    {
      // no action needed
    }

    ~LowDrums() // destructor
    {
      // do nothing
    }

    void LCDDraw(const Widget::DrawArgs &args) override
    {
      // nothing yet
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
