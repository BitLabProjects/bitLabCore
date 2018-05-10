#ifndef _STORYBOARD_H_
#define _STORYBOARD_H_

#include "timeline.h"
#include "config.h"

class Storyboard
{
public:
  Storyboard();

  void create(int32_t newTimelinesCapacity, millisec totalDuration);
  void addTimeline(int32_t newEntriesCapacity);
  void addEntry(int32_t time, uint8_t output, int32_t value, int32_t duration);
  const TimelineEntry *getCurrent(uint8_t output);
  bool isFinished(millisec currTime);
  void reset();
  
  int32_t entriesCount(uint8_t output);
  void advanceTimeline(uint8_t output);
  bool isTimelineFinished(uint8_t output);

  int32_t timelinesCount;

private:
  Timeline *timelines;
  int32_t timelinesCapacity;
  int32_t duration;
};

#endif