#include "timeline.h"

Timeline::Timeline()
{
  entries = NULL;
  entriesCount = 0;
  entriesCapacity = 0;
  currentIdx = 0;
}

void Timeline::create(int32_t newEntriesCapacity)
{
  if (entries)
  {
    delete[] entries;
  }
  entries = new TimelineEntry[newEntriesCapacity];
  entriesCapacity = newEntriesCapacity;
  entriesCount = 0;
  currentIdx = 0;
}

void Timeline::add(int32_t time, uint8_t output, int32_t value, int32_t duration)
{
  if (entriesCount == entriesCapacity)
  {
    //No more space! TODO signal
    return;
  }

  if (entriesCount > 0)
  {
    //Verify order, the entries must be ordered by time.
    if (time < entries[entriesCount - 1].time)
    {
      //Error, discard. TODO Signal
      return;
    }
  }

  entries[entriesCount].time = time;
  entries[entriesCount].output = output;
  entries[entriesCount].value = value;
  entries[entriesCount].duration = duration;
  entriesCount += 1;
}

const TimelineEntry *Timeline::getCurrent()
{
  return &entries[currentIdx];
}

void Timeline::moveFirst()
{
  currentIdx = 0;
}
void Timeline::moveNext()
{
  if (currentIdx < entriesCount)
  {
    currentIdx += 1;
  }
}
bool Timeline::isFinished()
{
  return currentIdx == entriesCount;
}