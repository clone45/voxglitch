#include "CycleCounter.hpp"
#include <vector>

const int MAX_SEQUENCE_LENGTH = 16; 

struct CycleCounters
{
    unsigned int sequence_length = MAX_SEQUENCE_LENGTH;
    CycleCounter *cycle_counters[MAX_SEQUENCE_LENGTH];

    CycleCounters()
    {
        for (unsigned int i = 0; i < MAX_SEQUENCE_LENGTH; i++)
        {
            cycle_counters[i] = new CycleCounter();
        }
    }

    ~CycleCounters()
    {
        for (unsigned int i = 0; i < MAX_SEQUENCE_LENGTH; i++)
        {
            delete cycle_counters[i]; // Free each CycleCounter
            cycle_counters[i] = nullptr; // Set the pointer to nullptr
        }
    }

    void setCountDown(unsigned int index, int value)
    {
        cycle_counters[index]->setCountDown(value);
    }

    int getCountDown(unsigned int index)
    {
        return(cycle_counters[index]->getCountDown());
    }

    int *getCountDownPointer(unsigned int index)
    {
        return(cycle_counters[index]->getCountDownPointer());
    }

    void setCycle(unsigned int index, unsigned int value)
    {
        cycle_counters[index]->setCycle(value);
    }

    unsigned int getCycle(unsigned int index)
    {
        return(cycle_counters[index]->getCycle());
    }

    void reset()
    {
        for (unsigned i = 0; i < MAX_SEQUENCE_LENGTH; i++)
        {
            cycle_counters[i]->reset();
        }
    }

    void resetCounter(unsigned int index)
    {
        cycle_counters[index]->reset();
    }

    void decrement(unsigned int index)
    {
        cycle_counters[index]->decrement();
    }

};