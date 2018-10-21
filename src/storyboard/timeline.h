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

  void create(uint8_t output, uint8_t newEntriesCapacity);
  void clear();
  void setOutput(uint8_t output);
  uint8_t getOutput() { return output; }
  void add(millisec time, int32_t value, millisec duration);
  void set(uint8_t entryIdx, millisec time, int32_t value, millisec duration);
  const TimelineEntry *getCurrent();
  void moveFirst();
  void moveNext();
  bool isFinished();
  uint8_t getEntriesCount() { return entriesCount; }

  uint32_t calcCrc32(uint32_t initialCrc);

private:
  uint8_t output;
  TimelineEntry *entries;
  uint8_t entriesCount;
  uint8_t entriesCapacity;
  uint8_t currentIdx;
};

#endif