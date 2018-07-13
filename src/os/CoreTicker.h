#ifndef _CORETICKER_H_
#define _CORETICKER_H_

#include "mbed.h"
#include "types.h"

class CoreTicker {
public:
  CoreTicker(int ticksPerSecond, mbed::Callback<void(millisec64 timeDelta)> onTickCallback);

private:
  Ticker ticker;
  int ticksPerSecond;
  volatile bool tick_received;
  volatile millisec64 tick_count;
  volatile millisec64 currTime; //Time in milliseconds

  mbed::Callback<void(millisec64 timeDelta)> onTickCallback;

  void tick();
};

#endif