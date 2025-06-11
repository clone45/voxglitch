struct CycleCounter
{
    unsigned int cycle = 0;
    int count_down = 0;

    void setCycle(unsigned int new_cycle)
    {
        cycle = new_cycle;
    }

    unsigned int getCycle()
    {
        return(cycle);
    }

    void setCountDown(int new_count_down)
    {
        count_down = new_count_down;
    }

    int getCountDown()
    {
        return(count_down);
    }

    int *getCountDownPointer()
    {
        return(&count_down);
    }

    void reset()
    {
        count_down = cycle;
    }

    void decrement()
    {
        // Note that this may go to -1, and that's expected
        count_down--;
    }
};
