namespace groove_box
{
  // Why does TrackModel Exist?  Well..
  // It's all about copying tracks.
  //
  // Moving the Track's variables into the TrackModel gives us the ability
  // to take advantage of c++'s ability to quickly copy structures and not
  // have to copy all of the variables one-by-one.  Since there are 128 tracks
  // (8 per memory bank), that would be a lot of copying.  Instead, we can either
  // use the assignment operator (=) or memcopy.
  //
  // However, IMPORTANT, don't add any pointers to this structure, because
  // they'd be "shallow copied" and all of the pointers would end up pointing
  // to the same thing instead of making a copy.
  //
  struct TrackModel
  {
    bool steps[NUMBER_OF_STEPS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    unsigned int playback_position = 0;
    unsigned int range_end = NUMBER_OF_STEPS - 1; // was length
    unsigned int range_start = 0;
    unsigned int ratchet_counter = 0;

    // Global track values set by the expandcer
    float track_pan = 0.0;
    float track_pitch = 0.0;
    float track_volume = 0.0;

    float volume_slew_target = 0.0;
    float pan_slew_target = 0.0;
    float filter_cutoff_slew_target = 0.0;
    float filter_resonance_slew_target = 0.0;

    // The "skipped" variable keep track of when a trigger has been skipped because
    // the "Percentage" funtion is non-zero and didn't fire on the current step.
    // When a step is skipped, then ratcheting should not be applied to it for
    // that skipped step.

    bool skipped = false;

    ParameterLockSettings parameter_lock_settings[NUMBER_OF_STEPS]; // settings assigned to each step
    ParameterLockSettings local_parameter_lock_settings;            // currently used settings
  };
}    