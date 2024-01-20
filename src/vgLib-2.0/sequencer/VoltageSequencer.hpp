#include <iomanip>

#include "VoltageSequencerHistory.hpp"
#include "../helpers/JSON.hpp"

namespace vgLib_v2
{

    struct VoltageSequencer : public Sequencer
    {
        enum Polarity
        {
            UNIPOLAR, // = 0
            BIPOLAR   // = 1
        };

        double unipolar_default_value = 0.0;
        double bipolar_default_value = 0.0;

        std::vector<double> sequence;
        bool sample_and_hold = false;
        Polarity polarity = UNIPOLAR;
        unsigned int snap_division = 0;
        HistoryManager history_manager;

        double range_low = 0.0;
        double range_high = 10.0;

        // constructor
        VoltageSequencer(unsigned int sequence_length = 32, float default_value = 0.0)
        {
            sequence.assign(sequence_length, default_value);
        }

        // Sets the size of the vector and initializes it with "value"
        void assign(unsigned int length, double value)
        {
            sequence.assign(length, value);
        }

        // Method to set the voltage range
        void setRange(double low, double high)
        {
            range_low = low;
            range_high = high;
        }

        // Modified applyRange method
        double applyRange(double value) const
        {
            return range_low + value * (range_high - range_low);
        }

        // Returns the 'raw' output from the sequencer, which ranges from 0 to 1.
        double getValue(int index) const
        {
            return sequence[index];
        }

        // Same as GetValue(int index), but if no index is provided, returns the value at the current playback position.
        double getValue() const
        {
            return getValue(getPlaybackPosition());
        }

        // Returns the value with the range applied, in terms of voltage
        double getVoltage(int index) const
        {
            double internalValue = sequence[index];
            return applyRange(internalValue);
        }

        double getVoltage() const
        {
            return getVoltage(getPlaybackPosition());
        }     

        void setValue(int index, double value)
        {
            float old_value = sequence[index];

            // clamp voltage to be within range
            if (value < 0.0)
            {
                value = 0.0;
            }
            else if (value > 1.0)
            {
                value = 1.0;
            }

            if (snap_division > 0)
            {
                sequence[index] = round(value * snap_division) / snap_division;
            }
            else
            {
                sequence[index] = value;
            }

            // Record the action in the history manager
            history_manager.recordAction(index, old_value, value);
        }

        void fill(double value)
        {
            sequence.assign(sequence.size(), value);
        }

        void fill()
        {
            fill(getDefault());
        }

        void setSnapDivision(unsigned int new_snap_division)
        {
            this->snap_division = new_snap_division;
        }

        void setPolarity(Polarity new_polarity)
        {
            this->polarity = new_polarity;
        }

        Polarity getPolarity() const
        {
            return this->polarity;
        }

        void setDefaults(double unipolar_default_value, double bipolar_default_value)
        {
            this->unipolar_default_value = unipolar_default_value;
            this->bipolar_default_value = bipolar_default_value;
        }

