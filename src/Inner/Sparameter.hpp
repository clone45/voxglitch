#pragma once
#include <memory>
#include <vector>

class Sparameter
{

private:

    float value = 0.0;

public:

    Sparameter(float initial_value = 0.0)
    {
        this->value = initial_value;
    }

    void setValue(float new_value)
    {
        this->value = new_value;
    }

    float getValue()
    {
        return value;
    }
};