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
        position_sequencer.setSnapDivisionIndex(4);

        // Volume sequencer
        volume_sequencer.assign(NUMBER_OF_STEPS, 1.0);

        // Sample selection sequencer
        sample_sequencer.assign(NUMBER_OF_STEPS, 0.0);
        sample_sequencer.snap_divisions[1] = NUMBER_OF_SAMPLES - 1;
        sample_sequencer.setSnapDivisionIndex(1);

        // Pan sequencer
        pan_sequencer.assign(NUMBER_OF_STEPS, 0.5);

        // Reverse sequencer
        reverse_sequencer.assign(NUMBER_OF_STEPS, 0.0);

        ratchet_sequencer.assign(NUMBER_OF_STEPS, 0.0);
        ratchet_sequencer.snap_divisions[1] = 5;
        ratchet_sequencer.setSnapDivisionIndex(1);
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
};