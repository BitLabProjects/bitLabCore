#ifndef _STORYBOARDPLAYER_H_
#define _STORYBOARDPLAYER_H_

#include "Storyboard.h"
#include "mbed.h"

struct PlayBufferEntry
{
  int32_t outputId;
  TimelineEntry entry;
};

enum PlayStatus
{
  Stopped,
  Paused,
  Playing
};

class StoryboardPlayer {
public:
  StoryboardPlayer(Storyboard* storyboard, Callback<void(const PlayBufferEntry*)> onSetOutput);

  void fillPlayBuffer();
  void advance(millisec timeDelta);

  void play();
  void pause();
  void stop();

  bool isPlaying() { return playStatus == PlayStatus::Playing; }
  millisec64 getStoryboardTime() { return storyboardTime; }
  void setStoryboardTimeSyncDelta(millisec value) { syncDelta = value; }

private:
  Storyboard* storyboard;
  Callback<void(const PlayBufferEntry*)> onSetOutput;

  //Circular buffer for storyboard playing
  PlayStatus playStatus;
  int playBufferCount;
  PlayBufferEntry* playBuffer;
  volatile int playBufferHead;
  volatile int playBufferTail;
  millisec playBufferHeadTime;
  millisec playBufferTailTime;
  millisec playBufferMaxTime;
  volatile millisec64 storyboardTime;
  // Synchronization delta: positive values means we need to catch up
  volatile millisec64 syncDelta;

  void executePlayBuffer();
};

#endif