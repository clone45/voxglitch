namespace scalar_110
{
  struct Sampler : Engine
  {

    // Engine specific variables
    unsigned int sample_selection = 0;
    unsigned int track_number = 0;
    SampleBank& sample_bank = SampleBank::get_instance();

    Sampler(unsigned int track_number) // constructor
    {
      this->track_number = track_number;

      // Get at least the first sample assigned
      sample_bank.assign(sample_selection, track_number);
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
  };

}