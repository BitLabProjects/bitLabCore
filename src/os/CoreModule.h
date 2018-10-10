#ifndef _COREMODULE_H_
#define _COREMODULE_H_

#include "types.h"

// Forward declaration to use the type in init method
class bitLabCore;

class CoreModule {
public:
  CoreModule() {};

  virtual const char* getName() = 0;
  virtual void init(const bitLabCore*) = 0;
  virtual void mainLoop() = 0;
  virtual void tick(millisec64 timeDelta) = 0;
};

#endif