#include "Timeline.h"
#include "..\utils.h"
#include "stddef.h"

Timeline::Timeline() : output(1)
{
  entries = NULL;
  entriesCount = 0;
  entriesCapacity = 0;
  currentIdx = 0;
}

void Timeline::create(uint8_t output, uint8_t newEntriesCapacity)
{
  if (entries)
  {
    delete[] entries;
  }
  entries = new TimelineEntry[newEntriesCapacity];
  entriesCapacity = newEntriesCapacity;
  setOutput(output);
  clear();
}

void Timeline::clear()
{
  entriesCount = 0;
  moveFirst();
}

void Timeline::setOutput(uint8_t output)
{
  this->output = output;
}

void Timeline::add(millisec time, int32_t value, millisec duration)
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

  entriesCount += 1;
  set(entriesCount - 1, time, value, duration);
}

void Timeline::set(uint8_t entryIdx, millisec time, int32_t value, millisec duration)
{
  if (entryIdx >= entriesCount) {
    // TODO signal
    return;
  }

  entries[entryIdx].time = time;
  entries[entryIdx].value = value;
  entries[entryIdx].duration = duration;
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

uint32_t Timeline::calcCrc32(uint32_t initialCrc) {
  auto crc32 = initialCrc;
  crc32 = Utils::crc32(output, crc32);
  crc32 = Utils::crc32(entriesCount, crc32);
  for (uint8_t i = 0; i < entriesCount; i++)
  {
    crc32 = Utils::crc32(entries[i].time, crc32);
    crc32 = Utils::crc32(entries[i].value, crc32);
    crc32 = Utils::crc32(entries[i].duration, crc32);
  }
  return crc32;
}