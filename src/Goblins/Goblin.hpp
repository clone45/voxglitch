struct Goblin
{
	// Start Position is the offset into the sample where playback should start.
	// It is set when the goblin is first created.
	float start_position;

	// sample_ptr points to the loaded sample in memory
	Sample *sample_ptr;

	// playback_position is similar to samplePos used in for samples.  However,
	// it's relative to the Goblin's start_position rather than the sample
	// start position.
	float playback_position = 0.0f;

	unsigned int sample_position = 0;

	std::pair<float, float> getStereoOutput()
	{
		// Note that we're adding two floating point numbers, then casting
		// them to an int, which is much faster than using floor()
		sample_position = this->start_position + this->playback_position;

		// Wrap if the sample position is past the sample end point
		if (sample_position >= this->sample_ptr->total_sample_count) sample_position = sample_position % this->sample_ptr->total_sample_count;

        // Return the audio
		return { this->sample_ptr->leftPlayBuffer[sample_position], this->sample_ptr->rightPlayBuffer[sample_position] };
	}

	void age(float step_amount, float playback_length)
	{
        // Step the playback position forward.
		playback_position = playback_position + step_amount;

        // If the playback position is past the playback length, then wrap the playback position to the beginning
		if(playback_position >= playback_length) playback_position = playback_position - playback_length;

		// If the playback position is less than 0, it's possible that we're playing the sample backwards,
		// so wrap around the end of the sample.
		if (playback_position < 0) playback_position = playback_length - playback_position;
	}
};
