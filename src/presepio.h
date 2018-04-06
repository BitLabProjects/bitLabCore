#ifndef _PRESEPIO_H_
#define _PRESEPIO_H_

#include "mbed.h"
#include "PinNames.h"

class Presepio {
public:
  Presepio();

  void init();
  void loop();

private:
  DigitalOut led_heartbeat;
  
  InterruptIn main_crossover;
  bool       rise_received;

  Serial     pc;

  Ticker     ticker;
  bool       tick_received;
  int        tick_count;

  void tick();
  void main_crossover_rise();
};

#endif