        double getDefault() const
        {
            if (this->polarity == UNIPOLAR)
            {
                return this->unipolar_default_value;
            }
            else
            {
                return this->bipolar_default_value;
            }
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

        void randomizeInWindow()
        {
            history_manager.startSession();

            for (int i = window_start; i <= this->window_end; i++)
            {
                this->setValue(i, rand() / double(RAND_MAX));
            }

            history_manager.endSession();
        }

        void invertInWindow()
        {
            history_manager.startSession();

            for (int i = window_start; i <= this->window_end; i++)
            {
                this->setValue(i, 1.0 - this->getValue(i));
            }

            history_manager.endSession();
        }

        void sortInWindow()
        {
            history_manager.startSession();

            std::sort(sequence.begin() + window_start, sequence.begin() + window_end + 1);

            history_manager.endSession();
        }

        void mirrorInWindow()
        {
            history_manager.startSession();

            int i = window_start;
            int j = window_end;

            while (i < j)
            {
                sequence[j] = sequence[i];
                i++;
                j--;
            }

            history_manager.endSession();
        }


        void reverseInWindow()
        {
            history_manager.startSession();

            int i = window_start;
            int j = window_end;

            while (i < j)
            {
                double temp = sequence[i];
                sequence[i] = sequence[j];
                sequence[j] = temp;
                i++;
                j--;
            }

            history_manager.endSession();
        }
        
        void shuffleInWindow()
        {
            history_manager.startSession();

            for (int i = window_start; i <= this->window_end; i++)
            {
                int j = rand() % (this->window_end - this->window_start + 1) + this->window_start;
                double temp = sequence[i];
                sequence[i] = sequence[j];
                sequence[j] = temp;
            }

            history_manager.endSession();
        }

        void zeroInWindow()
        {
            history_manager.startSession();

            for (int i = window_start; i <= this->window_end; i++)
            {
                this->setValue(i, 0.0);
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

        void initialize()
        {
            // this->sample_and_hold = false;
            // this->polarity = UNIPOLAR;
            sequence.assign(sequence.size(), getDefault());

            // Call base class initialize
            Sequencer::initialize();
        }

        void undo()
        {
            if (history_manager.undo_stack.empty())
            {
                return;
            }

            // Get the last session from the undo stack
            Session session = history_manager.undo_stack.top();
            history_manager.undo_stack.pop();

            // Add the session to the redo stack
            history_manager.redo_stack.push(session);

            // Process the actions in reverse order
            for (auto action = session.getActions().rbegin(); action != session.getActions().rend(); ++action)
            {
                sequence[action->index] = action->oldValue;
            }
        }

        void redo()
        {
            if (history_manager.redo_stack.empty())
            {
                return;
            }

            // Get the last session from the redo stack
            Session session = history_manager.redo_stack.top();
            history_manager.redo_stack.pop();

            // Add the session to the undo stack
            history_manager.undo_stack.push(session);

            // Process the actions in order
            for (auto action = session.getActions().begin(); action != session.getActions().end(); ++action)
            {
                sequence[action->index] = action->newValue;
            }
        }

        int getWindowStart()
        {
            return this->window_start;
        }

        int getWindowEnd()
        {
            return this->window_end;
        }

        void copy(VoltageSequencer *src_sequence)
        {
            // We will be replacing this code with the standardized copy/paste code
            /*
            this->sequence = src_sequence->sequence;
            this->window_start = src_sequence->window_start;
            this->window_end = src_sequence->window_end;
            this->snap_division = src_sequence->snap_division;
            this->sample_and_hold = src_sequence->sample_and_hold;
            */
        }

        // This function is for backwards compatibility with vgLib-1.0
        void clear()
        {
            initialize();
        }

        // This function is for backwards compatibility with vgLib-1.0
        void shiftLeft()
        {
            this->shiftLeftInWindow();
        }

        // This function is for backwards compatibility with vgLib-1.0
        void shiftRight()
        {
            this->shiftRightInWindow();
        }

        // This function is for backwards compatibility with vgLib-1.0
        void randomize()
        {
            this->randomizeInWindow();
        }

        json_t *serialize() const override
        {
            json_t* voltage_sequencer_json = Sequencer::serialize();

            // Add specific attributes for VoltageSequencer
            json_object_set_new(voltage_sequencer_json, "polarity", json_integer(getPolarity()));
            json_object_set_new(voltage_sequencer_json, "snap_division", json_integer(snap_division));
            json_object_set_new(voltage_sequencer_json, "range_low", json_real(range_low));
            json_object_set_new(voltage_sequencer_json, "range_high", json_real(range_high));
            json_object_set_new(voltage_sequencer_json, "sample_and_hold", json_boolean(sample_and_hold));

            // Create a JSON array for the sequence values
            //
            // Just a quick reminder on naming conventions: Values for a voltage sequencer always
            // range between 0 and 1.  The "voltages" are the values mapped based on range and
            // polarity.  This is why the JSON array is called "values" and not "voltages".
            //

            json_t *values_array = json_array();
            for (unsigned int column = 0; column < (unsigned int) getMaxLength(); column++)
            {
                json_array_append_new(values_array, json_real(getValue(column)));
            }

            // Add sequence array and other properties to the JSON object
            json_object_set_new(voltage_sequencer_json, "values", values_array);

            return voltage_sequencer_json;
        }

        // Deserialize a VoltageSequencer from a JSON object
        void deserialize(json_t* json) override 
        {
            Sequencer::deserialize(json);

            // Get the values array
            json_t* values_array_json = json_object_get(json, "values");
            if (values_array_json && json_is_array(values_array_json))
            {
                json_t* value_json;
                unsigned int index;

                json_array_foreach(values_array_json, index, value_json)
                {
                    if (index < (unsigned int) getMaxLength())
                    {
                        setValue(index, json_real_value(value_json));
                    }
                }
            }

            this->snap_division = JSON::getInteger(json, "snap_division");
            this->range_low = JSON::getNumber(json, "range_low");
            this->range_high = JSON::getNumber(json, "range_high");
            this->sample_and_hold = JSON::getBoolean(json, "sample_and_hold");


            /*
            // Get the polarity
            json_t* polarity_json = json_object_get(json, "polarity");
            if (polarity_json && json_is_integer(polarity_json))
            {
                this->polarity = (Polarity) json_integer_value(polarity_json);
            }

            // Get the snap division
            json_t* snap_division_json = json_object_get(json, "snap_division");
            if (snap_division_json && json_is_integer(snap_division_json))
            {
                this->snap_division = json_integer_value(snap_division_json);
            }

            // Get the range low
            json_t* range_low_json = json_object_get(json, "range_low");
            if (range_low_json && json_is_real(range_low_json))
            {
                this->range_low = json_real_value(range_low_json);
            }

            // Get the range high
            json_t* range_high_json = json_object_get(json, "range_high");
            if (range_high_json && json_is_real(range_high_json))
            {
                this->range_high = json_real_value(range_high_json);
            }

            // Get the sample and hold
            json_t* sample_and_hold_json = json_object_get(json, "sample_and_hold");
            if (sample_and_hold_json && json_is_boolean(sample_and_hold_json))
            {
                this->sample_and_hold = json_boolean_value(sample_and_hold_json);
            }
            */

        }

    };

} // namespace vgLib_v2