#ifndef _STORYBOARD_H_
#define _STORYBOARD_H_

#include "Timeline.h"
#include "..\os\types.h"

class Storyboard
{
public:
  Storyboard();

  void create(uint8_t newTimelinesCapacity, millisec totalDuration);
  Timeline* addTimeline(uint32_t outputHardwareId, uint8_t outputId, uint8_t newEntriesCapacity);
  Timeline* getTimeline(uint8_t output);
  inline Timeline* getTimelineByIdx(uint8_t idx) { return &timelines[idx]; };
  bool isFinished(millisec currTime);
  inline millisec getDuration() { return duration; }
  void reset();
  bool getNextTimelineAndEntry(millisec time, uint8_t* outputId, const TimelineEntry** entry);
  
  inline uint8_t getTimelinesCount() { return timelinesCount; };

  uint32_t calcCrc32(uint32_t initialCrc);

private:
  const static uint8_t timelinesCapacity = 16;
  Timeline timelines[timelinesCapacity];
  uint8_t timelinesCount;
  millisec duration;
};

#endif