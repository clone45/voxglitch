#include <algorithm>
#include <vector>
#include <random>

struct IndexSequencer
{
    enum PlaybackMode
    {
        FORWARD,
        BACKWARD,
        PINGPONG,
        RANDOM
    };

    int max_length = 16;
    int sequence_playback_position = 0;
    int pingpong_direction = 1; // 1 for forward, -1 for backward
    std::vector<int> random_steps;
    int last_played_step = -1; // for random mode
    std::mt19937 rng;
    std::vector<int> indicies_vec;

    PlaybackMode playback_mode = FORWARD;

    IndexSequencer()
    {
        rng.seed(std::random_device()());
    }

    /*
    
            // Copy the indicies_vec into a new vector that may be sorted later
        std::vector<int> sorted_indicies = indicies_vec;

        std::sort(sorted_indicies.begin(), sorted_indicies.end(),
            [this](int a, int b) -> bool
            {
                float voltageA = note_voltages[a];
                float voltageB = note_voltages[b];

                return voltageA < voltageB;
            });  
    
    
    */

    // void step(const std::vector<float>& note_voltages)
    
    void step()
    {
        if (indicies_vec.empty())
        {
            // Handle special case when all steps are excluded
            // In this case, just don't step.
            return;
        }

        if (playback_mode == FORWARD)
        {
            sequence_playback_position++;
            if (sequence_playback_position >= (int) indicies_vec.size())
            {
                sequence_playback_position = 0;
            }
        }
        else if (playback_mode == BACKWARD)
        {
            sequence_playback_position--;
            if (sequence_playback_position < 0)
            {
                sequence_playback_position = (int) indicies_vec.size() - 1;
            }
        }
        else if (playback_mode == PINGPONG)
        {
            // Check if the next step will go out of bounds, if so, reverse direction first
            int next_position = sequence_playback_position + pingpong_direction;
            if (next_position >= (int) indicies_vec.size() || next_position < 0)
            {
                pingpong_direction *= -1;
            }

            sequence_playback_position += pingpong_direction;
        }
        else if (playback_mode == RANDOM)
        {
            if (indicies_vec.empty()) return; // Return if there are no indices to play
            
            if (random_steps.empty())
            {
                // refill the random steps with the current indices
                random_steps = indicies_vec;
                std::shuffle(random_steps.begin(), random_steps.end(), rng);
                
                // Check if the last played step is the same as the first in the new shuffle
                if (!random_steps.empty() && random_steps.back() == last_played_step)
                {
                    // Swap the first and last element to avoid immediate repetition
                    if(random_steps.size() > 1)
                        std::swap(random_steps.front(), random_steps.back());
                }
            }

            // Pop the next step and play it
            sequence_playback_position = std::find(indicies_vec.begin(), indicies_vec.end(), random_steps.back()) - indicies_vec.begin();
            last_played_step = random_steps.back();

            random_steps.pop_back();
        }
    }

    int getIndex()
    {
        if (indicies_vec.empty()) return 0;
        if(sequence_playback_position < 0) return(indicies_vec[0]);
        
        return (indicies_vec[sequence_playback_position]);
    }

    void setPosition(int position)
    {
        sequence_playback_position = clamp(position, 0, indicies_vec.size() - 1);
    }

    void setPlaybackMode(PlaybackMode mode)
    {
        playback_mode = (PlaybackMode) ((unsigned int) mode % (NUMBER_OF_PLAYBACK_MODES / 2));

        // Clearing random steps to restart the non-repeating sequence
        if (mode == RANDOM)
        {
            random_steps.clear();
        }
    }

    void setPlaybackMode(int mode)
    {
        setPlaybackMode((PlaybackMode)mode);
    }

    void updateIndicies(std::vector<int>& indicies) 
    {
        indicies_vec = indicies;

        // Check to ensure that sequence_playback_position is still within bounds
        while (sequence_playback_position >= (int) indicies_vec.size())
        {
            sequence_playback_position = (int) indicies_vec.size() - 1;
        }
    }

    void reset()
    {
        sequence_playback_position = -1;
    }

    int getPlaybackPosition()
    {
        return (sequence_playback_position);
    }

    int getPosition()
    {
        return (sequence_playback_position);
    }

};
