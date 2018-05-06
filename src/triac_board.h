#ifndef _TRIACBOARD_H_
#define _TRIACBOARD_H_

#include "mbed.h"
#include "PinNames.h"
#include "config.h"

class TriacBoard
{
public:
  TriacBoard();

  void setOutput(int outputIdx, int percent);
  void updateOutputs();

private:
  // show connection to 50Hz external signal (230Vac)
  DigitalOut led_heartbeat;
  DigitalOut outputs[ANALOGOUT_COUNT];

  InterruptIn main_crossover;

  // percent set for each output
  int states[ANALOGOUT_COUNT];

  Ticker zerocross_sim;
  int rise_count;
  int tick_per_rise_count;

  void main_crossover_rise();
};

#endif