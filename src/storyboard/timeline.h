#ifndef _TIMELINE_H_
#define _TIMELINE_H_

#include "..\os\types.h"

struct TimelineEntry
{
  millisec time;
  int32_t value;
  millisec duration;

  void clear()
  {
    time = 0;
    duration = 0;
    value = 0;
  }
};

class Timeline
{
public:
  Timeline();

  void create(uint32_t outputHardwareId, uint8_t output, uint8_t newEntriesCapacity);
  void clear();
  void setOutputId(uint8_t outputId);
  uint32_t getOutputHardwareId() { return outputHardwareId; }
  uint8_t getOutputId() { return outputId; }
  void add(millisec time, int32_t value, millisec duration);
  void setEntry(uint8_t entryIdx, millisec time, int32_t value, millisec duration);
  const TimelineEntry *getCurrent();
  const TimelineEntry *getEntry(int idx) { return &entries[idx]; }
  void moveFirst();
  void moveNext();
  bool isFinished();
  inline uint8_t getEntriesCount() { return entriesCount; }

  uint32_t calcCrc32(uint32_t initialCrc);

private:
  uint32_t outputHardwareId;
  uint8_t outputId;
  const static uint8_t entriesCapacity = 10;
  TimelineEntry entries[entriesCapacity];
  uint8_t entriesCount;
  uint8_t currentIdx;
};

#endif