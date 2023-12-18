#pragma once

#include <cmath>  // For std::floor and std::round

#define MAX_CLOCK_DIVISION 16
#define MIN_CLOCK_DIVISION 1
#define MAX_VOLTAGE 5.0f  // Updating to new max voltage
#define MIN_VOLTAGE -5.0f // Adding a new min voltage

struct ClockDivider
{
  public:
    unsigned int counter = 0; // Counter to keep track of the ticks
    bool newClockState = false; // Indicates whether a new clock pulse should be emitted

    ClockDivider()
    {
      // Initialize here if needed
    }

    // Function to calculate division based on voltage
    unsigned int calculateDivision(float voltage)
    {
      // Clamp voltage between MIN_VOLTAGE and MAX_VOLTAGE
      voltage = std::max(MIN_VOLTAGE, std::min(voltage, MAX_VOLTAGE));
      
      // Map the voltage to the clock division range
      return static_cast<unsigned int>(std::round((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * (MAX_CLOCK_DIVISION - MIN_CLOCK_DIVISION) + MIN_CLOCK_DIVISION));
    }

    // Function to process each clock tick and determine new clock state based on rate
    // Returns true if a new clock pulse should be emitted, false otherwise
    bool process(float rate)
    {
      unsigned int division = calculateDivision(rate); // Get division based on voltage
      counter++;
      
      if (counter >= division)
      {
        counter = 0;
        newClockState = true;
        return true;  // Emit a new clock pulse
      }
      else
      {
        newClockState = false;
        return false; // Do not emit a new clock pulse
      }
    }
};
