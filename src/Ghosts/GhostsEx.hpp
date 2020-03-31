#pragma once
#include <stack>
#include "../Common/submodules.hpp"
#define REMOVAL_RAMP_ACCUMULATOR 0.01f

struct Ghost
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
    StereoSmoothSubModule loop_smooth;

    float removal_smoothing_ramp = 0;

    float output_voltage_left = 0;
    float output_voltage_right = 0;

    bool marked_for_removal = false;
    bool erase_me = false;

    std::pair<float, float> getStereoOutput(float smooth_rate)
    {
        if(erase_me == true) return {0, 0};

        // Note that we're adding two floating point numbers, then casting
        // them to an int, which is much faster than using floor()
        sample_position = this->start_position + this->playback_position;

        // Wrap if the sample position is past the sample end point
        sample_position = sample_position % this->sample_ptr->total_sample_count;

        output_voltage_left  = this->sample_ptr->leftPlayBuffer[sample_position];
        output_voltage_right = this->sample_ptr->rightPlayBuffer[sample_position];

        // Smooth out transitions (or passthrough unmodified when not triggered)
        std::tie(output_voltage_left, output_voltage_right) = loop_smooth.process(output_voltage_left, output_voltage_right, smooth_rate);

        if(marked_for_removal && (removal_smoothing_ramp < 1))
        {
            removal_smoothing_ramp += REMOVAL_RAMP_ACCUMULATOR;
            output_voltage_left = (output_voltage_left * (1.0f - removal_smoothing_ramp));
            output_voltage_right = (output_voltage_right * (1.0f - removal_smoothing_ramp));
            if(removal_smoothing_ramp >= 1) erase_me = true;
        }

        return {output_voltage_left, output_voltage_right};
    }

    void step(float step_amount)
    {
        if(erase_me == false)
        {
            // Step the playback position forward.
            playback_position = playback_position + step_amount;

            // If the playback position is past the playback length, then wrap the playback position to the beginning
            if(playback_position >= playback_length)
            {
                // fmod is modulus for floating point variables
                playback_position = fmod(playback_position, playback_length);

                // loop_smooth_left.trigger();
                // loop_smooth_right.trigger();
                loop_smooth.trigger();
            }
        }
    }

    void markForRemoval()
    {
        if(marked_for_removal == false) marked_for_removal = true;
    }
};

struct GhostsEx
{
    std::deque<Ghost> graveyard;

    GhostsEx()
    {
    }

    virtual ~GhostsEx() {
    }

    virtual void markAllForRemoval()
    {
        // Iterate over active grains, mark them for removal, and copy them
        // into the deprecated_grains deque
        for (Ghost &ghost : graveyard)
        {
            ghost.markForRemoval();
        }
    };

    // Return number of active grains
    virtual int size()
    {
        return(graveyard.size());
    }

    virtual bool isEmpty()
    {
        return(graveyard.empty());
    }

    virtual void add(float start_position, float playback_length, Sample *sample_ptr)
    {
        Ghost ghost;

        // Configure it for playback
        ghost.start_position = start_position;
        ghost.playback_length = playback_length;
        ghost.sample_ptr = sample_ptr;

        graveyard.push_back(ghost);
    }

    // Once there are too many active grains, we move a lot of the older active
    // grains into the deprecated grains bucket.  These deprecated grains will
    // quickly fade out, then be recycled by being placed into the available grain pool.

    virtual void markOldestForRemoval(unsigned int nth)
    {
        if(nth >= graveyard.size())
        {
            markAllForRemoval();
            return;
        }

        for(unsigned int i=0; i < nth; i++)
        {
            graveyard[i].markForRemoval();
        }
    }

    virtual std::pair<float, float> process(float smooth_rate, float step_amount)
    {
        float left_mix_output = 0;
        float right_mix_output = 0;

        //
        // Process grains
        // ---------------------------------------------------------------------

        for (Ghost &ghost : graveyard)
        {
            if(ghost.erase_me != true)
            {
                std::pair<float, float> stereo_output = ghost.getStereoOutput(smooth_rate);
                left_mix_output  += stereo_output.first;
                right_mix_output += stereo_output.second;
                ghost.step(step_amount);
            }
        }

        // perform cleanup of grains ready for removal
        graveyard.erase(std::remove_if(
            graveyard.begin(), graveyard.end(),
                [](const Ghost& ghost) {
                    return ghost.erase_me;
                }), graveyard.end());

        return {left_mix_output, right_mix_output};
    }

};
