struct AutobreakMemory
{
    VoltageSequencer position_sequencer;
    VoltageSequencer sample_sequencer;
    VoltageSequencer volume_sequencer;
    VoltageSequencer pan_sequencer;
    VoltageSequencer reverse_sequencer;
    VoltageSequencer ratchet_sequencer;

    AutobreakMemory()
    {
        // There are some hacks here to modify the snap divisions
        // that come standard with the voltage sequencer.  I should
        // rethink how the voltage sequencer is assigned the snap
        // division in the future, possibly moving the snap values
        // array into the module.

        // Position sequencer
        position_sequencer.assign(NUMBER_OF_STEPS, 0.0);
        position_sequencer.setSnapDivision(16);

        // Volume sequencer
        volume_sequencer.assign(NUMBER_OF_STEPS, 1.0);

        // Sample selection sequencer
        sample_sequencer.assign(NUMBER_OF_STEPS, 0.0);
        sample_sequencer.setSnapDivision(NUMBER_OF_SAMPLES - 1);

        // Pan sequencer
        pan_sequencer.assign(NUMBER_OF_STEPS, 0.5);

        // Reverse sequencer
        reverse_sequencer.assign(NUMBER_OF_STEPS, 0.0);

        // ratchet_sequencer.assign(NUMBER_OF_STEPS, 0.0);
        // ratchet_sequencer.snap_divisions[1] = 5;
        ratchet_sequencer.setSnapDivision(5);
    }

    void copy(AutobreakMemory *src_memory)
    {
        this->position_sequencer.copy(&src_memory->position_sequencer);
        this->sample_sequencer.copy(&src_memory->sample_sequencer);
        this->volume_sequencer.copy(&src_memory->volume_sequencer);
        this->pan_sequencer.copy(&src_memory->pan_sequencer);
        this->reverse_sequencer.copy(&src_memory->reverse_sequencer);
        this->ratchet_sequencer.copy(&src_memory->ratchet_sequencer);
    }

    void clear()
    {
        this->position_sequencer.initialize();
        this->sample_sequencer.initialize();
        this->volume_sequencer.fill(1.0);
        this->pan_sequencer.fill(0.5);
        this->reverse_sequencer.initialize();
        this->ratchet_sequencer.initialize();

        this->position_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->sample_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->volume_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->pan_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->reverse_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->ratchet_sequencer.setLength(MAX_SEQUENCER_STEPS);
    }
};