#include "StoryboardPlayer.h"

#include "..\os\os.h"
#include "..\Utils.h"

StoryboardPlayer::StoryboardPlayer(Storyboard *storyboard, Callback<void(const PlayBufferEntry*)> onSetOutput) : storyboard(storyboard),
                                                                                                                 onSetOutput(onSetOutput)
{
  playBufferCount = 10;
  playBuffer = new PlayBufferEntry[playBufferCount];
  stop();
}

void StoryboardPlayer::play()
{
  playBufferMaxTime = storyboard->getDuration();
  // Fill initial buffer
  fillPlayBuffer();
  playStatus = Playing;
}
void StoryboardPlayer::pause()
{
  //Keep everything as-is, so that a successive play resumes from where we left on pause
  playStatus = Paused;
}
void StoryboardPlayer::stop()
{
  //Reset all the buffers, a successive play starts from the beginning
  playBufferHead = 0;
  playBufferTail = 0;
  playBufferHeadTime = 0;
  playBufferTailTime = 0;
  storyboardTime = 0;
  storyboard->reset();
  playStatus = Stopped;
}

void StoryboardPlayer::fillPlayBuffer()
{
  if (playStatus != Playing)
  {
    return;
  }

  int fillCount = 0;
  int playBufferLast = (playBufferTail + (playBufferCount - 1)) % playBufferCount;
  if (playBufferHead == playBufferLast)
  {
    return;
  }

  bool lastCycleWasReset = false;
  //Os::debug("play buffer: filling...\n");
  while (playBufferHead != playBufferLast)
  {
    // Find next and put it in head position, then increment head
    uint8_t outputId;
    const TimelineEntry *entry;
    if (!storyboard->getNextTimelineAndEntry(playBufferHeadTime, &outputId, &entry))
    {
      if (lastCycleWasReset)
      {
        //Maybe empty storyboard? stop play
        //Os::debug("play buffer: no entry found for two cycles, stopping\n");
        stop();
        return;
      }
      lastCycleWasReset = true;
      //Nothing to add, all the timelines are ended but the storyboard isn't
      //Os::debug("play buffer: no entry found\n");
      storyboard->reset();
      playBufferHeadTime = 0;
      continue;
    }
    //Os::debug("play buffer: adding #%i at %i ms to %i in %i ms\n", output, entry->time, entry->value, entry->duration);
    playBuffer[playBufferHead].outputId = outputId;
    playBuffer[playBufferHead].entry = *entry;
    playBufferHeadTime = entry->time;

    fillCount += 1;

    playBufferHead = (playBufferHead + 1) % playBufferCount;
    lastCycleWasReset = false;
  }
  //Os::debug("play buffer: filled %i entries\n", fillCount);
}

void StoryboardPlayer::advance(millisec timeDelta)
{
  if (playStatus == Playing)
  {
    millisec64 newStoryboardTime = (storyboardTime + timeDelta) % playBufferMaxTime;
    if (newStoryboardTime < playBufferTailTime)
    {
      //execute all remaining entries up to max time
      storyboardTime = playBufferMaxTime;
      executePlayBuffer();
      //Then reset
      playBufferTailTime = 0;
    }

    storyboardTime = newStoryboardTime;
    executePlayBuffer();
  }
}

void StoryboardPlayer::executePlayBuffer()
{
  //Try consuming the next entry in the play buffer, if present
  if (playBufferTail != playBufferHead)
  {
    PlayBufferEntry *pbEntry = &playBuffer[playBufferTail];
    millisec entryTime = pbEntry->entry.time;
    //Keep track of tail time, that is the time of the last applied entry,
    //to discard entries that belong to the beginning of the next storyboard cycle
    if (entryTime >= playBufferTailTime && entryTime <= storyboardTime)
    {
      //Apply!
      onSetOutput(pbEntry);
      playBufferTailTime = entryTime;
      playBufferTail = (playBufferTail + 1) % playBufferCount;
    }
    else
    {
      //This entry is too new or of another storyboard cycle, stop executing
      return;
    }
  }
}
