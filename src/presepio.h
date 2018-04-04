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
  Serial     pc;
  Ticker     ticker;
  bool       tick_received;
  int        tick_count;

  void tick();
};

#endif