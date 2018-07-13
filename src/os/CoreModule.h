#ifndef _COREMODULE_H_
#define _COREMODULE_H_

#include "types.h"

class CoreModule {
public:
  CoreModule() {};

  virtual void init() = 0;
  virtual void mainLoop() = 0;
  virtual void tick(millisec64 timeDelta) = 0;
};

#endif