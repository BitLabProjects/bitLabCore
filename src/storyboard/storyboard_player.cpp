#include "storyboard_player.h"

#include "..\os\os.h"

StoryboardPlayer::StoryboardPlayer(Storyboard *storyboard, 
                                   RelayBoard *relay_board, 
                                   TriacBoard *triac_board) : 
  storyboard(storyboard),
  relay_board(relay_board),
  triac_board(triac_board)
{
  playBufferCount = 10;
  playBuffer = new PlayBufferEntry[playBufferCount];
  stop();
}

void StoryboardPlayer::init()
{
  // Do nothing
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

void StoryboardPlayer::mainLoop()
{
  if (playStatus == Playing)
  {
    fillPlayBuffer();
  }
}

void StoryboardPlayer::fillPlayBuffer()
{
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
    uint8_t output;
    const TimelineEntry *entry;
    if (!storyboard->getNextTimelineAndEntry(playBufferHeadTime, &output, &entry))
    {
      if (lastCycleWasReset)
      {
        //Maybe empty storyboard? stop play
        Os::debug("play buffer: no entry found for two cycles, stopping\n");
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
    playBuffer[playBufferHead].output = output;
    playBuffer[playBufferHead].entry = *entry;
    playBufferHeadTime = entry->time;

    fillCount += 1;

    playBufferHead = (playBufferHead + 1) % playBufferCount;
    lastCycleWasReset = false;
  }
  //Os::debug("play buffer: filled %i entries\n", fillCount);
}

void StoryboardPlayer::tick(millisec64 timeDelta)
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

  triac_board->onTick(storyboardTime);
  relay_board->onTick();
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
      applyTimelineEntry(pbEntry->output, &pbEntry->entry);
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

void StoryboardPlayer::applyTimelineEntry(uint8_t output, const TimelineEntry *entry)
{
  if (output >= 1 && output <= 8)
  {
    int triacIdx = output - 1;
    triac_board->setOutput(triacIdx, entry->value, entry->time, entry->duration);
  }
  else if (output >= 9 && output <= 40)
  {
    int relayIdx = output - 9;
    relay_board->setOutput(relayIdx, entry->value);
  }
}
