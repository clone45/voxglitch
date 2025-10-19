#pragma once

#define MAX_DELAY_SIZE 8192

// Simple circular buffer delay line for Karplus-Strong
struct DelayLine
{
    float buffer[MAX_DELAY_SIZE];
    unsigned int writePos = 0;
    unsigned int length = 1024;

    DelayLine()
    {
        clear();
    }

    void clear()
    {
        for(unsigned int i = 0; i < MAX_DELAY_SIZE; i++)
        {
            buffer[i] = 0.0;
        }
        writePos = 0;
    }

    void setLength(unsigned int newLength)
    {
        if(newLength > MAX_DELAY_SIZE) newLength = MAX_DELAY_SIZE;
        if(newLength < 1) newLength = 1;
        length = newLength;
    }

    unsigned int getLength()
    {
        return length;
    }

    // Push a new sample into the delay line
    void push(float sample)
    {
        buffer[writePos] = sample;
        writePos++;
        if(writePos >= length) writePos = 0;
    }

    // Read the oldest sample (at the read position)
    float read()
    {
        return buffer[writePos];
    }

    // Fill the entire buffer with values (for excitation)
    void fill(float *samples, unsigned int count)
    {
        unsigned int samplesToFill = (count < length) ? count : length;
        for(unsigned int i = 0; i < samplesToFill; i++)
        {
            buffer[i] = samples[i];
        }
        writePos = 0;
    }
};
