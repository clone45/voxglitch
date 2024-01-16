#include <iomanip>

#include "VoltageSequencerHistory.hpp"

namespace vgLib_v2
{

    struct VoltageSequencer : Sequencer
    {
        enum Polarity
        {
            UNIPOLAR, // = 0
            BIPOLAR   // = 1
        };

        /* TODO: possibly implement defaults this way, because default values for things such as
                sample and hold and snap division are not currently stored, and thus cannot be
                _restored_ when the sequencer is reset.

        struct defaults 
        {
            Polarity polarity = UNIPOLAR;
            double unipolar_value = 0.0;
            double bipolar_value = 0.0;
            unsigned int snap_division = 0;
            bool sample_and_hold = false;
        };
        */

        double unipolar_default_value = 0.0;
        double bipolar_default_value = 0.0;

        std::vector<double> sequence;
        bool sample_and_hold = false;
        Polarity polarity = UNIPOLAR;
        unsigned int snap_division = 0;
        HistoryManager history_manager;


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

        // Returns the 'raw' output from the sequencer, which ranges from 0 to 1.
        double getValue(int index)
        {
            return sequence[index];
        }

        // Same as GetValue(int index), but if no index is provided, returns the value at the current playback position.
        double getValue()
        {
            return getValue(getPlaybackPosition());
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

        Polarity getPolarity()
        {
            return this->polarity;
        }

        void setDefaults(double unipolar_default_value, double bipolar_default_value)
        {
            this->unipolar_default_value = unipolar_default_value;
            this->bipolar_default_value = bipolar_default_value;
        }

        double getDefault()
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

    };

} // namespace vgLib_v2