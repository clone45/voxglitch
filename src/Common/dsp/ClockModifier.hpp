#include <iostream>
#include <cmath> // for std::floor, std::abs
#include <deque>

class ClockModifier
{
private:

  double time_between_clocks_ms = 0.0; // Time between incoming clock pulses in milliseconds

  std::deque<double> timer_targets;

  // Variables for clock multiplication
  double previous_clock_time = 0.0;

  // Variables for clock division
  unsigned int clock_division_counter = 0;

  // Variable for rate factor
  float rate_factor = 1.0f;

public:

  bool clock(double time_now, float rate_factor)
  {
    if(previous_clock_time == 0.0)
    {
      previous_clock_time = time_now;
    }
    else
    {
      time_between_clocks_ms = time_now - previous_clock_time;
      previous_clock_time = time_now;
    }

    this->rate_factor = rate_factor;

    if(rate_factor == 0.0f) return(true);

    // Clock divide
    if(rate_factor < 0)
    {
      return clockDivide(static_cast<unsigned int>(std::floor(rate_factor * -1.0f)));
    }

    // Clock multiply
    clockMultiply(static_cast<unsigned int>(std::floor(rate_factor) + 1), time_now);
    return(true);
  }

  // I only call the process function if I'm doing clock multiplication
  bool process(double time_now)
  {
    if(timer_targets.empty()) return false;

    if(time_now >= timer_targets.front())  // Use front() instead of top()
    {
      timer_targets.pop_front();  // Use pop_front() instead of pop()
      return true;
    }
    else
    {
      return false;
    }
  }

  void clockMultiply(int clock_multiplication, double time_now)
  {
    // Clear the timer_targets deque
    timer_targets.clear();

    // Fill the timer_targets deque with the time between clocks divided by the rate factor
    double interval = time_between_clocks_ms / clock_multiplication;
    for(int i = 0; i < (clock_multiplication - 1); i++)
    {
      timer_targets.push_back(interval * (i + 1) + time_now);  // Use push_back() instead of push()
    }
  }

  // Simple clock division
  bool clockDivide(unsigned int clock_division)
  {
    if(clock_division == 0) clock_division = 1;

    clock_division_counter++;

    if (clock_division_counter >= clock_division)
    {
      clock_division_counter = 0;
      return true; // Emit a new clock pulse
    }
    else
    {
      return false; // Do not emit a new clock pulse
    }
  }

};
