struct Page
{
    VoltageSequencer voltage_sequencer;  
    VoltageSequencer chance_sequencer;
    CycleCounters *cycle_counters = new CycleCounters();
    bool dice_roll_outcome = true;

    bool rollDice()
    {
        // Read the chance sequencer
        float chance = chance_sequencer.getValue();

        // genereate a randome number between 0 and 1
        float random_number = random::uniform();

        // Store the outcome
        dice_roll_outcome = (random_number <= chance);

        // Return the outcome
        return dice_roll_outcome;
    }
};