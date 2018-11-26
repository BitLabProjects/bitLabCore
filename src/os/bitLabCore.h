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

  inline uint32_t getHardwareId() const { return hardware_id; };
  const char *getClockSourceDescr() const;

  const CoreModule* findModule(const char* name) const;

private:
  #ifdef UseSerialForMessages
  Serial pc;
  #endif
  
  #ifdef UseSDCard
  SDBlockDevice sdbd;
  FATFileSystem fs;
  #endif

  uint32_t hardware_id;

  CoreTicker coreTicker;
  void tick(millisec timeDelta);

  vector<CoreModule*> modules;

  int getUsedHeap();
};

#endif