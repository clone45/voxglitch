struct AutobreakMemory
{
    AutobreakVoltageSequencer position_sequencer;
    AutobreakVoltageSequencer sample_sequencer;
    AutobreakVoltageSequencer volume_sequencer;
    AutobreakVoltageSequencer pan_sequencer;
    AutobreakVoltageSequencer reverse_sequencer;
    AutobreakVoltageSequencer ratchet_sequencer;

    unsigned int snap_division_indexes[6] = {};
    unsigned int voltage_range_indexes[6] = {};

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

    void clear()
    {
        this->position_sequencer.clear();
        this->sample_sequencer.clear();
        this->volume_sequencer.fill(1.0);
        this->pan_sequencer.fill(0.5);
        this->reverse_sequencer.clear();
        this->ratchet_sequencer.clear();

        this->position_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->sample_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->volume_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->pan_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->reverse_sequencer.setLength(MAX_SEQUENCER_STEPS);
        this->ratchet_sequencer.setLength(MAX_SEQUENCER_STEPS);
    }
};