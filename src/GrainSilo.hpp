#pragma once

#include "smooth.hpp"

struct Grain
{
    // Start Position is the offset into the sample where playback should start.
	// It is set when the ghost is first created.
	float start_position;

	// Playback length for the ghost, measuring in .. er.. ticks?
	float playback_length;

	// sample_ptr points to the loaded sample in memory
	Sample *sample_ptr;

	// playback_position is similar to samplePos used in for samples.  However,
	// it's relative to the Ghost's start_position rather than the sample
	// start position.
	float playback_position = 0.0f;

	unsigned int sample_position = 0;

	// Smoothing classes to remove clicks and pops that would happen when sample
	// playback position jumps around.
	Smooth loop_smooth_left;
	Smooth loop_smooth_right;

	float removal_smoothing_ramp = 0;

	float output_voltage_left = 0;
	float output_voltage_right = 0;

	bool removed = false;
	bool marked_for_removal = false;

	std::pair<float, float> getStereoOutput(float smooth_rate)
	{
		// Note that we're adding two floating point numbers, then casting
		// them to an int, which is much faster than using floor()
		sample_position = this->start_position + this->playback_position;

		// Wrap if the sample position is past the sample end point
		sample_position = sample_position % this->sample_ptr->total_sample_count;

		output_voltage_left  = this->sample_ptr->leftPlayBuffer[sample_position];
		output_voltage_right = this->sample_ptr->rightPlayBuffer[sample_position];

		// Smooth out transitions (or passthrough unmodified when not triggered)
		output_voltage_left  = loop_smooth_left.process(output_voltage_left, smooth_rate);
		output_voltage_right = loop_smooth_right.process(output_voltage_right, smooth_rate);

		if(marked_for_removal && (removal_smoothing_ramp < 1))
		{
			removal_smoothing_ramp += 0.001f;
			output_voltage_left = (output_voltage_left * (1.0f - removal_smoothing_ramp));
			output_voltage_right = (output_voltage_right * (1.0f - removal_smoothing_ramp));
			if(removal_smoothing_ramp >= 1) removed = true;
		}

		return {output_voltage_left, output_voltage_right};
    }

    void step(float step_amount)
	{
		// Step the playback position forward.
		playback_position = playback_position + step_amount;

		// If the playback position is past the playback length, then wrap the playback position to the beginning
		if(playback_position >= playback_length)
		{
			// fmod is modulus for floating point variables
			playback_position = fmod(playback_position, playback_length);

			loop_smooth_left.trigger();
			loop_smooth_right.trigger();
		}
	}

	void markForRemoval()
	{
		marked_for_removal = true;
	}
};

struct GrainSilo
{
    std::vector<Grain> grains;

	GrainSilo()
	{

	}

	virtual ~GrainSilo() {}

	virtual void markAllForRemoval()
	{
        for(Grain& grain : grains)
        {
            if(! grain.marked_for_removal) grain.markForRemoval();
        }
	};

    virtual void cleanup()
    {
        grains.erase(std::remove_if(grains.begin(),grains.end(),[](const Grain &grain) { return grain.removed; }), grains.end());
    }

    virtual int getSize()
    {
        return(grains.size());
    }

    virtual bool isEmpty()
    {
        return(grains.empty());
    }

    virtual void add(float start_position, float playback_length, Sample *sample_ptr)
    {
        Grain grain;
        grain.start_position = start_position;
        grain.playback_length = playback_length;
        grain.sample_ptr = sample_ptr;
        grains.push_back(grain);
    }

    virtual void markOldestForRemoval(int nth)
    {
        for(int i=0; i < nth; i++)
        {
            if(! grains[i].marked_for_removal) grains[i].markForRemoval();
        }
    }

    virtual std::pair<float, float> process(float smooth_rate, float step_amount)
    {
        float left_mix_output = 0;
        float right_mix_output = 0;

        for(Grain& grain : grains)
        {
            std::pair<float, float> stereo_output = grain.getStereoOutput(smooth_rate);
            left_mix_output  += stereo_output.first;
            right_mix_output += stereo_output.second;
            grain.step(step_amount);
        }

        return {left_mix_output, right_mix_output};
    }

};
