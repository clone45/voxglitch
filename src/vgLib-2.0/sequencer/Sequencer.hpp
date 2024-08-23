#include <algorithm>
#include <vector>
#include <random>

namespace vgLib_v2
{
    enum PlaybackMode
    {
        FORWARD,
        BACKWARD,
        PINGPONG,
        RANDOM
    };

    struct Sequencer
    {
        int window_start = 0;
        int window_end = 15;
        int max_length = 16;
        int sequence_playback_position = 0;
        int pingpong_direction = 1; // 1 for forward, -1 for backward
        std::vector<int> random_steps;
        int last_played_step = -1; // for random mode
        std::mt19937 rng;

        PlaybackMode playback_mode = FORWARD;

        Sequencer()
        {
            rng.seed(std::random_device()());
        }

        void step()
        {
            if (playback_mode == FORWARD)
            {
                sequence_playback_position++;
                if (sequence_playback_position > window_end)
                {
                    sequence_playback_position = window_start;
                }
            }
            else if (playback_mode == BACKWARD)
            {
                sequence_playback_position--;
                if (sequence_playback_position < window_start)
                {
                    sequence_playback_position = window_end;
                }
            }
            else if (playback_mode == PINGPONG)
            {
                // Check if the next step will go out of bounds, if so, reverse direction first
                int next_position = sequence_playback_position + pingpong_direction;
                if (next_position > window_end || next_position < window_start)
                {
                    pingpong_direction *= -1;
                }

                sequence_playback_position += pingpong_direction;
            }
            else if (playback_mode == RANDOM)
            {
                if (random_steps.empty())
                {
                    // refill the random steps
                    for (int i = window_start; i <= window_end; ++i)
                    {
                        random_steps.push_back(i);
                    }
                    std::shuffle(random_steps.begin(), random_steps.end(), rng);

                    // Check if the last played step is the same as the first in the new shuffle
                    if (!random_steps.empty() && random_steps.front() == last_played_step)
                    {
                        // Swap the first and last element to avoid immediate repetition
                        std::swap(random_steps.front(), random_steps.back());
                    }
                }

                // Pop the next step and play it
                if (!random_steps.empty())
                {
                    sequence_playback_position = random_steps.back();
                    last_played_step = random_steps.back();
                    random_steps.pop_back();
                }
            }

            if (sequence_playback_position < window_start)
            {
                sequence_playback_position = window_start;
            }
            else if (sequence_playback_position > window_end)
            {
                sequence_playback_position = window_end;
            }
        }

        void setPosition(int position)
        {
            sequence_playback_position = clamp(position, this->window_start, this->window_end);
        }

        void setPlaybackMode(PlaybackMode mode)
        {
            playback_mode = mode;
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

        void reset()
        {
            sequence_playback_position = 0;
        }

        int getPlaybackPosition()
        {
            return (sequence_playback_position);
        }

        int getPosition()
        {
            return (sequence_playback_position);
        }

        // Window start and window end start at 0.  This means, for a sequence of 8 steps, 
        // the window start is 0 and the window end is 7.

        void setWindowStart(int window_start)
        {
            this->window_start = clamp(window_start, 0, this->window_end);
        }

        int getWindowStart()
        {
            return (window_start);
        }

        void setWindowEnd(int window_end)
        {
            this->window_end = clamp(window_end, this->window_start, this->max_length);
        }

        bool isWithinWindow(int position)
        {
            return ((position >= window_start) && (position <= window_end));
        }

        int getWindowEnd()
        {
            return (window_end);
        }

        void setMaxLength(int max_length)
        {
            this->max_length = max_length;
        }

        int getMaxLength()
        {
            return (max_length);
        }

        void initialize()
        {
            window_start = 0;
            window_end = this->max_length - 1;
            sequence_playback_position = 0;
            pingpong_direction = 1;
        }
    };

} // namespace vgLib_v2