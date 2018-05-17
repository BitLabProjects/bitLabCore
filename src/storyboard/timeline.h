#ifndef _TIMELINE_H_
#define _TIMELINE_H_

#include "mbed.h"

struct TimelineEntry
{
  int32_t time;
  int32_t value;
  int32_t duration;
};

class Timeline
{
public:
  Timeline();

  void create(uint8_t output, int32_t newEntriesCapacity);
  void add(int32_t time, int32_t value, int32_t duration);
  const TimelineEntry *getCurrent();
  void moveFirst();
  void moveNext();
  bool isFinished();

private:
  uint8_t output;
  TimelineEntry *entries;
  int32_t entriesCount;
  int32_t entriesCapacity;
  int32_t currentIdx;
};

#endif