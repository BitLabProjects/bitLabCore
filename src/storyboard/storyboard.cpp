#include "Storyboard.h"

#include "..\utils.h"
#include "..\os\os.h"

Storyboard::Storyboard()
{
  timelines = NULL;
  timelinesCapacity = 0;
  timelinesCount = 0;
  duration = 0;
}

void Storyboard::create(uint8_t newTimelinesCapacity, millisec totalDuration)
{
  if (timelines)
  {
    delete[] timelines;
  }
  timelines = new Timeline[newTimelinesCapacity];
  timelinesCapacity = newTimelinesCapacity;
  duration = totalDuration;
}

Timeline *Storyboard::addTimeline(uint32_t outputHardwareId, uint8_t outputId, uint8_t newEntriesCapacity)
{
  if (timelinesCount < timelinesCapacity)
  {
    timelines[timelinesCount].create(outputHardwareId, outputId, newEntriesCapacity);
    timelinesCount += 1;
  }
  else
  {
    //Os::debug("Error");
  }

  return &timelines[timelinesCount - 1];
}

Timeline *Storyboard::getTimeline(uint8_t output)
{
  return getTimelineByIdx(getTimelineIdx(output));
}

bool Storyboard::isFinished(millisec currTime)
{
  return (currTime >= duration);
}

void Storyboard::reset()
{
  for (uint8_t outputIdx = 0; outputIdx < timelinesCount; outputIdx++)
  {
    timelines[outputIdx].moveFirst();
  }
}

bool Storyboard::getNextTimelineAndEntry(millisec time, uint8_t* output, const TimelineEntry** entry) 
{
  if (isFinished(time))
  {
    return false;
  }

  //Os::debug("next search: begin...\n");
  //Look at the next entry for each timeline, and use the one with nearest time
  int8_t idxBest = -1;
  millisec timeBest = 0;
  for (int8_t idx = 0; idx < timelinesCount; idx++)
  {
    if (timelines[idx].isFinished())
    {
      //Os::debug("next search: #%i is finished...\n", idx+1);
      continue;
    }
    const TimelineEntry* curr = timelines[idx].getCurrent();
    if (idxBest == -1 || curr->time < timeBest) {
      //Os::debug("next search: #%i is better, %i ms < %i ms...\n", idx+1, curr->time, timeBest);
      idxBest = idx;
      timeBest = curr->time;
    } else {
      //Os::debug("next search: #%i is worse, %i ms >= %i ms...\n", idx+1, curr->time, timeBest);
    }
  }

  if (idxBest == -1) {
    // All timelines finished
    //Os::debug("next search: nothing found\n");
    return false;
  }

  // One found, return it and advance the relative timeline by one
  *output = idxBest + 1;
  *entry = timelines[idxBest].getCurrent();
  timelines[idxBest].moveNext();

  //Os::debug("next search: best is #%i at %i ms\n", *output, (*entry)->time);

  return true;
}

uint8_t Storyboard::getTimelineIdx(uint8_t output)
{
  return Utils::max(0, Utils::min((uint8_t)(output - 1), (uint8_t)(timelinesCount - 1)));
}

uint32_t Storyboard::calcCrc32(uint32_t initialCrc) {
  auto crc32 = initialCrc;
  //Always insert a 123 in the crc to avoid having a zero when no timeline is present
  crc32 = Utils::crc32((uint8_t)123, crc32);
  crc32 = Utils::crc32(timelinesCount, crc32);
  crc32 = Utils::crc32(duration, crc32);
  for (uint8_t i = 0; i < timelinesCount; i++)
  {
    crc32 = timelines[i].calcCrc32(crc32);
  }
  return crc32;
}