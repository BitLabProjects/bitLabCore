#include "CoreTicker.h"

CoreTicker::CoreTicker(int ticksPerSecond, 
                       mbed::Callback<void(millisec64 timeDelta)> onTickCallback) {
  this->ticksPerSecond = ticksPerSecond;
  this->onTickCallback = onTickCallback;
  currTime = 0;
  tick_received = false;
  tick_count = 0;
  
  ticker.attach(callback(this, &CoreTicker::tick), 1.0 / ticksPerSecond);
}

void CoreTicker::tick()
{
  tick_received = true;
  tick_count += 1;

  millisec64 prevCurrTime = currTime;
  currTime = 1000 * tick_count / ticksPerSecond;

  onTickCallback(currTime - prevCurrTime);
}