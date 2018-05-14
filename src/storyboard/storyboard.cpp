#include "storyboard.h"
#include "utils.h"

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

void Storyboard::addTimeline(int32_t newEntriesCapacity)
{
  if (timelinesCount >= timelinesCapacity)
  {
    return;
  }
  timelines[timelinesCount] = Timeline();
  timelines[timelinesCount].create(newEntriesCapacity);
  timelinesCount += 1;
}

void Storyboard::addEntry(int32_t time, uint8_t output, int32_t value, int32_t duration)
{
  if (output > timelinesCount)
  {
    // not enough timelines!!
    return;
  }

  timelines[output - 1].add(time, output, value, duration);
}

const TimelineEntry *Storyboard::getCurrent(uint8_t output)
{
  uint8_t normalizedOutput = Utils::min(output - 1, timelinesCapacity - 1);
  return timelines[normalizedOutput].getCurrent();
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

void Storyboard::advanceTimeline(uint8_t output)
{
  timelines[output - 1].moveNext();
}

bool Storyboard::isTimelineFinished(uint8_t output)
{
  return timelines[output - 1].isFinished();
}