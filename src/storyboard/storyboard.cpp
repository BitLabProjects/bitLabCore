#include "storyboard.h"
#include "utils.h"

#include "..\os\os.h"

Storyboard::Storyboard()
{
  timelines = NULL;
  timelinesCapacity = 0;
  timelinesCount = 0;
}

void Storyboard::create(int32_t newTimelinesCapacity, millisec totalDuration)
{
  if (timelines)
  {
    delete[] timelines;
  }
  timelines = new Timeline[newTimelinesCapacity];
  timelinesCapacity = newTimelinesCapacity;
  duration = totalDuration;
}

Timeline *Storyboard::addTimeline(uint8_t output, int32_t newEntriesCapacity)
{
  if (timelinesCount < timelinesCapacity)
  {
    timelines[timelinesCount].create(output, newEntriesCapacity);
    timelinesCount += 1;
  }
  else
  {
    Os::debug("Error");
  }

  return &timelines[timelinesCount - 1];
}

Timeline *Storyboard::getTimeline(uint8_t output)
{
  return &timelines[getTimelineIdx(output)];
}

bool Storyboard::isFinished(millisec currTime)
{
  return (currTime >= duration);
}

void Storyboard::reset()
{
  for (uint8_t outputIdx = 0; outputIdx < timelinesCapacity; outputIdx++)
  {
    timelines[outputIdx].moveFirst();
  }
}

int Storyboard::getTimelineIdx(uint8_t output)
{
  return Utils::max(0, Utils::min(output - 1, timelinesCapacity - 1));
}