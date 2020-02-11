#pragma once
#include <stack>
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

	bool recycle = false;
	bool marked_for_removal = false;

	std::pair<float, float> getStereoOutput(float smooth_rate)
	{
		if(recycle == true) return {0.0f, 0.0f};

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
			if(removal_smoothing_ramp >= 1)
			{
				recycle = true;
				removal_smoothing_ramp = 1;
			}
		}

		return {output_voltage_left, output_voltage_right};
    }

    void step(float step_amount)
	{
		if(recycle == false)
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
	}

	void markForRemoval()
	{
		if(marked_for_removal == false && recycle == false) marked_for_removal = true;
	}

	void reset()
	{
		marked_for_removal = false;
		recycle = false;
		removal_smoothing_ramp = 0;
		playback_position = 0;
		sample_position = 0;
		loop_smooth_left.reset();
		loop_smooth_right.reset();
	}
};

struct GrainSilo
{
	std::deque<Grain *> active_grains;
	std::deque<Grain *> deprecated_grains;
	std::deque<Grain *> available_grain_pool;

	GrainSilo()
	{
		// Fill up the available grain pool with empty grains
		for(int i=0; i<128; i++)
		{
			Grain *grain = new Grain;
			grain->start_position = 0;
			grain->playback_length = 0;
			grain->sample_ptr = NULL;
			available_grain_pool.push_front(grain);
		}
	}

	virtual ~GrainSilo() {

		// If the module is removed from the patch, clean up all the pointers
		// to avoid memory leaks.

		for (Grain * grain_ptr : active_grains) { delete(grain_ptr); }
		active_grains.clear();

		for (Grain * grain_ptr : deprecated_grains) { delete(grain_ptr); }
		deprecated_grains.clear();

		for (Grain * grain_ptr : available_grain_pool) { delete(grain_ptr); }
		available_grain_pool.clear();
	}

	virtual void markAllForRemoval()
	{
		// Iterate over active grains, mark them for removal, and copy them
		// into the deprecated_grains deque
		for (Grain * grain_ptr : active_grains)
		{
			grain_ptr->markForRemoval();
			deprecated_grains.push_back(grain_ptr);
		}

		// ... then remove all the active grains.
		active_grains.clear();
	};

	virtual int active()
    {
        return(active_grains.size());
    }

    virtual bool isEmpty()
    {
        return((active_grains.size() + deprecated_grains.size()) == 0);
    }

    virtual void add(float start_position, float playback_length, Sample *sample_ptr)
    {
		if(available_grain_pool.size() > 0)
		{
			Grain *new_grain = available_grain_pool.back();
			available_grain_pool.pop_back();
			new_grain->start_position = start_position;
			new_grain->playback_length = playback_length;
			new_grain->sample_ptr = sample_ptr;
			active_grains.push_back(new_grain);
		}
    }

    virtual void markOldestForRemoval(int nth)
    {
        for(int i=0; i < nth; i++)
        {
			if(active_grains.size() > 0)
			{
				Grain *grain_ptr = active_grains.front();
				active_grains.pop_front();
				grain_ptr->markForRemoval();
				deprecated_grains.push_back(grain_ptr);
			}
        }
    }

    virtual std::pair<float, float> process(float smooth_rate, float step_amount)
    {
        float left_mix_output = 0;
        float right_mix_output = 0;

		//
		// Process active_grains
		// ---------------------------------------------------------------------

		for (Grain * grain : active_grains)
		{
			std::pair<float, float> stereo_output = grain->getStereoOutput(smooth_rate);

			if(grain->recycle != true)
			{
	            left_mix_output  += stereo_output.first;
	            right_mix_output += stereo_output.second;
				grain->step(step_amount);
			}
		}


		//
		// Process deprecated_grains
		// ---------------------------------------------------------------------

		for (Grain * grain : deprecated_grains)
		{
			std::pair<float, float> stereo_output = grain->getStereoOutput(smooth_rate);

			if(grain->recycle != true)
			{
	            left_mix_output  += stereo_output.first;
	            right_mix_output += stereo_output.second;
				grain->step(step_amount);
			}
		}

		// The block of code above is responsible for seeting recycle = true (in getStereoOutput)
		// which is why this block of code is safe to be here.

		while((! deprecated_grains.empty()) && (deprecated_grains.back()->recycle == true))
		{
			Grain *grain = deprecated_grains.back();
			deprecated_grains.pop_back();
			grain->reset();
			available_grain_pool.push_front(grain); // move grain back to available grain pool
		}

        return {left_mix_output, right_mix_output};
    }

};
