#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"

class Presepio {
public:
  Presepio();

  void init();
  void loop();
  void dimming();

private:
  DigitalOut led_heartbeat;
  DigitalOut led_dimming;
  
  InterruptIn main_crossover;
  bool       rise_received;

  Serial     pc;

  Ticker     ticker;
  bool       tick_received;
  int        tick_count;
  int        rise_count;

  void tick();
  void main_crossover_rise();
};

#endif