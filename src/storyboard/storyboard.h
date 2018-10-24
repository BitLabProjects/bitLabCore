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
  bool getNextTimelineAndEntry(millisec time, uint8_t* output, const TimelineEntry** entry);
  
  inline uint8_t getTimelinesCount() { return timelinesCount; };
  uint8_t getEntriesCount(uint8_t output);

  uint32_t calcCrc32(uint32_t initialCrc);

private:
  Timeline *timelines;
  uint8_t timelinesCapacity;
  uint8_t timelinesCount;
  millisec duration;

  uint8_t getTimelineIdx(uint8_t output);
};

#endif