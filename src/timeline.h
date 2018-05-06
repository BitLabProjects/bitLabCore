#ifndef _TIMELINE_H_
#define _TIMELINE_H_

#include "mbed.h"

struct TimelineEntry
{
  int32_t time;
  uint8_t output;
  int32_t value;
  int32_t duration;

  static const uint8_t OutputForTimelineEnd = 255;
  bool isTimelineEnd() const
  {
    return output == OutputForTimelineEnd;
  }
};

class Timeline
{
public:
  Timeline();

  void create(int32_t newEntriesCapacity);
  void add(int32_t time, uint8_t output, int32_t value, int32_t duration);
  const TimelineEntry *getCurrent();
  void moveFirst();
  void moveNext();
  bool isFinished();

private:
  TimelineEntry *entries;
  int32_t entriesCount;
  int32_t entriesCapacity;
  int32_t currentIdx;
};

#endif