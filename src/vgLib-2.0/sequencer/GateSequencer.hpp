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

        bool getValue(int index) const
        {
            return (sequence[index]);
        }

        bool getValue() const
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

        double getDefault() const
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

        /**
         * Deserializes a JSON object into this GateSequencer object.
         *
         * This method overrides the deserialize method in the Sequencer base class
         * and extends it to handle the specific attributes of the GateSequencer.
         *
         * @param json The JSON object to be deserialized.
         *
         * Example JSON input:
         * 
         * {
         *     "sequence_playback_position": 2,
         *     "window_start": 1,
         *     "window_end": 3,
         *     "max_length": 4,
         *     "pingpong_direction": -1,
         *     "playback_mode": 1, // Assuming 1 represents BACKWARD
         *     "values": [true, false, true, false]
         * }
         */
        void deserialize(json_t* json) override 
        {
            // First, call the deserialize method of the base class to handle common attributes.
            Sequencer::deserialize(json);

            // Retrieve the 'values' array from the JSON object. 
            json_t* values_array_json = json_object_get(json, "values");

            // Check if the retrieved 'values' is a valid JSON array.
            if (values_array_json && json_is_array(values_array_json))
            {
                json_t* value_json; // Temporary variable to hold each value in the array.
                unsigned int index; // Index for iterating through the array.

                // Iterate over each element in the JSON array.
                json_array_foreach(values_array_json, index, value_json)
                {
                    // Ensure the index is within the bounds of the sequence length.
                    if (index < (unsigned int) getMaxLength())
                    {
                        // Check if the current item is a boolean and deserialize it.
                        if (json_is_boolean(value_json))
                        {
                            // Convert JSON boolean to C++ boolean and set the sequence value.
                            bool value = json_boolean_value(value_json);
                            setValue(index, value);
                        }
                    }
                }
            }
        }


        /**
         * Serializes a GateSequencer object into a JSON object.
         *
         * This method overrides the serialize method in the Sequencer base class
         * and extends it to include the specific attributes of the GateSequencer.
         *
         * @return A JSON object representing the serialized GateSequencer.
         *
         * Example JSON output:
         * {
         *     "sequence_playback_position": 0,
         *     "window_start": 0,
         *     "window_end": 3,
         *     "max_length": 4,
         *     "pingpong_direction": 1,
         *     "playback_mode": 0, // Assuming 0 represents FORWARD
         *     "values": [true, false, true, false]
         * }
         */
        json_t *serialize() const override
        {
            // Call the serialize method of the base class to handle common attributes.
            json_t* gate_sequencer_json = Sequencer::serialize();

            // Create a new JSON array to store the sequence values.
            json_t *values_array = json_array();

            // Iterate over the sequence and add each value to the JSON array.
            for (unsigned int column = 0; column < (unsigned int) getMaxLength(); column++)
            {
                // Retrieve the value from the sequence.
                bool value = getValue(column);

                // Append the boolean value to the JSON array.
                json_array_append_new(values_array, json_boolean(value));
            }

            // Add the sequence array to the JSON object.
            json_object_set_new(gate_sequencer_json, "values", values_array);

            return gate_sequencer_json;
        }

    };

} // namespace vgLib_v1