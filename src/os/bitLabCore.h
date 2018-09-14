#ifndef _BITLABCORE_H_
#define _BITLABCORE_H_

#include <vector>

#include "mbed.h"

#ifdef UseSDCard
#include "FATFileSystem.h"
#include "SDBlockDevice.h"
#endif
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
  
  #ifdef UseSDCard
  SDBlockDevice sdbd;
  FATFileSystem fs;
  #endif

  CoreTicker coreTicker;
  void tick(millisec64 timeDelta);

  vector<CoreModule*> modules;

  int getUsedHeap();
};

#endif