#pragma once

// From: https://github.com/surge-synthesizer/clap-saw-demo/blob/main/src/saw-voice.cpp
// Reformatted to fit my personal coding style.

enum Mode
{
    LP,
    HP,
    BP,
    NOTCH,
    PEAK,
    ALL
};

struct Filter
{
    int mode = LP;
    float cutoff = 1.0;
    float resonance = 0.7;
    float pival = 3.14159265358979323846;

    float ic1eq[2] = {0.0, 0.0};
    float ic2eq[2] = {0.0, 0.0};
    float g = 0.0;
    float k = 0.0;
    float gk = 0.0;
    float a1 = 0.0;
    float a2 = 0.0;
    float a3 = 0.0;
    float ak = 0.0;

    bool dirty = true;

    Filter()
    {
        dirty = true;
    }

    // cutoff value should range from 0 to 1
    void setCutoff(float cutoff)
    {
        if(this->cutoff != cutoff)
        {
            this->cutoff = cutoff;
            dirty = true;
        }
    }
    
    // resonance should range from 0 to 1
    void setResonance(float resonance)
    {
        if(this->resonance != resonance)
        {
            this->resonance = resonance;
            dirty = true;
        }
    }

    void setMode(int mode)
    {
        if(this->mode != mode)
        {
            init();
            this->mode = mode;
            dirty = true;
        }
    }

    void recalculate()
    {
        float key = 69 + (cutoff * 8.68);
        float tuned_cutoff = 440.0 * (pow(2.0, key - 69.0) / 12);
        tuned_cutoff = clamp(tuned_cutoff, 10.0, 15000.0); // just to be safe/lazy

        resonance = clamp(resonance, 0.01f, 0.99f);
        g = std::tan(pival * tuned_cutoff * APP->engine->getSampleTime());
        k = 2.0 - 2.0 * resonance;
        gk = g + k;
        a1 = 1.0 / (1.0 + g * gk);
        a2 = g * a1;
        a3 = g * a2;
        ak = gk * a1;
    }

    void init()
    {
        for (int channel = 0; channel < 2; ++channel)
        {
            ic1eq[channel] = 0.f;
            ic2eq[channel] = 0.f;
        }
    }

    void process(float *left, float *right)
    {
        if(dirty) recalculate();

        float vin[2] = {*left, *right};
        float result[2] = {0, 0};

        for (int channel = 0; channel < 2; channel++)
        {
            auto v3 = vin[channel] - ic2eq[channel];
            auto v0 = a1 * v3 - ak * ic1eq[channel];
            auto v1 = a2 * v3 + a1 * ic1eq[channel];
            auto v2 = a3 * v3 + a2 * ic1eq[channel] + ic2eq[channel];

            ic1eq[channel] = 2 * v1 - ic1eq[channel];
            ic2eq[channel] = 2 * v2 - ic2eq[channel];

            switch (mode)
            {
            case LP:
                result[channel] = v2;
                break;
            case BP:
                result[channel] = v1;
                break;
            case HP:
                result[channel] = v0;
                break;
            case NOTCH:
                result[channel] = v2 + v0; // low + high
                break;
            case PEAK:
                result[channel] = v2 - v0; // low - high;
                break;
            case ALL:
                result[channel] = v2 + v0 - k * v1; // low + high - k * band
                break;
            }
        }

        *left = result[0];
        *right = result[1];
    }
};
