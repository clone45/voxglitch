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

        int getPlaybackPosition() const
        {
            return (sequence_playback_position);
        }

        int getPosition() const
        {
            return (sequence_playback_position);
        }

        void setWindowStart(int window_start)
        {
            this->window_start = clamp(window_start, 0, this->window_end);
        }

        int getWindowStart() const
        {
            return (window_start);
        }

        void setWindowEnd(int window_end)
        {
            this->window_end = clamp(window_end, this->window_start, this->max_length);
        }

        bool isWithinWindow(int position) const
        {
            return ((position >= window_start) && (position <= window_end));
        }

        int getWindowEnd() const
        {
            return (window_end);
        }

        void setMaxLength(int max_length)
        {
            this->max_length = max_length;
        }

        int getMaxLength() const
        {
            return (max_length);
        }

        // This function is for backwards compatibility with vgLib-1.0
        int getLength()
        {
            return (max_length);
        }

        // This function is for backwards compatibility with vgLib-1.0
        void setLength(unsigned int length)
        {
            this->setMaxLength(length);
            this->setWindowStart(0);
            this->setWindowEnd(length - 1);
        }

        void initialize()
        {
            window_start = 0;
            window_end = this->max_length - 1;
            sequence_playback_position = 0;
            pingpong_direction = 1;
        }

        // serialize
        virtual json_t *serialize() const
        {
            json_t *sequencer_data_json = json_object();

            // serialize window_start, window_end, sequence_playback_position, pingpong_direction, playback_mode
            json_object_set_new(sequencer_data_json, "window_start", json_integer(this->getWindowStart()));
            json_object_set_new(sequencer_data_json, "window_end", json_integer(this->getWindowEnd()));
            json_object_set_new(sequencer_data_json, "sequence_playback_position", json_integer(this->getPlaybackPosition()));
            json_object_set_new(sequencer_data_json, "pingpong_direction", json_integer(this->pingpong_direction));
            json_object_set_new(sequencer_data_json, "playback_mode", json_integer(this->playback_mode));

            return sequencer_data_json;
        }

        // deserialize
        virtual void deserialize(json_t *json_sequencer)
        {
            if (!json_sequencer || !json_is_object(json_sequencer))
                return;

            // deserialize window_start, window_end, sequence_playback_position, pingpong_direction, playback_mode

            // Load window_start
            json_t *window_start_json = json_object_get(json_sequencer, "window_start");
            if (window_start_json && json_is_integer(window_start_json))
            {
                this->setWindowStart(json_integer_value(window_start_json));
            }

            // Load window_end
            json_t *window_end_json = json_object_get(json_sequencer, "window_end");
            if (window_end_json && json_is_integer(window_end_json))
            {
                this->setWindowEnd(json_integer_value(window_end_json));
            }

            // Load sequence_playback_position
            json_t *sequence_playback_position_json = json_object_get(json_sequencer, "sequence_playback_position");
            if (sequence_playback_position_json && json_is_integer(sequence_playback_position_json))
            {
                this->setPosition(json_integer_value(sequence_playback_position_json));
            }

            // Load pingpong_direction
            json_t *pingpong_direction_json = json_object_get(json_sequencer, "pingpong_direction");
            if (pingpong_direction_json && json_is_integer(pingpong_direction_json))
            {
                this->pingpong_direction = json_integer_value(pingpong_direction_json);
            }

            // Load playback_mode
            json_t *playback_mode_json = json_object_get(json_sequencer, "playback_mode");
            if (playback_mode_json && json_is_integer(playback_mode_json))
            {
                this->setPlaybackMode(json_integer_value(playback_mode_json));
            }
        }
    };

} // namespace vgLib_v2