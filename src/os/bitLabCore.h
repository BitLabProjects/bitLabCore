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

  void addModule(CoreModule* module);
  void run();

protected:
  void init();
  void tick(millisec64 timeDelta);

private:
  Serial pc;
  SDFileSystem sd;
  CoreTicker coreTicker;

  vector<CoreModule*> modules;

  int getUsedHeap();
};

#endif