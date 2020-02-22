#pragma once
#include <stack>
#include "GrainAmpSlopes.hpp"
#include "submodules.hpp"

#define MAX_GRAINS 600

struct SimpleGrain
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
    float pan = 0;
    unsigned int sample_position = 0;

    float output_voltage_left = 0;
    float output_voltage_right = 0;

    bool erase_me = false;

    // StereoFadeOutSubModule stereo_fade_out;
    // StereoFadeInSubModule stereo_fade_in;
    StereoPanSubModule panner;

    SimpleGrain()
    {
        // Since the fade out isn't processed until the sample playback is past
        // a certain point, we can just trigger it now and it'll start working
        // later.
        // stereo_fade_out.trigger();
        // stereo_fade_in.trigger();
    }

    std::pair<float, float> getStereoOutput(float smooth_rate, int selected_slope)
    {
        if(playback_length == 0) return {0,0};

        // Note that we're adding two floating point numbers, then casting
        // them to an int, which is much faster than using floor()
        sample_position = this->start_position + this->playback_position;

        if(sample_position >= this->sample_ptr->total_sample_count)
        {
            // NOTE: Ideally, the sample position should be reaching the total sample
            // count (or length) exactly as an applied amp envelope is reaching 0
            erase_me = true;
        }
        else
        {
            output_voltage_left  = this->sample_ptr->leftPlayBuffer[sample_position];
            output_voltage_right = this->sample_ptr->rightPlayBuffer[sample_position];

            // Apply amplitude slope
            int slope_index = (playback_position / playback_length) * 512.0;
            slope_index = clamp(slope_index, 0, 511);
            selected_slope = clamp(selected_slope, 0, 9);

            float slope_value = GRAIN_AMP_SLOPES[selected_slope][slope_index];

            output_voltage_left  = (slope_value / 256.0) * output_voltage_left;
            output_voltage_right = (slope_value / 256.0) * output_voltage_right;

            // Apply pan
            std::tie(output_voltage_left, output_voltage_right) = panner.process(output_voltage_left, output_voltage_right, pan);
        }

        return {output_voltage_left, output_voltage_right};
    }

    void step(float step_amount)
    {
        if(erase_me == false)
        {
            // Step the playback position forward.
            playback_position = playback_position + step_amount;
            if(playback_position >= playback_length) erase_me = true;
        }
    }

};

struct SimpleGrainEngine
{

    std::deque<SimpleGrain> grain_queue;

    SimpleGrainEngine()
    {
    }

    virtual ~SimpleGrainEngine() {
    }

    // Return number of active grains
    virtual int size()
    {
        return(grain_queue.size());
    }

    virtual bool isEmpty()
    {
        return(grain_queue.empty());
    }

    virtual void add(float start_position, float playback_length, float pan, Sample *sample_ptr)
    {
        if(grain_queue.size() > MAX_GRAINS) return;
        if(playback_length == 0) return;

        SimpleGrain grain;

        // Configure it for playback
        grain.start_position = start_position;
        grain.playback_length = playback_length;
        grain.sample_ptr = sample_ptr;
        grain.pan = pan;

        grain_queue.push_back(grain);
    }

    virtual std::pair<float, float> process(float smooth_rate, float step_amount, int selected_slope)
    {
        float left_mix_output = 0;
        float right_mix_output = 0;

        //
        // Process grains
        // ---------------------------------------------------------------------

        for (SimpleGrain &grain : grain_queue)
        {
            if(grain.erase_me != true)
            {
                std::pair<float, float> stereo_output = grain.getStereoOutput(smooth_rate, selected_slope);
                left_mix_output  += stereo_output.first;
                right_mix_output += stereo_output.second;
                grain.step(step_amount);
            }
        }

        // perform cleanup of grains ready for removal
        grain_queue.erase(std::remove_if(
            grain_queue.begin(), grain_queue.end(),
                [](const SimpleGrain& grain) {
                    return grain.erase_me;
                }), grain_queue.end());

        return {left_mix_output, right_mix_output};
    }

};
