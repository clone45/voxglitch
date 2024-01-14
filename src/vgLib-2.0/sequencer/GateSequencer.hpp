namespace vgLib_v2
{

    struct GateSequencer : Sequencer
    {
        // std::array<bool, MAX_SEQUENCER_STEPS> sequence;

        std::vector<bool> sequence;
        HistoryManager history_manager;
        bool default_value = false;

        // constructor
        GateSequencer()
        {
        }

        // This must be called before interacting with the voltage sequencer since
        // it sets the size of the vector and initializes it with "value"
        void assign(unsigned int length, double value)
        {
            sequence.assign(length, value);
        }

        bool getValue(int index)
        {
            return (sequence[index]);
        }

        bool getValue()
        {
            return (sequence[sequence_playback_position]);
        }

        void setValue(int index, bool value)
        {
            sequence[index] = value;
        }

        void toggleValue(int index)
        {
            sequence[index] = !sequence[index];
        }

        double getDefault()
        {
            return (default_value);
        }

        void shiftLeft()
        {
            this->shiftLeftInWindow();
        }

        void shiftRight()
        {
            this->shiftRightInWindow();
        }

        void shiftLeftInWindow()
        {
            double temp = sequence[this->window_start];
            for (int i = this->window_start; i < this->window_end; i++)
            {
                this->setValue(i, this->getValue(i + 1));
            }
            sequence[this->window_end] = temp;
        }

        void shiftRightInWindow()
        {
            double temp = sequence[this->window_end];

            for (int i = this->window_end; i > window_start; i--)
            {
                this->setValue(i, this->getValue(i - 1));
            }

            sequence[window_start] = temp;
        }

        void randomize()
        {
            this->randomizeInWindow();
        }

        void clear()
        {
            this->resetInWindow();
        }

        void randomizeInWindow()
        {
            history_manager.startSession();

            for (int i = window_start; i <= this->window_end; i++)
            {
                this->setValue(i, rand() / double(RAND_MAX));
            }

            history_manager.endSession();
        }

        void resetInWindow()
        {
            history_manager.startSession();

            for (int i = window_start; i <= this->window_end; i++)
            {
                this->setValue(i, getDefault());
            }

            history_manager.endSession();
        }


    };

} // namespace vgLib_v1