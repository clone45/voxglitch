namespace vgLib_v2
{

    struct GateSequencer : Sequencer
    {
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

        void deserialize(json_t* json) override 
        {
            Sequencer::deserialize(json);

            json_t* values_array_json = json_object_get(json, "values");
            if (values_array_json && json_is_array(values_array_json))
            {
                json_t* value_json;
                unsigned int index;

                json_array_foreach(values_array_json, index, value_json)
                {
                    if (index < (unsigned int) getMaxLength())
                    {
                        if (json_is_boolean(value_json))
                        {
                            bool value = json_boolean_value(value_json);
                            setValue(index, value);
                        }
                    }
                }
            }
        }

        json_t *serialize() override
        {
            json_t* gate_sequencer_json = Sequencer::serialize();

            json_t *values_array = json_array();
            for (unsigned int column = 0; column < (unsigned int) getMaxLength(); column++)
            {
                bool value = getValue(column);
                json_array_append_new(values_array, json_boolean(value));
            }

            json_object_set_new(gate_sequencer_json, "values", values_array);

            return gate_sequencer_json;
        }


    };

} // namespace vgLib_v1