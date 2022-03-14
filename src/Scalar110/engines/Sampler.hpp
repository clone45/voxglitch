namespace scalar_110
{
  struct Sampler : Engine
  {
    // All engines should have these variables
    std::string knob_labels[8] = {"Sample","","","","","","",""};
    unsigned int knob_lcd_focus[8] = {
      LCD_PAGE_SAMPLES,
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
    unsigned int sample_selection = 0;
    unsigned int track_number = 0;
    SampleBank& sample_bank = SampleBank::get_instance();

    Sampler(unsigned int track_number) // constructor
    {
      this->track_number = track_number;

      this->default_params.p[0] = 0;
      this->default_params.p[1] = 0;
      this->default_params.p[2] = 0;
      this->default_params.p[3] = 0;
      this->default_params.p[4] = 0;
      this->default_params.p[5] = 0;
      this->default_params.p[6] = 0;
      this->default_params.p[7] = 0;

      // Get at least the first sample assigned
      sample_bank.assign(sample_selection, track_number);
    }

    unsigned int getLCDController(unsigned int parameter_number) override
    {
      return(knob_lcd_focus[parameter_number]);
    }

    StepParams *getDefaultParams() override
    {
      return(&default_params);
    }

    std::pair<float, float> process() override
    {
      float left_audio = 0.0;
      float right_audio = 0.0;

      std::tie(left_audio, right_audio) = sample_bank.getOutput(track_number);

      left_audio = (left_audio * 5.0);
      right_audio = (right_audio * 5.0);

      return { left_audio, right_audio };
    }

    std::string getKnobLabel(unsigned int knob_number) override
    {
      return(knob_labels[knob_number]);
    }

    void trigger(StepParams *step_parameters) override
    {
      // read sample selection knob
      unsigned int sample_selection = (step_parameters->p[0] * sample_bank.size()); // sample selection
      sample_selection = clamp(sample_selection, 0, sample_bank.size() - 1);

      sample_bank.assign(sample_selection, track_number);

      // Trigger sample playback
      sample_bank.trigger(track_number);
    }

    void reset() override
    {
      // no action needed
    }

    ~Sampler() // destructor
    {
      // do nothing
    }

    void LCDDraw(const Widget::DrawArgs &args) override
    {
      // nothing yet
    }
  };

}
