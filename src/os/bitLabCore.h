#ifndef _BITLABCORE_H_
#define _BITLABCORE_H_

#include <vector>

#include "mbed.h"

#include "SDFileSystem.h"
#include "CoreTicker.h"
#include "CoreModule.h"

class bitLabCore {
public:
  bitLabCore();

  void init();
  void addModule(CoreModule* module);
  void run();

private:
  Serial pc;
  SDFileSystem sd;
  CoreTicker coreTicker;
  void tick(millisec64 timeDelta);

  vector<CoreModule*> modules;

  int getUsedHeap();
};

#endif