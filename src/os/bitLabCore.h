#ifndef _BITLABCORE_H_
#define _BITLABCORE_H_

#include "mbed.h"

#include "SDFileSystem.h"
#include "CoreTicker.h"

class bitLabCore {
public:
  bitLabCore();

  void run();

protected:
  virtual void init();
  virtual void mainLoop() = 0;
  virtual void tick(millisec64 timeDelta) = 0;

private:
  Serial pc;
  SDFileSystem sd;
  CoreTicker coreTicker;

  int getUsedHeap();
};

#endif