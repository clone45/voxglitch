// This represents a grouping of a voltage sequencer, gate sequencer
// and controls that might effect both sequencers, such as a window (start/end)
// selector, or sample & hold settings.
//
// The goal is to make it easier to avoid code duplication in my various modules
// that use this common layout so that I can easily update all modules at once
// when updating the sequencer behavior.  It may also allow me to eventually 
// create an expander module that will work will all of my sequencer based modules.
//
// Idea: Give the user the ability to turn on or off different features.

namespace vgLib_v2
{
    struct VGSequencerPlugin
    {
        VoltageSequencer voltage_sequencer;
        GateSequencer gate_sequencer;

        int window_start = 0; // Start index of the window for both sequencers
        int window_end = 15; // End index of the window for both sequencers

        // resize method
        void resize(int new_length)
        {
            voltage_sequencer.setLength(new_length);
            gate_sequencer.setLength(new_length);
        }

        void setMaxLength(int max_length)
        {
            voltage_sequencer.setMaxLength(max_length);
            gate_sequencer.setMaxLength(max_length);
        }

        void setWindowEnd(int window_end)
        {
            voltage_sequencer.setWindowEnd(window_end);
            gate_sequencer.setWindowEnd(window_end);
        }

        void assign(int index, float value)
        {
            voltage_sequencer.assign(index, value);
            gate_sequencer.assign(index, value);
        }

        void randomize()
        {
            voltage_sequencer.randomize();
            gate_sequencer.randomize();
        }
        
        void setLength(unsigned int length)
        {
            voltage_sequencer.setLength(length);
            gate_sequencer.setLength(length);
        }

        void reset()
        {
            voltage_sequencer.reset();
            gate_sequencer.reset();
        }

        void step()
        {
            voltage_sequencer.step();
            gate_sequencer.step();
        }

        float getVoltage()
        {
            return voltage_sequencer.getVoltage();
        }

        bool getGate()
        {
            return gate_sequencer.getValue();
        }

        toggleSampleAndHold()
        {
            voltage_sequencer.sample_and_hold ^= true;
        }
    };
}