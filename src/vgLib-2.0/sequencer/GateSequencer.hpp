namespace vgLib_v2
{

    // Gate sequencer has been restored from the old codebase and still have some older code,
    // such as the sequence_lenth variable, which is not used in the new codebase.  I had to create a 
    // local sequence_length variable to replace it.  

    struct GateSequencer : Sequencer
    {
        std::vector<bool> sequence;
        unsigned int sequence_length = 16;

        // constructor
        GateSequencer()
        {
            sequence.assign(sequence_length, 0);
        }

        void setMaxLength(unsigned int length)
        {
            sequence_length = length;
            
            // Call parent setMaxLength
            Sequencer::setMaxLength(length);
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

        void addGate() // for realtime gate entry
        {
            sequence[sequence_playback_position] = ! sequence[sequence_playback_position];
        }

        void shiftLeftInWindow()
        {
            shiftLeft();
        }

        void shiftRightInWindow()
        {
            shiftRight();
        }

        void shiftLeft()
        {
            double temp = sequence[0];
            for (unsigned int i = 0; i < this->sequence_length - 1; i++)
            {
                sequence[i] = sequence[i + 1];
            }
            sequence[this->sequence_length - 1] = temp;
        }

        void shiftRight()
        {
            double temp = sequence[this->sequence_length - 1];

            for (unsigned int i = this->sequence_length - 1; i > 0; i--)
            {
                sequence[i] = sequence[i - 1];
            }

            sequence[0] = temp;
        }

        void randomize()
        {
            for (unsigned int i = 0; i < this->sequence_length; i++)
            {
                this->setValue(i, fmod(std::rand(), 2));
            }
        }

        void clear()
        {
            // sequence.fill(0.0);
            sequence.assign(sequence.size(), 0);
        }
    };

} // namespace vgLib_v2